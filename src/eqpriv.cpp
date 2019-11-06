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

#include "eqpriv.h"

EQueuePrivate::EQueuePrivate()
    : m_rmutex(False), m_wmutex(False), m_semFree(0,False), m_semMsgs(0,False)
{
   m_refCnt = 0;
   m_numReaders = 0;
   m_numWriters = 0;
   m_multipleReaders = False;
   m_multipleWriters = False;
   m_msgSize = 0;
   m_msgCnt = 0;
   m_head = 0;
   m_tail = 0;
}

EQueuePrivate::~EQueuePrivate()
{
   destroy();
}

/// @cond DOXYGEN_EXCLUDE

ULong &EQueuePrivate::msgSize()
{
   return m_msgSize;
}

Int &EQueuePrivate::msgCnt()
{
   return m_msgCnt;
}

Long &EQueuePrivate::msgHead()
{
   return m_head;
}

Long &EQueuePrivate::msgTail()
{
   return m_tail;
}

Bool &EQueuePrivate::multipleReaders()
{
   return m_multipleReaders;
}

Bool &EQueuePrivate::multipleWriters()
{
   return m_multipleWriters;
}

Int &EQueuePrivate::numReaders()
{
   return m_numReaders;
}

Int &EQueuePrivate::numWriters()
{
   return m_numWriters;
}

Int &EQueuePrivate::refCnt()
{
   return m_refCnt;
}
pChar EQueuePrivate::data()
{
   return m_pData;
}

Void EQueuePrivate::initReadMutex()
{
   m_rmutex.init();
}

Void EQueuePrivate::initWriteMutex()
{
   m_wmutex.init();
}

Void EQueuePrivate::initSemFree(UInt initialCount)
{
   m_semFree.init(initialCount);
}

Void EQueuePrivate::initSemMsgs(UInt initialCount)
{
   m_semMsgs.init(initialCount);
}

Void EQueuePrivate::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
   m_pData = (pChar)malloc(nSize);
   memset(m_pData, 0, nSize);
}

/// @endcond