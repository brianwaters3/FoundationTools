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

#ifndef __eqpriv_h_included
#define __eqpriv_h_included

/// @file
/// @brief Contains the private queue class definition

#include "eqbase.h"

/// @brief The private queue class.
class EQueuePrivate : virtual public EQueueBase
{
public:
   /// @brief Default constructor.
   EQueuePrivate();
   /// @brief Class destructor.
   ~EQueuePrivate();

   /// @brief Initializes the private message queue.
   /// @param nMsgSize the maximum message size.
   /// @param nMsgCnt the maximum number of messages that can be in the queue at one time.
   /// @param queueId the queue identifier.
   /// @param bMultipleReaders indicates whether or not that there are multiple readers.
   /// @param bMultipleWriters indicates whether or not that there are multiple writers.
   /// @param eMode the open mode.
   Void init(Int nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
             Bool bMultipleWriters, EQueueBase::Mode eMode)
   {
      EQueueBase::init(nMsgSize, nMsgCnt, queueId, bMultipleReaders, bMultipleWriters, eMode);
   }

protected:
   /// @cond DOXYGEN_EXCLUDE
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

   EMutexData &readMutex() { return m_rmutex; }
   EMutexData &writeMutex() { return m_wmutex; }
   ESemaphoreData &semFree() { return m_semFree; }
   ESemaphoreData &semMsgs() { return m_semMsgs; }

   virtual EQueueMessage *allocMessage(Long msgType) = 0;
   /// @endcond

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

   EMutexPrivate m_rmutex;
   EMutexPrivate m_wmutex;
   ESemaphorePrivate m_semFree;
   ESemaphorePrivate m_semMsgs;

   pChar m_pData;
};

#endif // #define __eqpriv_h_included
