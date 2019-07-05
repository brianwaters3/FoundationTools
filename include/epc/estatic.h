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

#include "ebase.h"
#include "egetopt.h"

enum
{
   STATIC_INIT_TYPE_SHARED_OBJECT_MANAGER,
   STATIC_INIT_TYPE_PRIORITY,
   STATIC_INIT_TYPE_THREADS, // initialized thread info

   STATIC_INIT_TYPE_STRING_MANAGER,
   STATIC_INIT_TYPE_FILE_MANAGER,
   STATIC_INIT_TYPE_DLLS,
   STATIC_INIT_TYPE_BEFORE_OTHER,
   STATIC_INIT_TYPE_OTHER,
   STATIC_INIT_TYPE_AFTER_ALL //For anything that relies upon others
};

class EStatic
{
public:
   static Void Initialize(EGetOpt &opt);
   static Void UnInitialize();

   virtual Int getInitType() { return STATIC_INIT_TYPE_OTHER; }

   virtual Void init(EGetOpt &opt) {}
   virtual Void uninit() {}

   EStatic();
   virtual ~EStatic();

protected:
   static EStatic *m_pFirst;
   EStatic *m_pNext;
};

#endif // #define __estatic_h_included
