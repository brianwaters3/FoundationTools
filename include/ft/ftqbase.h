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

#ifndef __ftqbase_h_included
#define __ftqbase_h_included

#include "ftbase.h"
#include "fttime.h"
#include "fttimer.h"
#include "fttq.h"
#include "ftmessage.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(FTQueueBaseError_UnInitialized);
DECLARE_ERROR(FTQueueBaseError_NotOpenForWriting);
DECLARE_ERROR(FTQueueBaseError_NotOpenForReading);
DECLARE_ERROR(FTQueueBaseError_MultipleReadersNotAllowed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTQueueBase;

class FTQueueMessage : public FTMessage
{
   friend class FTQueueBase;

public:
   FTQueueMessage()
   {
   }

   ~FTQueueMessage()
   {
   }

   FTQueueMessage &operator=(const FTQueueMessage &a)
   {
      m_timer = a.m_timer;
      m_msgType = a.m_msgType;
      return *this;
   }

   virtual Void getLength(ULong &length);
   virtual Void serialize(pVoid pBuffer, ULong &nOffset);
   virtual Void unserialize(pVoid pBuffer, ULong &nOffset);

   FTTime &getTimer()
   {
      return m_timer;
   }

   Void setMsgType(Long msgType) { m_msgType = msgType; }
   Long getMsgType() { return m_msgType; }

private:
   FTTime m_timer;
   Long m_msgType;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTQueuePublic;
class FTQueuePrivate;

class FTQueueBase
{
   friend class FTQueuePublic;
   friend class FTQueuePrivate;

public:
   enum Mode
   {
      ReadOnly,
      WriteOnly,
      ReadWrite
   };

   Bool push(FTQueueMessage &msg, Bool wait = True);
   FTQueueMessage *pop(Bool wait = True);

   //FTSemaphoreBase &getMsgSemaphore() { return semMsgs(); }

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

   virtual FTMutexData &readMutex() = 0;
   virtual FTMutexData &writeMutex() = 0;
   virtual FTSemaphoreData &semMsgs() = 0;
   virtual FTSemaphoreData &semFree() = 0;

   virtual FTQueueMessage *allocMessage(Long msgType) = 0;

   Mode mode() { return m_mode; }

   FTQueueBase();
   ~FTQueueBase();

   Void init(ULong nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
             Bool bMultipleWriters, FTQueueBase::Mode eMode);

private:
   Bool m_initialized;
   Mode m_mode;
   Char m_rBuffer[USHRT_MAX];
   Char m_wBuffer[USHRT_MAX];
   //    FTSemaphoreNotice m_semFreeNotice;
};

#endif // #define __ftqbase_h_included
