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

/// @file
/// @brief Defines a class for access to shared memory.

#include "esynch.h"

/// @cond DOXYGEN_EXCLUDE
DECLARE_ERROR(ESharedMemoryError_NotInitialized);
DECLARE_ERROR_ADVANCED4(ESharedMemoryError_UnableToCreate);
DECLARE_ERROR_ADVANCED(ESharedMemoryError_UnableToMap);

class ESharedMemoryError_UnableToCreateKeyFile : public EError
{
public:
   ESharedMemoryError_UnableToCreateKeyFile(cpStr pszFile);
   virtual const cpStr Name() const { return "ESharedMemoryError_UnableToCreateKeyFile"; }
};
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief The shared memory access class.
class ESharedMemory
{
public:
   /// @brief Default constructor.
   ESharedMemory();
   /// @brief Class constructor.
   /// @param file the file name associated with the shared memory.
   /// @param id the identifier for the shared memory.
   /// @param size the amount of memory to allocate for this shared memory object.
   ESharedMemory(cpStr file, Int id, Int size);
   /// @brief Class destructor.
   ~ESharedMemory();

   /// @brief Initializes this shared memory object.  This method is called by the
   ///    parameterized constructor.
   /// @param file the file name associated with the shared memory.
   /// @param id the identifier for the shared memory.
   /// @param size the amount of memory to allocate for this shared memory object.
   Void init(cpStr file, Int id, Int size);

   /// @brief Retrieves a pointer to the first location of the shated memory.
   /// @return a pointer to the first location of the shated memory.
   pVoid getDataPtr()
   {
      return m_pData;
   }

   /// @brief Called when teh shared memory is destroyed.
   virtual Void onDestroy()
   {
   }

   /// @brief Retrieves the number of clients accessing the shared memory.
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
