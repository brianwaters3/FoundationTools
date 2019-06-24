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

#include "ftinternal.h"
#include "fttbase.h"
#include "ftsynch2.h"

Void FoundationTools::Initialize(FTGetOpt &options)
{
   FTStatic::Initialize(options);
   FTThreadBasic::Initialize();
}

Void FoundationTools::UnInitialize()
{
   FTSynchObjects::getSynchObjCtrlPtr()->logObjectUsage();

   FTThreadBasic::UnInitialize();
   FTStatic::UnInitialize();
}

Int FoundationTools::m_internalLogId = -1;
Int FoundationTools::m_appid = 0;
