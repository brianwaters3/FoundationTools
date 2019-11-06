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

#ifndef __esynch2_h_included
#define __esynch2_h_included

/// @file

#include "esynch.h"
#include "eshmem.h"
#include "eatomic.h"
#include "estatic.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(ESynchObjectsError_PublicObjectsNotEnabled);
DECLARE_ERROR_ADVANCED2(ESynchObjectsError_UnableToAllocateSynchObject);
DECLARE_ERROR(ESynchObjectsError_InvalidOffset);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESharedMemory;

/// @brief Used internally by EpcTools for managing access to public (shared memory) objects.
class ESynchObjects : public EStatic
{
/// @cond DOXYGEN_EXCLUDE
public:
   typedef struct
   {
      EMutexPrivate m_mutex;
      Int m_max;
      Int m_head;
      Long m_currused;
      Long m_maxused;
   } _esynchcontrol_t;

   typedef struct
   {
      Bool m_initialized;
      Long m_sequence;
      _esynchcontrol_t m_semaphoreCtrl;
      _esynchcontrol_t m_mutexCtrl;
   } esynchcontrol_t;

   typedef struct
   {
      Char m_name[EPC_FILENAME_MAX];
      Int m_queueid;
      Int m_msgSize;
      Int m_msgCnt;
      Bool m_multipleReaders;
      Bool m_multipleWriters;
   } epublicqueuedef_t;

   ESynchObjects();
   ~ESynchObjects();

   virtual Int getInitType() { return STATIC_INIT_TYPE_PRIORITY; }
   Void init(EGetOpt &options);
   Void uninit();

   Void logObjectUsage();

   static Int nextSemaphore();
   static Int nextMutex();

   static Void freeSemaphore(Int nSemId);
   static Void freeMutex(Int nMutexId);

   Long incSequence() { return atomic_inc(m_pCtrl->m_sequence); }

   epublicqueuedef_t *getPublicQueue(Int queueid)
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
      epc_strcpy_s(m_pPubQueues[idx].m_name, sizeof(m_pPubQueues[idx].m_name), pName);
      m_pPubQueues[idx].m_queueid = queueid;
      m_pPubQueues[idx].m_msgSize = msgSize;
      m_pPubQueues[idx].m_msgCnt = msgCnt;
      m_pPubQueues[idx].m_multipleReaders = multipleReaders;
      m_pPubQueues[idx].m_multipleWriters = multipleWriters;
   }

   static ESynchObjects *getSynchObjCtrlPtr()
   {
      if (!m_pThis)
         throw ESynchObjectsError_PublicObjectsNotEnabled();
      return m_pThis;
   }

   static ESemaphoreDataPublic &getSemaphore(Int ofs)
   {
      if (ofs < 0)
         throw ESynchObjectsError_InvalidOffset();
      return getSynchObjCtrlPtr()->m_pSemaphores[ofs - 1];
   }

   static EMutexDataPublic &getMutex(Int ofs)
   {
      if (ofs < 0)
         throw ESynchObjectsError_InvalidOffset();
      return getSynchObjCtrlPtr()->m_pMutexes[ofs - 1];
   }
/// @endcond

private:
   class ESynchObjectsSharedMemory : public ESharedMemory
   {
      friend class ESynchObjects;

   protected:
      Void onDestroy();
      Void setSynchObjectsPtr(ESynchObjects *p);

   private:
      ESynchObjects *m_pSynchObjects;
   };

   ESynchObjectsSharedMemory m_sharedmem;
   esynchcontrol_t *m_pCtrl;
   ESemaphoreDataPublic *m_pSemaphores;
   EMutexDataPublic *m_pMutexes;
   epublicqueuedef_t *m_pPubQueues;

   static ESynchObjects *m_pThis;
};

#endif // #define __esynch2_h_included
