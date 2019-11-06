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

#include "estatic.h"

/// @cond DOXYGEN_EXCLUDE
EStatic *EStatic::m_pFirst = NULL;
/// @endcond

Void EStatic::Initialize(EGetOpt &opt)
{
   for (Int t = 0; t <= STATIC_INIT_TYPE_AFTER_ALL; t++)
   {
      for (EStatic *p = EStatic::m_pFirst; p != NULL; p = p->m_pNext)
      {
         if (p->getInitType() == t)
            p->init(opt);
      }
   }
}

void EStatic::UnInitialize()
{
   for (Int t = STATIC_INIT_TYPE_AFTER_ALL; t >= 0; t--)
   {
      for (EStatic *p = EStatic::m_pFirst; p != NULL; p = p->m_pNext)
      {
         if (p->getInitType() == t)
            p->uninit();
      }
   }
}

/// @cond DOXYGEN_EXCLUDE

EStatic::~EStatic()
{
   EStatic *last = NULL;

   for (EStatic *p = m_pFirst; p != NULL && p != this; p = p->m_pNext)
      last = p;

   if (last)
      last->m_pNext = m_pNext;
   else
      m_pFirst = m_pNext;

   m_pNext = NULL;
}

EStatic::EStatic()
{
   m_pNext = m_pFirst;
   m_pFirst = this;
}

/// @endcond
