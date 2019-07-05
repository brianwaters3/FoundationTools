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

#ifndef __ethread_h_included
#define __ethread_h_included

#include "etbase.h"
#include "etq.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadPublic : public EThreadBase
{
public:
   EThreadPublic();
   ~EThreadPublic();

   Bool sendMessage(UInt message, Bool wait = True);
   Bool sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait = True);
   Bool sendMessage(UInt message, pVoid voidPtr, Bool wait = True);
   Bool sendMessage(UInt message, LongLong quadPart, Bool wait = True);

   virtual Void init(Short appId, UShort threadId, pVoid arg, Int queueSize = 16384, Bool suspended = False, Dword stackSize = 0);
   Void quit();
   Void suspend();

   virtual Void onInit();
   virtual Void onQuit();
   virtual Void onSuspend();

   Short getAppId() { return m_appId; }
   UShort getThreadId() { return m_threadId; }

   DECLARE_MESSAGE_MAP()

protected:
   virtual Void pumpMessages();
   virtual Void defMessageHandler(EThreadMessage &msg);
   virtual Void messageQueued();

   EThreadQueueBase &queue() { return m_queue; }

private:
   Short m_appId;
   UShort m_threadId;
   Int m_queueSize;

   EThreadQueuePublic m_queue;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadPrivate : public EThreadBase
{
public:
   EThreadPrivate();
   ~EThreadPrivate();

   Bool sendMessage(UInt message, Bool wait = True);
   Bool sendMessage(UInt message, Dword lowPart, Long highPart, Bool wait = True);
   Bool sendMessage(UInt message, pVoid voidPtr, Bool wait = True);
   Bool sendMessage(UInt message, LongLong quadPart, Bool wait = True);

   virtual Void init(Short appId, UShort threadId, pVoid arg, Int queueSize = 16384, Bool suspended = False, Dword stackSize = 0);
   Void quit();
   Void suspend();

   virtual Void onInit();
   virtual Void onQuit();
   virtual Void onSuspend();

   Short getAppId() { return m_appId; }
   UShort getThreadId() { return m_threadId; }

   DECLARE_MESSAGE_MAP()

protected:
   virtual Void pumpMessages();
   virtual Void defMessageHandler(EThreadMessage &msg);
   virtual Void messageQueued();

   EThreadQueueBase &queue() { return m_queue; }

private:
   Short m_appId;
   UShort m_threadId;
   Int m_queueSize;

   EThreadQueuePrivate m_queue;
};

#endif // #define __ethread_h_included
