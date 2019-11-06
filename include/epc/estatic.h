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

#ifndef __estatic_h_included
#define __estatic_h_included

/// @file
/// @brief Performs static initialization associated with any
///        EpcTools class that requires it.  Initialization and
///        uninitialization is performed by EpcTools::Initialize()
///        and EpcTools::UnInitialize().

#include "ebase.h"
#include "egetopt.h"

/// @brief Static initialization priorities.  Initialization is
///        performed in ascending order.
enum
{
   /// @cond DOXYGEN_EXCLUDE
   STATIC_INIT_TYPE_SHARED_OBJECT_MANAGER,
   STATIC_INIT_TYPE_PRIORITY,
   STATIC_INIT_TYPE_THREADS, // initialized thread info

   STATIC_INIT_TYPE_STRING_MANAGER,
   STATIC_INIT_TYPE_FILE_MANAGER,
   STATIC_INIT_TYPE_DLLS,
   STATIC_INIT_TYPE_BEFORE_OTHER,
   STATIC_INIT_TYPE_OTHER,
   STATIC_INIT_TYPE_AFTER_ALL //For anything that relies upon others
   /// @endcond
};

/// @brief Any EpcTools that needs to have initialization performed
///        as part of EpcTools::Initialize() should derive from this
///        class and provide implementations for init() and uninit().
class EStatic
{
public:
   static Void Initialize(EGetOpt &opt);
   static Void UnInitialize();

   virtual Int getInitType() { return STATIC_INIT_TYPE_OTHER; }

   /// @brief Performs class specific initialization.
   /// @param opt reference to the options object.
   virtual Void init(EGetOpt &opt) {}
   /// @brief Performs uninitialization at system shutdown.
   virtual Void uninit() {}

   /// @cond DOXYGEN_EXCLUDE
   EStatic();
   virtual ~EStatic();
   /// @endcond

protected:
   /// @cond DOXYGEN_EXCLUDE
   static EStatic *m_pFirst;
   EStatic *m_pNext;
   /// @endcond
};

#endif // #define __estatic_h_included
