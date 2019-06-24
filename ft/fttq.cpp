/*
* Copyright (c) 2009-2019 Brian Waters
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

#include "fttq.h"

Bool FTThreadQueueBase::m_debug = False;

FTThreadQueueBase::FTThreadQueueBase()
{
   m_initialized = False;
}

FTThreadQueueBase::~FTThreadQueueBase()
{
}

Void FTThreadQueueBase::init(Int nMsgCnt,
                             Int threadId,
                             Bool bMultipleWriters,
                             FTThreadQueueBase::Mode eMode)
{
   m_mode = eMode;

   // construct the shared memory name
   Char szName[FT_FILENAME_MAX];
   ft_sprintf_s(szName, sizeof(szName), "%d", threadId);

   // calcuate the space required
   int nSize = sizeof(FTThreadMessage) * nMsgCnt;

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
      Char szMutexName[FT_FILENAME_MAX];
      Char szSemFreeName[FT_FILENAME_MAX];
      Char szSemMsgsName[FT_FILENAME_MAX];

      ft_sprintf_s(szMutexName, sizeof(szMutexName), "ThreadQueueMutex_%s", szName);
      ft_sprintf_s(szSemFreeName, sizeof(szSemFreeName), "ThreadQueueSemFree_%s", szName);
      ft_sprintf_s(szSemMsgsName, sizeof(szSemMsgsName), "ThreadQueueSemMsgs_%s", szName);

      mutex().init(szMutexName);
      semFree().init(msgCnt(), msgCnt(), szSemFreeName);
      semMsgs().init(0, msgCnt(), szSemMsgsName);

      semFreeId() = semFree().getSemid();
      semMsgsId() = semMsgs().getSemid();
   }
   else
   {
   }

   FTMutexLock l(mutex());

   if ((eMode == ReadOnly || eMode == ReadWrite) && numReaders() > 0)
   {
      throw new FTThreadQueueBaseError_MultipleReadersNotAllowed();
   }

   refCnt()++;
   numReaders() += (eMode == ReadOnly || eMode == ReadWrite) ? 1 : 0;
   numWriters() += (eMode == WriteOnly || eMode == ReadWrite) ? 1 : 0;

   m_initialized = True;
}

Void FTThreadQueueBase::destroy()
{
   if (m_initialized)
   {
      mutex().enter();

      if (refCnt() == 1)
      {
         semFree().destroy();
         semMsgs().destroy();

         mutex().leave();
         mutex().destroy();
      }
      else
      {
         refCnt()--;
         numReaders() -= (m_mode == ReadOnly || m_mode == ReadWrite) ? 1 : 0;
         numWriters() -= (m_mode == WriteOnly || m_mode == ReadWrite) ? 1 : 0;
         mutex().leave();
      }
   }
}

Bool FTThreadQueueBase::push(UInt msgid, Bool wait)
{
   FTThreadMessage::fttmessage_data_t d;
   d.quadPart = 0;
   return push(msgid, d, wait);
}

Bool FTThreadQueueBase::push(UInt msgid, Dword lowPart, Long highPart, Bool wait)
{
   FTThreadMessage::fttmessage_data_t d;
   d.u.lowPart = lowPart;
   d.u.highPart = highPart;
   return push(msgid, d, wait);
}

Bool FTThreadQueueBase::push(UInt msgid, pVoid voidPtr, Bool wait)
{
   FTThreadMessage::fttmessage_data_t d;
   d.voidPtr = voidPtr;
   return push(msgid, d, wait);
}

Bool FTThreadQueueBase::push(UInt msgid, LongLong quadPart, Bool wait)
{
   FTThreadMessage::fttmessage_data_t d;
   d.quadPart = quadPart;
   return push(msgid, d, wait);
}

Bool FTThreadQueueBase::push(UInt msgid, FTThreadMessage::fttmessage_data_t &d, Bool wait)
{
   if (m_mode == ReadOnly)
      throw new class FTThreadQueueBaseError_NotOpenForWriting();

   if (semFree().Decrement(wait) < 0)
      return False;

   {
      FTMutexLock l(mutex());

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

Bool FTThreadQueueBase::pop(FTThreadMessage &msg, Bool wait)
{
   if (m_mode == WriteOnly)
      throw new FTThreadQueueBaseError_NotOpenForReading();

   if (!semMsgs().Decrement(wait))
      return False;

   msg = data()[msgTail()++];

   if (msgTail() >= msgCnt())
      msgTail() = 0;

   semFree().Increment();

   return True;
}

Bool FTThreadQueueBase::peek(FTThreadMessage &msg, Bool wait)
{
   if (m_mode == WriteOnly)
      throw new FTThreadQueueBaseError_NotOpenForReading();

   if (!semMsgs().Decrement(wait))
      return False;

   msg = data()[msgTail()];

   // since we are not pulling the message off, we need to increment
   // the semaphore to put it back where it was
   semMsgs().Increment();

   return True;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTThreadQueuePublic::FTThreadQueuePublic()
{
   m_pCtrl = NULL;
   m_pData = NULL;
}

FTThreadQueuePublic::~FTThreadQueuePublic()
{
}

Void FTThreadQueuePublic::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_sharedmem.init(sFile, cId, nSize + sizeof(ftthreadmessagequeue_ctrl_t));
   m_pCtrl = (ftthreadmessagequeue_ctrl_t *)m_sharedmem.getDataPtr();
   m_pData = (FTThreadMessage *)(((pChar)m_sharedmem.getDataPtr()) + sizeof(ftthreadmessagequeue_ctrl_t));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTThreadQueuePrivate::FTThreadQueuePrivate()
    : m_mutex(False)
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

FTThreadQueuePrivate::~FTThreadQueuePrivate()
{
   if (m_pData)
   {
      delete[](pChar) m_pData;
      m_pData = NULL;
   }
}

Void FTThreadQueuePrivate::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_pData = (FTThreadMessage *)new Char[nSize];
   memset((pChar)m_pData, 0, nSize);
}
