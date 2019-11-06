/*
* Copyright (c) 2009-2019 Brian Waters
* Copyright (c) 2019 Sprint
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "eqbase.h"
#include "eatomic.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void EQueueMessage::getLength(ULong &length)
{
   elementLength(m_timer, length);
   elementLength(m_msgType, length);
}

Void EQueueMessage::serialize(pVoid pBuffer, ULong &nOffset)
{
   pack(m_timer, pBuffer, nOffset);
   pack(m_msgType, pBuffer, nOffset);
}

Void EQueueMessage::unserialize(pVoid pBuffer, ULong &nOffset)
{
   unpack(m_timer, pBuffer, nOffset);
   unpack(m_msgType, pBuffer, nOffset);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EQueueBase::EQueueBase()
{
   m_initialized = False;
   m_mode = ReadOnly;
   memset(m_rBuffer, 0, sizeof(m_rBuffer));
   memset(m_wBuffer, 0, sizeof(m_rBuffer));
}

EQueueBase::~EQueueBase()
{
}

Void EQueueBase::init(ULong nMsgSize,
                       Int nMsgCnt,
                       Int queueId,
                       Bool bMultipleReaders,
                       Bool bMultipleWriters,
                       EQueueBase::Mode eMode)
{
   m_mode = eMode;

   // construct the shared memory name
   Char szName[EPC_FILENAME_MAX];
   epc_sprintf_s(szName, sizeof(szName), "%d", queueId);

   // calcuate the space required
   int nSize = nMsgSize * nMsgCnt;

   // initialize the shared memory
   allocDataSpace(szName, 'A', nSize);

   // initialize the control block values
   if (refCnt() == 0)
   {
      refCnt() = 0;
      numReaders() = 0;
      numWriters() = 0;
      multipleReaders() = bMultipleReaders;
      multipleWriters() = bMultipleWriters;
      msgSize() = nMsgSize;
      msgCnt() = nMsgCnt;
      msgHead() = 0;
      msgTail() = 0;

      // initialize the control mutex and semaphores
      initReadMutex();
      initWriteMutex();
      initSemFree(msgCnt());
      initSemMsgs(0);
   }

   try
   {
      EMutexLock l(writeMutex());

      if (!multipleReaders() && (eMode == ReadOnly || eMode == ReadWrite) && numReaders() > 0)
      {
         throw EQueueBaseError_MultipleReadersNotAllowed();
      }

      refCnt()++;
      numReaders() += (eMode == ReadOnly || eMode == ReadWrite) ? 1 : 0;
      numWriters() += (eMode == WriteOnly || eMode == ReadWrite) ? 1 : 0;

      m_initialized = True;
   }
   catch (EError &e)
   {
      throw;
   }
}

/// @endcond

Void EQueueBase::destroy()
{
   if (m_initialized)
   {
      bool destroyWriteMutex = false;

      {
         EMutexLock l(writeMutex());

         numReaders() -= (m_mode == ReadOnly || m_mode == ReadWrite) ? 1 : 0;
         numWriters() -= (m_mode == WriteOnly || m_mode == ReadWrite) ? 1 : 0;

         if (refCnt() == 1)
         {
            semFree().destroy();
            semMsgs().destroy();

            readMutex().destroy();
            destroyWriteMutex = True;
         }
         else
         {
            refCnt()--;
         }
      }

      if (destroyWriteMutex)
         writeMutex().destroy();

      m_initialized = False;
   }
}

Bool EQueueBase::push(EQueueMessage &msg, Bool wait)
{
   ULong length;
   Int i;

   if (m_mode == ReadOnly)
      throw EQueueBaseError_NotOpenForWriting();

   // get the message length
   length = sizeof(ULong);
   msg.getLength(length);

   // calculate the number of slots required to package the message
   int nSlots = (length / msgSize()) + (((length % msgSize()) > 0) ? 1 : 0);

   // reserve slots for the message
   for (i = 0; i < nSlots; i++)
   {
      if (!semFree().Decrement(wait))
      {
         // the required number of slots are not available
         // "un" reserve the slots that were allocated
         for (; i > 0; i--)
            semFree().Increment();
         return False;
      }
   }

   ULong msgOfs = 0;
   ULong amtWritten = 0;

   // lock the object if necessary
   EMutexLock l(writeMutex(), multipleWriters());

   length = sizeof(ULong);
   // serizlize the message heirarchy
   msg.serialize(m_wBuffer, length);
   // set the message length
   //*((ULong*)m_wBuffer) = length;  the avoid strict-aliasing rules warning
   pULong pulBuffer = (pULong)m_wBuffer;
   *pulBuffer = length;

   int dataOfs = msgHead() * msgSize();
   for (i = 0; i < nSlots; i++)
   {
      int len = (length - amtWritten) > msgSize() ? msgSize() : length - amtWritten;
      memcpy(&data()[dataOfs], &m_wBuffer[msgOfs], len);

      amtWritten += len;

      msgHead()++;

      msgOfs += msgSize();

      if (msgHead() >= msgCnt())
      {
         msgHead() = 0;
         dataOfs = 0;
      }
      else
      {
         dataOfs += msgSize();
      }
   }

   semMsgs().Increment();

   return True;
}

EQueueMessage *EQueueBase::pop(Bool wait)
{
   ULong length;
   ULong offset;

   if (m_mode == WriteOnly)
      throw EQueueBaseError_NotOpenForReading();

   if (!semMsgs().Decrement(wait))
      return NULL;

   EMutexLock l(readMutex(), multipleReaders());

   // get the message length
   length = *(ULong *)&data()[msgTail() * msgSize()];

   // get the message
   ULong amt = 0;
   while (True)
   {
      int len = (length - amt) > msgSize() ? msgSize() : length - amt;

      memcpy(&m_rBuffer[amt], &data()[msgTail() * msgSize()], len);
      amt += len;

      msgTail()++;
      if (msgTail() >= msgCnt())
         msgTail() = 0;

      semFree().Increment();

      if (amt >= length)
         break;
   }

   // unserialize the message
   EQueueMessage msg;

   offset = sizeof(ULong); // skip over the message length
   msg.unserialize(m_rBuffer, offset);

   EQueueMessage *pMsg = allocMessage(msg.getMsgType());
   if (pMsg)
   {
      offset = sizeof(ULong); // skip over the message length
      pMsg->unserialize(m_rBuffer, offset);
   }

   return pMsg;
}
