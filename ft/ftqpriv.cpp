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

#include "ftqpriv.h"

FTQueuePrivate::FTQueuePrivate()
    : m_rmutex(False), m_wmutex(False)
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

FTQueuePrivate::~FTQueuePrivate()
{
    destroy();
}

ULong& FTQueuePrivate::msgSize()
{
    return m_msgSize;
}

Int& FTQueuePrivate::msgCnt()
{
    return m_msgCnt;
}

Long& FTQueuePrivate::msgHead()
{
    return m_head;
}

Long& FTQueuePrivate::msgTail()
{
    return m_tail;
}

Bool& FTQueuePrivate::multipleReaders()
{
    return m_multipleReaders;
}

Bool& FTQueuePrivate::multipleWriters()
{
    return m_multipleWriters;
}

Int& FTQueuePrivate::numReaders()
{
    return m_numReaders;
}

Int& FTQueuePrivate::numWriters()
{
    return m_numWriters;
}

Int& FTQueuePrivate::refCnt()
{
    return m_refCnt;
}
pChar FTQueuePrivate::data()
{
    return m_pData;
}

Void FTQueuePrivate::allocDataSpace(cpStr sFile, Char cId, Int nSize)
{
    m_pData = (pChar)malloc(nSize);
    memset(m_pData, 0, nSize);
}
