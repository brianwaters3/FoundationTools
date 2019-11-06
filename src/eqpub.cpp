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

#include "eqpub.h"
#include "esynch2.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

EQueuePublicError_QueueNotFound::EQueuePublicError_QueueNotFound(Int queueid)
{
   setSevere();
   setTextf("Unable to find queueid %d ", queueid);
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EQueuePublic::EQueuePublic()
{
   m_pCtrl = NULL;
   m_pData = NULL;
}

EQueuePublic::~EQueuePublic()
{
   destroy();
}

/// @cond DOXYGEN_EXCLUDE

Void EQueuePublic::init(Int queueid, EQueueBase::Mode mode)
{
   ESynchObjects::epublicqueuedef_t *pQueue = ESynchObjects::getSynchObjCtrlPtr()->getPublicQueue(queueid);

   if (!pQueue)
      throw EQueuePublicError_QueueNotFound(queueid);

   init(pQueue->m_msgSize, pQueue->m_msgCnt, pQueue->m_queueid,
        pQueue->m_multipleReaders, pQueue->m_multipleWriters, mode);
}

ULong &EQueuePublic::msgSize()
{
   return m_pCtrl->m_msgSize;
}

Int &EQueuePublic::msgCnt()
{
   return m_pCtrl->m_msgCnt;
}

Long &EQueuePublic::msgHead()
{
   return m_pCtrl->m_head;
}

Long &EQueuePublic::msgTail()
{
   return m_pCtrl->m_tail;
}

Bool &EQueuePublic::multipleReaders()
{
   return m_pCtrl->m_multipleReaders;
}

Bool &EQueuePublic::multipleWriters()
{
   return m_pCtrl->m_multipleWriters;
}

Int &EQueuePublic::numReaders()
{
   return m_pCtrl->m_numReaders;
}

Int &EQueuePublic::numWriters()
{
   return m_pCtrl->m_numWriters;
}

Int &EQueuePublic::refCnt()
{
   return m_pCtrl->m_refCnt;
}

pChar EQueuePublic::data()
{
   return m_pData;
}

Void EQueuePublic::initReadMutex()
{
   EMutexPublic m;
   m_pCtrl->m_rmutexid = m.mutexId();
   m.detach();
}

Void EQueuePublic::initWriteMutex()
{
   EMutexPublic m;
   m_pCtrl->m_wmutexid = m.mutexId();
   m.detach();
}

Void EQueuePublic::initSemFree(UInt initialCount)
{
   ESemaphorePublic s(initialCount);
   semFreeId() = s.semIndex();
   s.detach();
}

Void EQueuePublic::initSemMsgs(UInt initialCount)
{
   ESemaphorePublic s(initialCount);
   semMsgsId() = s.semIndex();
   s.detach();
}

Void EQueuePublic::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_sharedmem.init(sFile, cId, nSize + sizeof(esharedqueue_ctrl_t));
   m_pCtrl = (esharedqueue_ctrl_t *)m_sharedmem.getDataPtr();
   m_pData = ((pChar)m_sharedmem.getDataPtr()) + sizeof(esharedqueue_ctrl_t);
}

/// @endcond