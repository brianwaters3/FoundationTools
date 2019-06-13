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

#include "../include/ft/ftthread.h"

BEGIN_MESSAGE_MAP(FTThreadPublic, FTThreadBase)
END_MESSAGE_MAP()

FTThreadPublic::FTThreadPublic()
{
    m_appId = 1;
    m_threadId = 1;
    m_queueSize = 16384;
}

FTThreadPublic::~FTThreadPublic()
{
}

Bool FTThreadPublic::sendMessage(UInt message, Bool wait)
{
    return FTThreadBase::sendMessage(message, wait);
}

Bool FTThreadPublic::sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait)
{
    return FTThreadBase::sendMessage(message, lowPart, highPart, wait);
}

Bool FTThreadPublic::sendMessage(UInt message, pVoid voidPtr, Bool wait)
{
    return FTThreadBase::sendMessage(message, voidPtr, wait);
}

Bool FTThreadPublic::sendMessage(UInt message, LongLong quadPart, Bool wait)
{
    return FTThreadBase::sendMessage(message, quadPart, wait);
}

Void FTThreadPublic::init(Short appId, UShort threadId, pVoid arg, Int queueSize, Bool suspended, Dword stackSize)
{
    m_appId = appId;
    m_threadId = threadId;
    m_queueSize = queueSize;

    long id = m_appId * 10000 + m_threadId;

    m_queue.init(m_queueSize, id, True, FTThreadQueueBase::ReadWrite);

    FTThreadBase::init(arg, suspended, stackSize);
}

Void FTThreadPublic::quit()
{
    FTThreadBase::quit();
}

Void FTThreadPublic::suspend()
{
    FTThreadBase::suspend();
}

Void FTThreadPublic::onInit()
{
    FTThreadBase::onInit();
}

Void FTThreadPublic::onQuit()
{
    FTThreadBase::onQuit();
}

Void FTThreadPublic::onSuspend()
{
    FTThreadBase::onSuspend();
}

Void FTThreadPublic::pumpMessages()
{
    FTThreadBase::pumpMessages();
}

Void FTThreadPublic::defMessageHandler(FTThreadMessage &msg)
{
    FTThreadBase::defMessageHandler(msg);
}

Void FTThreadPublic::messageQueued()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(FTThreadPrivate, FTThreadBase)
END_MESSAGE_MAP()

FTThreadPrivate::FTThreadPrivate()
{
    m_appId = 1;
    m_threadId = 1;
    m_queueSize = 16384;
}

FTThreadPrivate::~FTThreadPrivate()
{
}

Bool FTThreadPrivate::sendMessage(UInt message, Bool wait)
{
    return FTThreadBase::sendMessage(message, wait);
}

Bool FTThreadPrivate::sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait)
{
    return FTThreadBase::sendMessage(message, lowPart, highPart, wait);
}

Bool FTThreadPrivate::sendMessage(UInt message, pVoid voidPtr, Bool wait)
{
    return FTThreadBase::sendMessage(message, voidPtr, wait);
}

Bool FTThreadPrivate::sendMessage(UInt message, LongLong quadPart, Bool wait)
{
    return FTThreadBase::sendMessage(message, quadPart, wait);
}

Void FTThreadPrivate::init(Short appId, UShort threadId, pVoid arg, Int queueSize, Bool suspended, Dword stackSize)
{
    m_appId = appId;
    m_threadId = threadId;
    m_queueSize = queueSize;

    long id = m_appId * 10000 + m_threadId;

    m_queue.init(m_queueSize, id, True, FTThreadQueueBase::ReadWrite);

    FTThreadBase::init(arg, suspended, stackSize);
}

Void FTThreadPrivate::quit()
{
    FTThreadBase::quit();
}

Void FTThreadPrivate::suspend()
{
    FTThreadBase::suspend();
}

Void FTThreadPrivate::onInit()
{
    FTThreadBase::onInit();
}

Void FTThreadPrivate::onQuit()
{
    FTThreadBase::onQuit();
}

Void FTThreadPrivate::onSuspend()
{
    FTThreadBase::onSuspend();
}

Void FTThreadPrivate::pumpMessages()
{
    FTThreadBase::pumpMessages();
}

Void FTThreadPrivate::defMessageHandler(FTThreadMessage &msg)
{
    FTThreadBase::defMessageHandler(msg);
}

Void FTThreadPrivate::messageQueued()
{
}
