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

#ifndef __ftshmem_h_included
#define __ftshmem_h_included

#include "ftsynch.h"

DECLARE_ERROR(FTSharedMemoryError_NotInitialized);
DECLARE_ERROR_ADVANCED4(FTSharedMemoryError_UnableToCreate);
DECLARE_ERROR_ADVANCED(FTSharedMemoryError_UnableToMap);

class FTSharedMemoryError_UnableToCreateKeyFile : public FTError
{
public:
   FTSharedMemoryError_UnableToCreateKeyFile(cpStr pszFile);
   virtual cpStr Name() { return "FTSharedMemoryError_UnableToCreateKeyFile"; }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTSharedMemory
{
public:
   FTSharedMemory();
   FTSharedMemory(cpStr file, Int id, Int size);
   ~FTSharedMemory();

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
      _FTMutex s_mutex;
   } ftshmemctrl_t;

   _FTMutex &getMutex()
   {
      return m_pCtrl->s_mutex;
   }

   Char m_szShMem[FT_FILENAME_MAX + 1];
   Char m_szMutex[FT_FILENAME_MAX + 1];
   pVoid m_pShMem;
   pVoid m_pData;
   ftshmemctrl_t *m_pCtrl;

   Int m_shmid;
   key_t m_key;
};

#endif // #define __ftshmem_h_included
