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

#ifndef __ftqpriv_h_included
#define __ftqpriv_h_included

#include "ftqbase.h"

class FTQueuePrivate : virtual public FTQueueBase
{
public:
   FTQueuePrivate();
   ~FTQueuePrivate();

   Void init(Int nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
             Bool bMultipleWriters, FTQueueBase::Mode eMode)
   {
      FTQueueBase::init(nMsgSize, nMsgCnt, queueId, bMultipleReaders, bMultipleWriters, eMode);
   }

protected:
   Bool isPublic() { return False; }
   ULong &msgSize();
   Int &msgCnt();
   Long &msgHead();
   Long &msgTail();
   Bool &multipleReaders();
   Bool &multipleWriters();
   Int &numReaders();
   Int &numWriters();
   Int &refCnt();
   pChar data();
   Int ctrlSize();
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize);
   Void initReadMutex();
   Void initWriteMutex();
   Void initSemFree(UInt initialCount);
   Void initSemMsgs(UInt initialCount);

   FTMutexData &readMutex() { return m_rmutex; }
   FTMutexData &writeMutex() { return m_wmutex; }
   FTSemaphoreData &semFree() { return m_semFree; }
   FTSemaphoreData &semMsgs() { return m_semMsgs; }

   virtual FTQueueMessage *allocMessage(Long msgType) = 0;

private:
   Int m_refCnt;
   Int m_numReaders;
   Int m_numWriters;
   Bool m_multipleReaders;
   Bool m_multipleWriters;

   ULong m_msgSize;
   Int m_msgCnt;
   Long m_head; // next location to write
   Long m_tail; // next location to read

   FTMutexPrivate m_rmutex;
   FTMutexPrivate m_wmutex;
   FTSemaphorePrivate m_semFree;
   FTSemaphorePrivate m_semMsgs;

   pChar m_pData;
};

#endif // #define __ftqpriv_h_included
