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

#include "ethread.h"

BEGIN_MESSAGE_MAP(EThreadPublic, EThreadBase)
END_MESSAGE_MAP()

EThreadPublic::EThreadPublic()
{
   m_appId = 1;
   m_threadId = 1;
   m_queueSize = 16384;
}

EThreadPublic::~EThreadPublic()
{
}

Void EThreadPublic::init(Short appId, UShort threadId, pVoid arg, Int queueSize, Bool suspended, Dword stackSize)
{
   m_appId = appId;
   m_threadId = threadId;
   m_queueSize = queueSize;

   long id = m_appId * 10000 + m_threadId;

   m_queue.init(m_queueSize, id, True, EThreadQueueBase::ReadWrite);

   EThreadBase::init(arg, suspended, stackSize);
}

Void EThreadPublic::quit()
{
   EThreadBase::quit();
}

Void EThreadPublic::suspend()
{
   EThreadBase::suspend();
}

Void EThreadPublic::onInit()
{
   EThreadBase::onInit();
}

Void EThreadPublic::onQuit()
{
   EThreadBase::onQuit();
}

Void EThreadPublic::onSuspend()
{
   EThreadBase::onSuspend();
}

Void EThreadPublic::pumpMessages()
{
   EThreadBase::pumpMessages();
}

Void EThreadPublic::defMessageHandler(EThreadMessage &msg)
{
   EThreadBase::defMessageHandler(msg);
}

Void EThreadPublic::messageQueued()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(EThreadPrivate, EThreadBase)
END_MESSAGE_MAP()

EThreadPrivate::EThreadPrivate()
{
   m_appId = 1;
   m_threadId = 1;
   m_queueSize = 16384;
}

EThreadPrivate::~EThreadPrivate()
{
}

// Bool EThreadPrivate::sendMessage(UInt message, Bool wait)
// {
//    return EThreadBase::sendMessage(message, wait);
// }

// Bool EThreadPrivate::sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait)
// {
//    return EThreadBase::sendMessage(message, lowPart, highPart, wait);
// }

// Bool EThreadPrivate::sendMessage(UInt message, pVoid voidPtr, Bool wait)
// {
//    return EThreadBase::sendMessage(message, voidPtr, wait);
// }

// Bool EThreadPrivate::sendMessage(UInt message, LongLong quadPart, Bool wait)
// {
//    return EThreadBase::sendMessage(message, quadPart, wait);
// }

Void EThreadPrivate::init(Short appId, UShort threadId, pVoid arg, Int queueSize, Bool suspended, Dword stackSize)
{
   m_appId = appId;
   m_threadId = threadId;
   m_queueSize = queueSize;

   long id = m_appId * 10000 + m_threadId;

   m_queue.init(m_queueSize, id, True, EThreadQueueBase::ReadWrite);

   EThreadBase::init(arg, suspended, stackSize);
}

Void EThreadPrivate::quit()
{
   EThreadBase::quit();
}

Void EThreadPrivate::suspend()
{
   EThreadBase::suspend();
}

Void EThreadPrivate::onInit()
{
   EThreadBase::onInit();
}

Void EThreadPrivate::onQuit()
{
   EThreadBase::onQuit();
}

Void EThreadPrivate::onSuspend()
{
   EThreadBase::onSuspend();
}

Void EThreadPrivate::pumpMessages()
{
   EThreadBase::pumpMessages();
}

Void EThreadPrivate::defMessageHandler(EThreadMessage &msg)
{
   EThreadBase::defMessageHandler(msg);
}

Void EThreadPrivate::messageQueued()
{
}
