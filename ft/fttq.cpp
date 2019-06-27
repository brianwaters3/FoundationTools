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
#include "ftsynch2.h"

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
      initMutex();
      initSemFree(msgCnt());
      initSemMsgs(0);
   }
   else
   {
   }

   FTMutexLock l(mutex());

   if ((eMode == ReadOnly || eMode == ReadWrite) && numReaders() > 0)
   {
      throw FTThreadQueueBaseError_MultipleReadersNotAllowed();
   }

   refCnt()++;
   numReaders() += (eMode == ReadOnly || eMode == ReadWrite) ? 1 : 0;
   numWriters() += (eMode == WriteOnly || eMode == ReadWrite) ? 1 : 0;

   m_initialized = True;
}

Void FTThreadQueueBase::destroy()
{
   Bool destroyMutex = False;

   if (m_initialized)
   {
      FTMutexLock l(mutex());

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
      throw FTThreadQueueBaseError_NotOpenForWriting();

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
      throw FTThreadQueueBaseError_NotOpenForReading();

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
      throw FTThreadQueueBaseError_NotOpenForReading();

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

Void FTThreadQueuePublic::initMutex()
{
   FTMutexPublic m;
   m_pCtrl->m_mutexid = m.mutexId();
   m.detach();
}

Void FTThreadQueuePublic::initSemFree(UInt initialCount)
{
   FTSemaphorePublic s(initialCount);
   m_pCtrl->m_freeSemId = s.semIndex();
   s.detach();
}

Void FTThreadQueuePublic::initSemMsgs(UInt initialCount)
{
   FTSemaphorePublic s(initialCount);
   m_pCtrl->m_msgsSemId = s.semIndex();
   s.detach();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTThreadQueuePrivate::FTThreadQueuePrivate()
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

Void FTThreadQueuePrivate::initMutex()
{
   m_mutex.init();
}

Void FTThreadQueuePrivate::initSemFree(UInt initialCount)
{
   m_semFree.init(initialCount);
}

Void FTThreadQueuePrivate::initSemMsgs(UInt initialCount)
{
   m_semMsgs.init(initialCount);
}