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

#ifndef __etq_h_included
#define __etq_h_included

#include "ebase.h"
#include "eerror.h"
#include "etimer.h"
#include "esynch.h"
#include "esynch2.h"
#include "eshmem.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(EThreadQueueBaseError_NotOpenForWriting);
DECLARE_ERROR(EThreadQueueBaseError_NotOpenForReading);
DECLARE_ERROR(EThreadQueueBaseError_MultipleReadersNotAllowed);

DECLARE_ERROR(EThreadQueuePublicError_UnInitialized);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadQueueBase;

class EThreadMessage
{
   friend class EThreadQueueBase;

protected:
   typedef union _etmessage_data {
      struct
      {
         Dword lowPart;
         Long highPart;
      } u;
      pVoid voidPtr;
      LongLong quadPart;
   } etmessage_data_t;

public:
   EThreadMessage()
   {
      m_msgid = 0;
      m_data.quadPart = 0;
   }

   EThreadMessage(UInt msgid)
   {
      m_msgid = msgid;
      m_data.quadPart = 0;
   }
   EThreadMessage(UInt msgid, Dword lowPart, Long highPart)
   {
      m_msgid = msgid;
      m_data.u.lowPart = lowPart;
      m_data.u.highPart = highPart;
   }
   EThreadMessage(UInt msgid, pVoid voidPtr)
   {
      m_msgid = msgid;
      m_data.voidPtr = voidPtr;
   }
   EThreadMessage(UInt msgid, LongLong quadPart)
   {
      m_msgid = msgid;
      m_data.quadPart = quadPart;
   }

   ~EThreadMessage()
   {
   }

   EThreadMessage &operator=(EThreadMessage &val)
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

   ETimer &getTimer()
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
   ETimer m_timer;
   UInt m_msgid;
   etmessage_data_t m_data;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadMessageQueuePublic;
class EThreadMessageQueuePrivate;
class EThreadBase;

class EThreadQueueBase
{
   friend class EThreadMessageQueuePublic;
   friend class EThreadMessageQueuePrivate;
   friend class EThreadBase;

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

   Bool pop(EThreadMessage &msg, Bool wait = True);
   Bool peek(EThreadMessage &msg, Bool wait = True);

   Bool isInitialized() { return m_initialized; }
   Mode mode() { return m_mode; }

protected:
   Bool push(UInt msgid, EThreadMessage::etmessage_data_t &d, Bool wait = True);

   virtual Bool isPublic() = 0;
   virtual Int &msgCnt() = 0;
   virtual Int &msgHead() = 0;
   virtual Int &msgTail() = 0;
   virtual Bool &multipleWriters() = 0;
   virtual Int &numReaders() = 0;
   virtual Int &numWriters() = 0;
   virtual Int &refCnt() = 0;
   virtual EThreadMessage *data() = 0;
   virtual Void allocDataSpace(cpStr sFile, Char cId, Int nSize) = 0;
   virtual Void initMutex() = 0;
   virtual Void initSemFree(UInt initialCount) = 0;
   virtual Void initSemMsgs(UInt initialCount) = 0;

   virtual EMutexData &mutex() = 0;
   virtual ESemaphoreData &semMsgs() = 0;
   virtual ESemaphoreData &semFree() = 0;

   EThreadQueueBase();
   ~EThreadQueueBase();

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             EThreadQueueBase::Mode eMode);
   Void destroy();

private:
   static Bool m_debug;
   Bool m_initialized;
   Mode m_mode;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadQueuePublic : public EThreadQueueBase
{
public:
   EThreadQueuePublic();
   ~EThreadQueuePublic();

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             EThreadQueueBase::Mode eMode)
   {
      EThreadQueueBase::init(nMsgCnt, threadId, bMultipleWriters, eMode);
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
   EThreadMessage *data() { return m_pData; }
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize);
   Void initMutex();
   Void initSemFree(UInt initialCount);
   Void initSemMsgs(UInt initialCount);

   EMutexData &mutex() { return ESynchObjects::getMutex(m_pCtrl->m_mutexid); }
   ESemaphoreData &semFree() { return ESynchObjects::getSemaphore(m_pCtrl->m_freeSemId); }
   ESemaphoreData &semMsgs() { return ESynchObjects::getSemaphore(m_pCtrl->m_msgsSemId); }

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
   } ethreadmessagequeue_ctrl_t;

   ESharedMemory m_sharedmem;
   ethreadmessagequeue_ctrl_t *m_pCtrl;
   EThreadMessage *m_pData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadQueuePrivate : public EThreadQueueBase
{
public:
   EThreadQueuePrivate();
   ~EThreadQueuePrivate();

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             EThreadQueueBase::Mode eMode)
   {
      EThreadQueueBase::init(nMsgCnt, threadId, bMultipleWriters, eMode);
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
   EThreadMessage *data() { return m_pData; }
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize);
   Void initMutex();
   Void initSemFree(UInt initialCount);
   Void initSemMsgs(UInt initialCount);

   EMutexData &mutex() { return m_mutex; }
   ESemaphoreData &semFree() { return m_semFree; }
   ESemaphoreData &semMsgs() { return m_semMsgs; }

private:
   Int m_refCnt;
   Int m_numReaders;
   Int m_numWriters;
   Bool m_multipleWriters;

   Int m_msgCnt;
   Int m_head; // next location to write
   Int m_tail; // next location to read

   EMutexPrivate m_mutex;
   ESemaphorePrivate m_semFree;
   ESemaphorePrivate m_semMsgs;

   EThreadMessage *m_pData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #define __etq_h_included
