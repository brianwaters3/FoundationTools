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

#include "ftqpub.h"
#include "ftsynch2.h"

FTQueuePublicError_QueueNotFound::FTQueuePublicError_QueueNotFound(Int queueid)
{
   setSevere();
   setTextf("Unable to find queueid %d ", queueid);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTQueuePublic::FTQueuePublic()
{
   m_pCtrl = NULL;
   m_pData = NULL;
}

FTQueuePublic::~FTQueuePublic()
{
   destroy();
}

Void FTQueuePublic::init(Int queueid, FTQueueBase::Mode mode)
{
   FTSynchObjects::ftpublicqueuedef_t *pQueue = FTSynchObjects::getSynchObjCtrlPtr()->getPublicQueue(queueid);

   if (!pQueue)
      throw new FTQueuePublicError_QueueNotFound(queueid);

   init(pQueue->m_msgSize, pQueue->m_msgCnt, pQueue->m_queueid,
        pQueue->m_multipleReaders, pQueue->m_multipleWriters, mode);
}

ULong &FTQueuePublic::msgSize()
{
   return m_pCtrl->m_msgSize;
}

Int &FTQueuePublic::msgCnt()
{
   return m_pCtrl->m_msgCnt;
}

Long &FTQueuePublic::msgHead()
{
   return m_pCtrl->m_head;
}

Long &FTQueuePublic::msgTail()
{
   return m_pCtrl->m_tail;
}

Bool &FTQueuePublic::multipleReaders()
{
   return m_pCtrl->m_multipleReaders;
}

Bool &FTQueuePublic::multipleWriters()
{
   return m_pCtrl->m_multipleWriters;
}

Int &FTQueuePublic::numReaders()
{
   return m_pCtrl->m_numReaders;
}

Int &FTQueuePublic::numWriters()
{
   return m_pCtrl->m_numWriters;
}

Int &FTQueuePublic::refCnt()
{
   return m_pCtrl->m_refCnt;
}

pChar FTQueuePublic::data()
{
   return m_pData;
}

Void FTQueuePublic::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_sharedmem.init(sFile, cId, nSize + sizeof(ftsharedqueue_ctrl_t));
   m_pCtrl = (ftsharedqueue_ctrl_t *)m_sharedmem.getDataPtr();
   m_pData = ((pChar)m_sharedmem.getDataPtr()) + sizeof(ftsharedqueue_ctrl_t);
}
