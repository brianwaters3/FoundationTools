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

#ifndef __fttq_h_included
#define __fttq_h_included

#include "ftbase.h"
#include "fterror.h"
#include "fttimer.h"
#include "ftsynch.h"
#include "ftsynch2.h"
#include "ftshmem.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(FTThreadQueueBaseError_NotOpenForWriting);
DECLARE_ERROR(FTThreadQueueBaseError_NotOpenForReading);
DECLARE_ERROR(FTThreadQueueBaseError_MultipleReadersNotAllowed);

DECLARE_ERROR(FTThreadQueuePublicError_UnInitialized);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadQueueBase;

class FTThreadMessage
{
   friend class FTThreadQueueBase;

protected:
   typedef union _fttmessage_data {
      struct
      {
         Dword lowPart;
         Long highPart;
      } u;
      pVoid voidPtr;
      LongLong quadPart;
   } fttmessage_data_t;

public:
   FTThreadMessage()
   {
      m_msgid = 0;
      m_data.quadPart = 0;
   }

   FTThreadMessage(UInt msgid)
   {
      m_msgid = msgid;
      m_data.quadPart = 0;
   }
   FTThreadMessage(UInt msgid, Dword lowPart, Long highPart)
   {
      m_msgid = msgid;
      m_data.u.lowPart = lowPart;
      m_data.u.highPart = highPart;
   }
   FTThreadMessage(UInt msgid, pVoid voidPtr)
   {
      m_msgid = msgid;
      m_data.voidPtr = voidPtr;
   }
   FTThreadMessage(UInt msgid, LongLong quadPart)
   {
      m_msgid = msgid;
      m_data.quadPart = quadPart;
   }

   ~FTThreadMessage()
   {
   }

   FTThreadMessage &operator=(FTThreadMessage &val)
   {
      m_msgid = val.m_msgid;
      m_timer = val.m_timer;
      m_data.quadPart = val.m_data.quadPart;

      return *this;
   }

   Void set(UInt msgid)
   {
      m_msgid = msgid;
      m_data.quadPart = 0;
   }
   Void set(UInt msgid, Dword lowPart, Long highPart)
   {
      m_msgid = msgid;
      m_data.u.lowPart = lowPart;
      m_data.u.highPart = highPart;
   }
   Void set(UInt msgid, pVoid voidPtr)
   {
      m_msgid = msgid;
      m_data.voidPtr = voidPtr;
   }
   Void set(UInt msgid, LongLong quadPart)
   {
      m_msgid = msgid;
      m_data.quadPart = quadPart;
   }

   FTTimerElapsed &getTimer()
   {
      return m_timer;
   }
   UInt &getMsgId()
   {
      return m_msgid;
   }
   Dword &getLowPart()
   {
      return m_data.u.lowPart;
   }
   Long &getHighPart()
   {
      return m_data.u.highPart;
   }
   LongLong &getQuadPart()
   {
      return m_data.quadPart;
   }
   pVoid &getVoidPtr()
   {
      return m_data.voidPtr;
   }

private:
   FTTimerElapsed m_timer;
   UInt m_msgid;
   fttmessage_data_t m_data;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadMessageQueuePublic;
class FTThreadMessageQueuePrivate;
class FTThreadBase;

class FTThreadQueueBase
{
   friend class FTThreadMessageQueuePublic;
   friend class FTThreadMessageQueuePrivate;
   friend class FTThreadBase;

public:
   enum Mode
   {
      ReadOnly,
      WriteOnly,
      ReadWrite
   };

   Bool push(UInt msgid, Bool wait = True);
   Bool push(UInt msgid, Dword lowPart, Long highPart, Bool wait = True);
   Bool push(UInt msgid, pVoid voidPtr, Bool wait = True);
   Bool push(UInt msgid, LongLong quadPart, Bool wait = True);

   Bool pop(FTThreadMessage &msg, Bool wait = True);
   Bool peek(FTThreadMessage &msg, Bool wait = True);

   Bool isInitialized() { return m_initialized; }
   Mode mode() { return m_mode; }

protected:
   Bool push(UInt msgid, FTThreadMessage::fttmessage_data_t &d, Bool wait = True);

   virtual Bool isPublic() = 0;
   virtual Int &msgCnt() = 0;
   virtual Int &msgHead() = 0;
   virtual Int &msgTail() = 0;
   virtual Bool &multipleWriters() = 0;
   virtual Int &numReaders() = 0;
   virtual Int &numWriters() = 0;
   virtual Int &refCnt() = 0;
   virtual FTThreadMessage *data() = 0;
   virtual Void allocDataSpace(cpStr sFile, Char cId, Int nSize) = 0;
   virtual Void initMutex() = 0;
   virtual Void initSemFree(UInt initialCount) = 0;
   virtual Void initSemMsgs(UInt initialCount) = 0;

   virtual FTMutexData &mutex() = 0;
   virtual FTSemaphoreData &semMsgs() = 0;
   virtual FTSemaphoreData &semFree() = 0;

   FTThreadQueueBase();
   ~FTThreadQueueBase();

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             FTThreadQueueBase::Mode eMode);
   Void destroy();

private:
   static Bool m_debug;
   Bool m_initialized;
   Mode m_mode;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadQueuePublic : public FTThreadQueueBase
{
public:
   FTThreadQueuePublic();
   ~FTThreadQueuePublic();

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             FTThreadQueueBase::Mode eMode)
   {
      FTThreadQueueBase::init(nMsgCnt, threadId, bMultipleWriters, eMode);
   }

protected:
   Bool isPublic() { return True; }
   Int &msgCnt() { return m_pCtrl->m_msgCnt; }
   Int &msgHead() { return m_pCtrl->m_head; }
   Int &msgTail() { return m_pCtrl->m_tail; }
   Bool &multipleWriters() { return m_pCtrl->m_multipleWriters; }
   Int &numReaders() { return m_pCtrl->m_numReaders; }
   Int &numWriters() { return m_pCtrl->m_numWriters; }
   Int &refCnt() { return m_pCtrl->m_refCnt; }
   FTThreadMessage *data() { return m_pData; }
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize);
   Void initMutex();
   Void initSemFree(UInt initialCount);
   Void initSemMsgs(UInt initialCount);

   FTMutexData &mutex() { return FTSynchObjects::getMutex(m_pCtrl->m_mutexid); }
   FTSemaphoreData &semFree() { return FTSynchObjects::getSemaphore(m_pCtrl->m_freeSemId); }
   FTSemaphoreData &semMsgs() { return FTSynchObjects::getSemaphore(m_pCtrl->m_msgsSemId); }

private:
   typedef struct
   {
      Int m_refCnt;
      Int m_numReaders;
      Int m_numWriters;
      Bool m_multipleWriters;

      Int m_msgCnt;
      Int m_head; // next location to write
      Int m_tail; // next location to read

      Int m_mutexid;
      Int m_freeSemId;
      Int m_msgsSemId;
   } ftthreadmessagequeue_ctrl_t;

   FTSharedMemory m_sharedmem;
   ftthreadmessagequeue_ctrl_t *m_pCtrl;
   FTThreadMessage *m_pData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadQueuePrivate : public FTThreadQueueBase
{
public:
   FTThreadQueuePrivate();
   ~FTThreadQueuePrivate();

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             FTThreadQueueBase::Mode eMode)
   {
      FTThreadQueueBase::init(nMsgCnt, threadId, bMultipleWriters, eMode);
   }

protected:
   Bool isPublic() { return False; }
   Int &msgCnt() { return m_msgCnt; }
   Int &msgHead() { return m_head; }
   Int &msgTail() { return m_tail; }
   Bool &multipleWriters() { return m_multipleWriters; }
   Int &numReaders() { return m_numReaders; }
   Int &numWriters() { return m_numWriters; }
   Int &refCnt() { return m_refCnt; }
   FTThreadMessage *data() { return m_pData; }
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize);
   Void initMutex();
   Void initSemFree(UInt initialCount);
   Void initSemMsgs(UInt initialCount);

   FTMutexData &mutex() { return m_mutex; }
   FTSemaphoreData &semFree() { return m_semFree; }
   FTSemaphoreData &semMsgs() { return m_semMsgs; }

private:
   Int m_refCnt;
   Int m_numReaders;
   Int m_numWriters;
   Bool m_multipleWriters;

   Int m_msgCnt;
   Int m_head; // next location to write
   Int m_tail; // next location to read

   FTMutexPrivate m_mutex;
   FTSemaphorePrivate m_semFree;
   FTSemaphorePrivate m_semMsgs;

   FTThreadMessage *m_pData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #define __fttq_h_included
