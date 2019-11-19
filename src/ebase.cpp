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

#include "einternal.h"
#include "etbasic.h"
#include "esynch2.h"

Void EpcTools::Initialize(EGetOpt &options)
{
   options.setPrefix(SECTION_TOOLS);
   m_debug = options.get(MEMBER_DEBUG, false);
   m_public = options.get(MEMBER_ENABLE_PUBLIC_OBJECTS, false);
   options.setPrefix("");

   EStatic::Initialize(options);
   EThreadBasic::Initialize();
}

Void EpcTools::UnInitialize()
{
   //ESynchObjects::getSynchObjCtrlPtr()->logObjectUsage();
   EThreadBasic::UnInitialize();
   EStatic::UnInitialize();
}

Int EpcTools::m_internalLogId = -1;
Int EpcTools::m_appid = 0;
Bool EpcTools::m_public = False;
Bool EpcTools::m_debug = False;
