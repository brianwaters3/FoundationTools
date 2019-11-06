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

#ifndef __eqpub_h_included
#define __eqpub_h_included

#include "eqbase.h"
#include "eshmem.h"
#include "esynch2.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED2(EQueuePublicError_QueueNotFound);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EQueuePublic : public EQueueBase
{
public:
   /// @brief Default constructor.
   EQueuePublic();
   /// @brief Class destructor.
   ~EQueuePublic();

   /// @brief Initializes the queue object.
   /// @param queueid the queue identifier.
   /// @param mode the open mode.
   Void init(Int queueid, EQueueBase::Mode mode);

protected:
   /// @cond DOXYGEN_EXCLUDE
   Bool isPublic() { return True; }
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

   EMutexData &readMutex() { return ESynchObjects::getMutex(readMutexId()); }
   EMutexData &writeMutex() { return ESynchObjects::getMutex(writeMutexId()); }
   ESemaphoreData &semFree() { return ESynchObjects::getSemaphore(semFreeId()); }
   ESemaphoreData &semMsgs() { return ESynchObjects::getSemaphore(semMsgsId()); }

   virtual EQueueMessage *allocMessage(Long msgType) = 0;

   Void init(Int nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
             Bool bMultipleWriters, EQueueBase::Mode eMode)
   {
      EQueueBase::init(nMsgSize, nMsgCnt, queueId, bMultipleReaders, bMultipleWriters, eMode);
   }
   /// @endcond

private:
   typedef struct
   {
      Int m_refCnt;
      Int m_numReaders;
      Int m_numWriters;
      Bool m_multipleReaders;
      Bool m_multipleWriters;

      ULong m_msgSize;
      Int m_msgCnt;
      Long m_head; // next location to write
      Long m_tail; // next location to read
      Int m_rmutexid;
      Int m_wmutexid;
      Int m_semfreeid;
      Int m_semmsgsid;
   } esharedqueue_ctrl_t;

   Int &readMutexId() { return m_pCtrl->m_rmutexid; }
   Int &writeMutexId() { return m_pCtrl->m_wmutexid; }

   Int &semFreeId() { return m_pCtrl->m_semfreeid; }
   Int &semMsgsId() { return m_pCtrl->m_semmsgsid; }

   ESharedMemory m_sharedmem;
   esharedqueue_ctrl_t *m_pCtrl;
   pChar m_pData;
};

#endif // #define __eqpub_h_included
