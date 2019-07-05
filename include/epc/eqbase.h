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

#ifndef __eqbase_h_included
#define __eqbase_h_included

#include "ebase.h"
#include "etime.h"
#include "etimer.h"
#include "etq.h"
#include "emsg.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(EQueueBaseError_UnInitialized);
DECLARE_ERROR(EQueueBaseError_NotOpenForWriting);
DECLARE_ERROR(EQueueBaseError_NotOpenForReading);
DECLARE_ERROR(EQueueBaseError_MultipleReadersNotAllowed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EQueueBase;

class EQueueMessage : public EMessage
{
   friend class EQueueBase;

public:
   EQueueMessage()
   {
   }

   ~EQueueMessage()
   {
   }

   EQueueMessage &operator=(const EQueueMessage &a)
   {
      m_timer = a.m_timer;
      m_msgType = a.m_msgType;
      return *this;
   }

   virtual Void getLength(ULong &length);
   virtual Void serialize(pVoid pBuffer, ULong &nOffset);
   virtual Void unserialize(pVoid pBuffer, ULong &nOffset);

   ETime &getTimer()
   {
      return m_timer;
   }

   Void setMsgType(Long msgType) { m_msgType = msgType; }
   Long getMsgType() { return m_msgType; }

private:
   ETime m_timer;
   Long m_msgType;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EQueuePublic;
class EQueuePrivate;

class EQueueBase
{
   friend class EQueuePublic;
   friend class EQueuePrivate;

public:
   enum Mode
   {
      ReadOnly,
      WriteOnly,
      ReadWrite
   };

   Bool push(EQueueMessage &msg, Bool wait = True);
   EQueueMessage *pop(Bool wait = True);

   Void destroy();

protected:
   virtual Bool isPublic() = 0;
   virtual ULong &msgSize() = 0;
   virtual Int &msgCnt() = 0;
   virtual Long &msgHead() = 0;
   virtual Long &msgTail() = 0;
   virtual Bool &multipleReaders() = 0;
   virtual Bool &multipleWriters() = 0;
   virtual Int &numReaders() = 0;
   virtual Int &numWriters() = 0;
   virtual Int &refCnt() = 0;
   virtual pChar data() = 0;
   virtual Void allocDataSpace(cpStr sFile, Char cId, Int nSize) = 0;
   virtual Void initReadMutex() = 0;
   virtual Void initWriteMutex() = 0;
   virtual Void initSemFree(UInt initialCount) = 0;
   virtual Void initSemMsgs(UInt initialCount) = 0;

   virtual EMutexData &readMutex() = 0;
   virtual EMutexData &writeMutex() = 0;
   virtual ESemaphoreData &semMsgs() = 0;
   virtual ESemaphoreData &semFree() = 0;

   virtual EQueueMessage *allocMessage(Long msgType) = 0;

   Mode mode() { return m_mode; }

   EQueueBase();
   ~EQueueBase();

   Void init(ULong nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
             Bool bMultipleWriters, EQueueBase::Mode eMode);

private:
   Bool m_initialized;
   Mode m_mode;
   Char m_rBuffer[USHRT_MAX];
   Char m_wBuffer[USHRT_MAX];
};

#endif // #define __eqbase_h_included
