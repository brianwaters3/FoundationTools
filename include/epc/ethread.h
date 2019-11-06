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

/// @file
/// @brief Contains the definitions for public and private event based threads.

#include "etbase.h"
#include "etq.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Public event based thread where the event queue is stored in shared memory.
/// @details A public thread can have events posted to the event queue from other processes.
class EThreadPublic : public EThreadBase
{
public:
   /// @brief Default constructor.
   EThreadPublic();
   /// @brief Class destructor.
   ~EThreadPublic();

   /// @brief Initializes the thread object.
   /// @param appId identifies the application this thread is associated with.
   /// @param threadId identifies the thread within this application.
   /// @param arg an argument that will be passed through to the internal thread procedure.
   ///   Currently not used.
   /// @param queueSize the maximum number of unprocessed entries in the event queue.
   /// @param suspended if True, the thread is started in a suspended state.
   /// @param stackSize the stack size.
   virtual Void init(Short appId, UShort threadId, pVoid arg, Int queueSize = 16384, Bool suspended = False, Dword stackSize = 0);
   /// @brief Posts a quit event to this thread.
   Void quit();
   /// @brief Posts a suspend event to this thread.
   Void suspend();

   /// @brief Called when processing the EM_INIT event.
   virtual Void onInit();
   /// @brief Called when processing the EM_QUIT event.
   virtual Void onQuit();
   /// @brief Called when processing the EM_SUSPEND event.
   virtual Void onSuspend();

   /// @brief Retrieves the application ID.
   /// @return the application ID.
   Short getAppId() { return m_appId; }
   /// @brief Retrieves the thread ID.
   /// @return the thread ID.
   UShort getThreadId() { return m_threadId; }

   /// @cond DOXYGEN_EXCLUDE
   DECLARE_MESSAGE_MAP()
   /// @endcond

protected:
   /// @brief Called by the internal thread framework to process thread events.
   virtual Void pumpMessages();
   /// @brief Called if there when no other event handler can be identified for an event.
   virtual Void defMessageHandler(EThreadMessage &msg);
   /// @brief Called when a message has been queued to the event queue.
   virtual Void messageQueued();

   /// @brief Retrieves a reference to the internal event queue.
   /// @return a reference to the internal event queue.
   EThreadQueueBase &queue() { return m_queue; }

private:
   Short m_appId;
   UShort m_threadId;
   Int m_queueSize;

   EThreadQueuePublic m_queue;
};
typedef std::shared_ptr<EThreadPublic> EThreadPublicPtr;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Private event based thread where the event queue is allocated from the heap.
/// @details A private thread can only have events posted to the event queue from within the processes.
class EThreadPrivate : public EThreadBase
{
public:
   /// @brief Default constructor.
   EThreadPrivate();
   /// @brief Class destructor.
   ~EThreadPrivate();

   /// @brief Initializes the thread object.
   /// @param appId identifies the application this thread is associated with.
   /// @param threadId identifies the thread within this application.
   /// @param arg an argument that will be passed through to the internal thread procedure.
   ///   Currently not used.
   /// @param queueSize the maximum number of unprocessed entries in the event queue.
   /// @param suspended if True, the thread is started in a suspended state.
   /// @param stackSize the stack size.
   virtual Void init(Short appId, UShort threadId, pVoid arg, Int queueSize = 16384, Bool suspended = False, Dword stackSize = 0);
   /// @brief Posts a quit event to this thread.
   Void quit();
   /// @brief Posts a suspend event to this thread.
   Void suspend();

   /// @brief Called when processing the EM_INIT event.
   virtual Void onInit();
   /// @brief Called when processing the EM_QUIT event.
   virtual Void onQuit();
   /// @brief Called when processing the EM_SUSPEND event.
   virtual Void onSuspend();

   /// @brief Retrieves the application ID.
   /// @return the application ID.
   Short getAppId() { return m_appId; }
   /// @brief Retrieves the thread ID.
   /// @return the thread ID.
   UShort getThreadId() { return m_threadId; }

   /// @cond DOXYGEN_EXCLUDE
   DECLARE_MESSAGE_MAP()
   /// @endcond

protected:
   /// @brief Called by the internal thread framework to process thread events.
   virtual Void pumpMessages();
   /// @brief Called if there when no other event handler can be identified for an event.
   virtual Void defMessageHandler(EThreadMessage &msg);
   /// @brief Called when a message has been queued to the event queue.
   virtual Void messageQueued();

   /// @brief Retrieves a reference to the internal event queue.
   /// @return a reference to the internal event queue.
   EThreadQueueBase &queue() { return m_queue; }

private:
   Short m_appId;
   UShort m_threadId;
   Int m_queueSize;

   EThreadQueuePrivate m_queue;
};
typedef std::shared_ptr<EThreadPrivate> EThreadPrivatePtr;

#endif // #define __ethread_h_included
