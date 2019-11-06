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

#include "etq.h"
#include "esynch2.h"

/// @cond DOXYGEN_EXCLUDE

Bool EThreadQueueBase::m_debug = False;

EThreadQueueBase::EThreadQueueBase()
{
   m_initialized = False;
}

EThreadQueueBase::~EThreadQueueBase()
{
}

Void EThreadQueueBase::init(Int nMsgCnt,
                             Int threadId,
                             Bool bMultipleWriters,
                             EThreadQueueBase::Mode eMode)
{
   m_mode = eMode;

   // construct the shared memory name
   Char szName[EPC_FILENAME_MAX];
   epc_sprintf_s(szName, sizeof(szName), "%d", threadId);

   // calcuate the space required
   int nSize = sizeof(EThreadMessage) * nMsgCnt;

   // initialize the shared memory
   allocDataSpace(szName, 'A', nSize);

   // initialize the control block values
   if (refCnt() == 0)
   {
      refCnt() = 0;
      numReaders() = 0;
      numWriters() = 0;
      multipleWriters() = bMultipleWriters;
      msgCnt() = nMsgCnt;
      msgHead() = 0;
      msgTail() = 0;

      // initialize the control mutex and semaphores
      initMutex();
      initSemFree(msgCnt());
      initSemMsgs(0);
   }
   else
   {
   }

   EMutexLock l(mutex());

   if ((eMode == ReadOnly || eMode == ReadWrite) && numReaders() > 0)
   {
      throw EThreadQueueBaseError_MultipleReadersNotAllowed();
   }

   refCnt()++;
   numReaders() += (eMode == ReadOnly || eMode == ReadWrite) ? 1 : 0;
   numWriters() += (eMode == WriteOnly || eMode == ReadWrite) ? 1 : 0;

   m_initialized = True;
}

Void EThreadQueueBase::destroy()
{
   Bool destroyMutex = False;

   if (m_initialized)
   {
      EMutexLock l(mutex());

      if (refCnt() == 1)
      {
         semFree().destroy();
         semMsgs().destroy();

         destroyMutex = True;
      }
      else
      {
         refCnt()--;
         numReaders() -= (m_mode == ReadOnly || m_mode == ReadWrite) ? 1 : 0;
         numWriters() -= (m_mode == WriteOnly || m_mode == ReadWrite) ? 1 : 0;
      }
   }

   if (destroyMutex)
      mutex().destroy();
}

Bool EThreadQueueBase::push(UInt msgid, Bool wait)
{
   EThreadMessage::etmessage_data_t d;
   d.quadPart = 0;
   return push(msgid, d, wait);
}

Bool EThreadQueueBase::push(UInt msgid, Dword lowPart, Long highPart, Bool wait)
{
   EThreadMessage::etmessage_data_t d;
   d.u.lowPart = lowPart;
   d.u.highPart = highPart;
   return push(msgid, d, wait);
}

Bool EThreadQueueBase::push(UInt msgid, pVoid voidPtr, Bool wait)
{
   EThreadMessage::etmessage_data_t d;
   d.voidPtr = voidPtr;
   return push(msgid, d, wait);
}

Bool EThreadQueueBase::push(UInt msgid, LongLong quadPart, Bool wait)
{
   EThreadMessage::etmessage_data_t d;
   d.quadPart = quadPart;
   return push(msgid, d, wait);
}

Bool EThreadQueueBase::push(UInt msgid, EThreadMessage::etmessage_data_t &d, Bool wait)
{
   if (m_mode == ReadOnly)
      throw EThreadQueueBaseError_NotOpenForWriting();

   if (semFree().Decrement(wait) < 0)
      return False;

   {
      EMutexLock l(mutex());

      if (m_debug)
         data()[msgHead()].getTimer().Start();
      data()[msgHead()].getMsgId() = msgid;
      data()[msgHead()].getQuadPart() = d.quadPart;

      msgHead()++;

      if (msgHead() >= msgCnt())
         msgHead() = 0;
   }

   semMsgs().Increment();

   return True;
}

Bool EThreadQueueBase::pop(EThreadMessage &msg, Bool wait)
{
   if (m_mode == WriteOnly)
      throw EThreadQueueBaseError_NotOpenForReading();

   if (!semMsgs().Decrement(wait))
      return False;

   msg = data()[msgTail()++];

   if (msgTail() >= msgCnt())
      msgTail() = 0;

   semFree().Increment();

   return True;
}

Bool EThreadQueueBase::peek(EThreadMessage &msg, Bool wait)
{
   if (m_mode == WriteOnly)
      throw EThreadQueueBaseError_NotOpenForReading();

   if (!semMsgs().Decrement(wait))
      return False;

   msg = data()[msgTail()];

   // since we are not pulling the message off, we need to increment
   // the semaphore to put it back where it was
   semMsgs().Increment();

   return True;
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EThreadQueuePublic::EThreadQueuePublic()
{
   m_pCtrl = NULL;
   m_pData = NULL;
}

EThreadQueuePublic::~EThreadQueuePublic()
{
}

Void EThreadQueuePublic::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_sharedmem.init(sFile, cId, nSize + sizeof(ethreadmessagequeue_ctrl_t));
   m_pCtrl = (ethreadmessagequeue_ctrl_t *)m_sharedmem.getDataPtr();
   m_pData = (EThreadMessage *)(((pChar)m_sharedmem.getDataPtr()) + sizeof(ethreadmessagequeue_ctrl_t));
}

Void EThreadQueuePublic::initMutex()
{
   EMutexPublic m;
   m_pCtrl->m_mutexid = m.mutexId();
   m.detach();
}

Void EThreadQueuePublic::initSemFree(UInt initialCount)
{
   ESemaphorePublic s(initialCount);
   m_pCtrl->m_freeSemId = s.semIndex();
   s.detach();
}

Void EThreadQueuePublic::initSemMsgs(UInt initialCount)
{
   ESemaphorePublic s(initialCount);
   m_pCtrl->m_msgsSemId = s.semIndex();
   s.detach();
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EThreadQueuePrivate::EThreadQueuePrivate()
    : m_mutex(False), m_semFree(0, False), m_semMsgs(0, False)
{
   m_refCnt = 0;
   m_numReaders = 0;
   m_numWriters = 0;
   m_multipleWriters = False;

   m_msgCnt = 0;
   m_head = 0; // next location to write
   m_tail = 0; // next location to read
   m_pData = NULL;
}

EThreadQueuePrivate::~EThreadQueuePrivate()
{
   if (m_pData)
   {
      delete[](pChar) m_pData;
      m_pData = NULL;
   }
}

Void EThreadQueuePrivate::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_pData = (EThreadMessage *)new Char[nSize];
   memset((pChar)m_pData, 0, nSize);
}

Void EThreadQueuePrivate::initMutex()
{
   m_mutex.init();
}

Void EThreadQueuePrivate::initSemFree(UInt initialCount)
{
   m_semFree.init(initialCount);
}

Void EThreadQueuePrivate::initSemMsgs(UInt initialCount)
{
   m_semMsgs.init(initialCount);
}

/// @endcond
