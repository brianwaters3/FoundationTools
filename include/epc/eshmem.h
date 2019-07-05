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

#ifndef __eshmem_h_included
#define __eshmem_h_included

#include "esynch.h"

DECLARE_ERROR(ESharedMemoryError_NotInitialized);
DECLARE_ERROR_ADVANCED4(ESharedMemoryError_UnableToCreate);
DECLARE_ERROR_ADVANCED(ESharedMemoryError_UnableToMap);

class ESharedMemoryError_UnableToCreateKeyFile : public EError
{
public:
   ESharedMemoryError_UnableToCreateKeyFile(cpStr pszFile);
   virtual cpStr Name() { return "ESharedMemoryError_UnableToCreateKeyFile"; }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESharedMemory
{
public:
   ESharedMemory();
   ESharedMemory(cpStr file, Int id, Int size);
   ~ESharedMemory();

   Void init(cpStr file, Int id, Int size);

   pVoid getDataPtr()
   {
      return m_pData;
   }

   virtual Void onDestroy()
   {
   }

   Int getUsageCount();

private:
   typedef struct
   {
      Int s_usageCnt;
      EMutexPrivate s_mutex;
   } eshmemctrl_t;

   EMutexPrivate &getMutex()
   {
      return m_pCtrl->s_mutex;
   }

   Char m_szShMem[EPC_FILENAME_MAX + 1];
   Char m_szMutex[EPC_FILENAME_MAX + 1];
   pVoid m_pShMem;
   pVoid m_pData;
   eshmemctrl_t *m_pCtrl;

   Int m_shmid;
   key_t m_key;
};

#endif // #define __eshmem_h_included
