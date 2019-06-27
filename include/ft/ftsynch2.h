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

#ifndef __ftsynch2_h_included
#define __ftsynch2_h_included

#include "ftsynch.h"
#include "ftshmem.h"
#include "ftatomic.h"
#include "ftstatic.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(FTSynchObjectsError_PublicObjectsNotEnabled);
DECLARE_ERROR_ADVANCED2(FTSynchObjectsError_UnableToAllocateSynchObject);
DECLARE_ERROR(FTSynchObjectsError_InvalidOffset);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTSharedMemory;

class FTSynchObjects : public FTStatic
{
public:
   typedef struct
   {
      FTMutexPrivate m_mutex;
      Int m_max;
      Int m_head;
      Long m_currused;
      Long m_maxused;
   } _ftsynchcontrol_t;

   typedef struct
   {
      Bool m_initialized;
      Long m_sequence;
      _ftsynchcontrol_t m_semaphoreCtrl;
      _ftsynchcontrol_t m_mutexCtrl;
   } ftsynchcontrol_t;

   typedef struct
   {
      Char m_name[FT_FILENAME_MAX];
      Int m_queueid;
      Int m_msgSize;
      Int m_msgCnt;
      Bool m_multipleReaders;
      Bool m_multipleWriters;
   } ftpublicqueuedef_t;

   FTSynchObjects();
   ~FTSynchObjects();

   virtual Int getInitType() { return STATIC_INIT_TYPE_SHARED_OBJECT_MANAGER; }
   Void init(FTGetOpt &options);
   Void uninit();

   Void logObjectUsage();

   static Int nextSemaphore();
   static Int nextMutex();

   static Void freeSemaphore(Int nSemId);
   static Void freeMutex(Int nMutexId);

   Long incSequence() { return atomic_inc(m_pCtrl->m_sequence); }

   ftpublicqueuedef_t *getPublicQueue(Int queueid)
   {
      for (int i = 0; m_pPubQueues[i].m_name[0] != 0; i++)
      {
         if (m_pPubQueues[i].m_queueid == queueid)
            return &m_pPubQueues[i];
      }

      return NULL;
   }

   Void setPublicQueue(Int idx, cpChar pName, Int queueid, Int msgSize,
                       Int msgCnt, Bool multipleReaders, Bool multipleWriters)
   {
      ft_strcpy_s(m_pPubQueues[idx].m_name, sizeof(m_pPubQueues[idx].m_name), pName);
      m_pPubQueues[idx].m_queueid = queueid;
      m_pPubQueues[idx].m_msgSize = msgSize;
      m_pPubQueues[idx].m_msgCnt = msgCnt;
      m_pPubQueues[idx].m_multipleReaders = multipleReaders;
      m_pPubQueues[idx].m_multipleWriters = multipleWriters;
   }

   static FTSynchObjects *getSynchObjCtrlPtr()
   {
      if (!m_pThis)
         throw FTSynchObjectsError_PublicObjectsNotEnabled();
      return m_pThis;
   }

   static FTSemaphoreDataPublic &getSemaphore(Int ofs)
   {
      if (ofs < 0)
         throw FTSynchObjectsError_InvalidOffset();
      return getSynchObjCtrlPtr()->m_pSemaphores[ofs - 1];
   }

   static FTMutexDataPublic &getMutex(Int ofs)
   {
      if (ofs < 0)
         throw FTSynchObjectsError_InvalidOffset();
      return getSynchObjCtrlPtr()->m_pMutexes[ofs - 1];
   }

private:
   class FTSynchObjectsSharedMemory : public FTSharedMemory
   {
      friend class FTSynchObjects;

   protected:
      Void onDestroy();
      Void setSynchObjectsPtr(FTSynchObjects *p);

   private:
      FTSynchObjects *m_pSynchObjects;
   };

   FTSynchObjectsSharedMemory m_sharedmem;
   ftsynchcontrol_t *m_pCtrl;
   FTSemaphoreDataPublic *m_pSemaphores;
   FTMutexDataPublic *m_pMutexes;
   ftpublicqueuedef_t *m_pPubQueues;

   static FTSynchObjects *m_pThis;
};

#endif // #define __ftsynch2_h_included
