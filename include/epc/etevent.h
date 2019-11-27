/*
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

#ifndef __ETEVENT_H
#define __ETEVENT_H

#include <unistd.h>
#include <sys/syscall.h>

#include "ebase.h"
#include "etbasic.h"
#include "eerror.h"
#include "egetopt.h"
#include "eshmem.h"
#include "esynch.h"
#include "esynch2.h"
#include "etimer.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

DECLARE_ERROR(EThreadQueueBaseError_NotOpenForWriting);
DECLARE_ERROR(EThreadQueueBaseError_NotOpenForReading);
DECLARE_ERROR(EThreadQueueBaseError_MultipleReadersNotAllowed);
DECLARE_ERROR(EThreadQueuePublicError_UnInitialized);

DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToInitialize);
DECLARE_ERROR_ADVANCED(EThreadTimerError_NotInitialized);
DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToStart);
DECLARE_ERROR_ADVANCED(EThreadTimerError_UnableToRegisterTimerHandler);

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief The base class that all event message data objects should
///   be derived from.
/// @brief This class contains functionality that should be provided by all
///   event message data objects.
class EThreadEventMessageDataBase
{
public:
   /// @brief Default constructor.
   EThreadEventMessageDataBase() : m_msgid() {}
   /// @brief Class constructor.
   /// @param msgid the event message ID.
   EThreadEventMessageDataBase(UInt msgid) : m_msgid(msgid) {}

   /// @brief Retrieves the event message ID.
   /// @return the event message ID.
   UInt getMessageId() { return m_msgid; }
   /// @brief Sets the event message ID.
   /// @param msgid the event message ID.
   /// @return a reference to this object.
   EThreadEventMessageDataBase &setMessageId(UInt msgid) { m_msgid = msgid; return *this; }
   /// @brief Retrieves the void pointer.
   /// @details This virtual method must be defined in the derived class.
   /// @return the void pointer.
   virtual pVoid getVoidPtr() = 0;
   /// @brief Sets the void pointer.
   /// @param p the void pointer.
   /// @return a reference to this object.
   virtual Void setVoidPtr(pVoid p) = 0;

   /// @brief Retrieves the timer associated with this event message.
   /// @details This timer is started when the message is inserted into the thread
   ///   event queue.
   /// @return the timer associated with this event message.
   ETimer &getTimer() { return m_timer; }

private:   
   ETimer m_timer;
   UInt m_msgid;
};

/// @brief An event message data object that provides access to a union over 8 bytes of data.
class EThreadEventMessageData : public EThreadEventMessageDataBase
{
public:
   /// @cond DOXYGEN_EXCLUDE
   typedef union {
      pVoid       voidptr;
      LongLong    int64;
      Long        int32[sizeof(pVoid) / sizeof(Long)];
      Short       int16[sizeof(pVoid) / sizeof(Short)];
      Char        int8[sizeof(pVoid) / sizeof(Char)];
      ULongLong   uint64;
      ULong       uint32[sizeof(pVoid) / sizeof(ULong)];
      UShort      uint16[sizeof(pVoid) / sizeof(UShort)];
      UChar       uint8[sizeof(pVoid) / sizeof(UChar)];
   } DataUnion;
   /// @endcond

   /// @brief Default constructor.
   EThreadEventMessageData() : EThreadEventMessageDataBase(), m_data() {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   EThreadEventMessageData(UInt msgid) : EThreadEventMessageDataBase(msgid), m_data() {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param data a data object to include in the message (contains the union definition).
   EThreadEventMessageData(UInt msgid, EThreadEventMessageData &data) : EThreadEventMessageDataBase(msgid), m_data(data.data()) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v a 64-bit signed integer to include in the event message.
   EThreadEventMessageData(UInt msgid, pVoid v) : EThreadEventMessageDataBase(msgid), m_data{.voidptr=v} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v a 64-bit signed integer to include in the event message.
   EThreadEventMessageData(UInt msgid, LongLong v) : EThreadEventMessageDataBase(msgid), m_data{.int64=v} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v a 64-bit unsigned integer to include in the event message.
   EThreadEventMessageData(UInt msgid, ULongLong v) : EThreadEventMessageDataBase(msgid), m_data{.uint64=v} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 32-bit signed integer to include in the event message.
   /// @param v2 the second 32-bit signed integer to include in the event message.
   EThreadEventMessageData(UInt msgid, Long v1, Long v2) : EThreadEventMessageDataBase(msgid), m_data{.int32={v1,v2}} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 32-bit unsigned integer to include in the event message.
   /// @param v2 the second 32-bit unsigned integer to include in the event message.
   EThreadEventMessageData(UInt msgid, ULong v1, ULong v2) : EThreadEventMessageDataBase(msgid), m_data{.uint32={v1,v2}} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 16-bit signed integer to include in the event message.
   /// @param v2 the second 16-bit signed integer to include in the event message.
   /// @param v3 the third 16-bit signed integer to include in the event message.
   /// @param v4 the fourth 16-bit signed integer to include in the event message.
   EThreadEventMessageData(UInt msgid, Short v1, Short v2, Short v3, Short v4) : EThreadEventMessageDataBase(msgid), m_data{.int16={v1,v2,v3,v4}} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 16-bit unsigned integer to include in the event message.
   /// @param v2 the second 16-bit unsigned integer to include in the event message.
   /// @param v3 the third 16-bit unsigned integer to include in the event message.
   /// @param v4 the fourth 16-bit unsigned integer to include in the event message.
   EThreadEventMessageData(UInt msgid, UShort v1, UShort v2, UShort v3, UShort v4) : EThreadEventMessageDataBase(msgid), m_data{.uint16={v1,v2,v3,v4}} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 8-bit signed integer to include in the event message.
   /// @param v2 the second 8-bit signed integer to include in the event message.
   /// @param v3 the third 8-bit signed integer to include in the event message.
   /// @param v4 the fourth 8-bit signed integer to include in the event message.
   /// @param v5 the fifth 8-bit signed integer to include in the event message.
   /// @param v6 the sixth 8-bit signed integer to include in the event message.
   /// @param v7 the seventh 8-bit signed integer to include in the event message.
   /// @param v8 the eighth 8-bit signed integer to include in the event message.
   EThreadEventMessageData(UInt msgid, Char v1, Char v2, Char v3, Char v4, Char v5, Char v6, Char v7, Char v8) : EThreadEventMessageDataBase(msgid), m_data{.int8={v1,v2,v3,v4,v5,v6,v7,v8}} {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 8-bit unsigned integer to include in the event message.
   /// @param v2 the second 8-bit unsigned integer to include in the event message.
   /// @param v3 the third 8-bit unsigned integer to include in the event message.
   /// @param v4 the fourth 8-bit unsigned integer to include in the event message.
   /// @param v5 the fifth 8-bit unsigned integer to include in the event message.
   /// @param v6 the sixth 8-bit unsigned integer to include in the event message.
   /// @param v7 the seventh 8-bit unsigned integer to include in the event message.
   /// @param v8 the eighth 8-bit unsigned integer to include in the event message.
   EThreadEventMessageData(UInt msgid, UChar v1, UChar v2, UChar v3, UChar v4, UChar v5, UChar v6, UChar v7, UChar v8) : EThreadEventMessageDataBase(msgid), m_data{.uint8={v1,v2,v3,v4,v5,v6,v7,v8}} {}

   /// @brief Retrieves the data union object.
   /// @return a reference to the data union object.
   DataUnion &data() { return m_data; }

   /// @brief Retrieves the void pointer.
   /// @return the void pointer.
   pVoid getVoidPtr() { return m_data.voidptr; }
   /// @brief Sets the void pointer.
   /// @param p the void pointer.
   /// @return a reference to this object.
   Void setVoidPtr(pVoid p) { m_data.voidptr = p; }

private:
   DataUnion m_data;
};

/// @brief The event message base class.
/// @details This base class is a non-template class so that any event message
///   can be referred to by a non-template object.
class _EThreadEventMessageBase
{
public:
   /// @brief Virtual class destructor.
   virtual ~_EThreadEventMessageBase() {}
};

/// @brief A template class that all event message classes should derive from.
template <class T>
class EThreadEventMessageBase : public _EThreadEventMessageBase
{
public:
   /// @brief Default constructor.
   EThreadEventMessageBase()
      : m_data()
   {
   }
   /// @brief Copy constructor.
   /// @param data the T to copy.
   EThreadEventMessageBase(const T &data)
      : m_data( data )
   {
   }
   /// @brief Class destructor.
   virtual ~EThreadEventMessageBase()
   {
   }
   /// @brief Assignment operator.
   /// @param data the message object to copy.
   T &operator=(const T &data)
   {
      m_data = data;
      return *this;
   }
   /// @brief Retrieves the data portion of the message.
   /// @return a reference to the data portion of the message.
   T &data()
   {
      return m_data;
   }
   /// @brief Retrieves the event message ID associated with this event message.
   /// @return the event message ID associated with this event message.
   UInt getMessageId() { return m_data.getMessageId(); }
   /// @brief Sets the event message ID for this event message.
   /// @param msgid the event message ID for this event message.
   /// @return a reference to this object.
   EThreadEventMessageBase &setMessageId(UInt msgid) { m_data.setMessageid( msgid ); return *this; }
   /// @brief Retrieves the void pointer from the data portion of this event message.
   /// @return the void pointer from the data portion of this event message.
   pVoid getVoidPtr() { return m_data.getVoidPtr(); }
   /// @brief Sets the void pointer for this event message.
   /// @param p the void pointer for this event message.
   /// @return a reference to this object.
   Void setVoidPtr(pVoid p) { m_data.setVoidPtr(p); }
   /// @brief Retrieves the timer associated with this event message.
   /// @details This timer is started when the message is inserted into the thread
   ///   event queue.
   /// @return the timer associated with this event message.
   ETimer &getTimer() { return m_data.getTimer(); }

private:
   T m_data;
};

typedef EThreadEventMessageBase<EThreadEventMessageData> _EThreadMessage;

/// @brief An event message that is to be sent to a thread.
/// @details This thread message contains a union that interprets 8 bytes
///          in various ways.
class EThreadMessage : public _EThreadMessage
{
public:
   /// @brief Default class constructor.
   EThreadMessage() : _EThreadMessage() {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   EThreadMessage(UInt msgid) : _EThreadMessage(EThreadEventMessageData(msgid)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param data a data object to include in the message (contains the union definition).
   EThreadMessage(UInt msgid, EThreadEventMessageData &data) : _EThreadMessage(data) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v a void pointer to include in the event message.
   EThreadMessage(UInt msgid, pVoid v) : _EThreadMessage(EThreadEventMessageData(msgid, v)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v a 64-bit signed integer to include in the event message.
   EThreadMessage(UInt msgid, LongLong v) : _EThreadMessage(EThreadEventMessageData(msgid, v)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v a 64-bit unsigned integer to include in the event message.
   EThreadMessage(UInt msgid, ULongLong v) : _EThreadMessage(EThreadEventMessageData(msgid, v)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 32-bit signed integer to include in the event message.
   /// @param v2 the second 32-bit signed integer to include in the event message.
   EThreadMessage(UInt msgid, Long v1, Long v2=0) : _EThreadMessage(EThreadEventMessageData(msgid, v1, v2)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 32-bit unsigned integer to include in the event message.
   /// @param v2 the second 32-bit unsigned integer to include in the event message.
   EThreadMessage(UInt msgid, ULong v1, ULong v2=0) : _EThreadMessage(EThreadEventMessageData(msgid, v1, v2)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 16-bit signed integer to include in the event message.
   /// @param v2 the second 16-bit signed integer to include in the event message.
   /// @param v3 the third 16-bit signed integer to include in the event message.
   /// @param v4 the fourth 16-bit signed integer to include in the event message.
   EThreadMessage(UInt msgid, Short v1, Short v2=0, Short v3=0, Short v4=0) : _EThreadMessage(EThreadEventMessageData(msgid, v1, v2, v3, v4)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 16-bit unsigned integer to include in the event message.
   /// @param v2 the second 16-bit unsigned integer to include in the event message.
   /// @param v3 the third 16-bit unsigned integer to include in the event message.
   /// @param v4 the fourth 16-bit unsigned integer to include in the event message.
   EThreadMessage(UInt msgid, UShort v1, UShort v2=0, UShort v3=0, UShort v4=0) : _EThreadMessage(EThreadEventMessageData(msgid, v1, v2, v3, v4)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 8-bit signed integer to include in the event message.
   /// @param v2 the second 8-bit signed integer to include in the event message.
   /// @param v3 the third 8-bit signed integer to include in the event message.
   /// @param v4 the fourth 8-bit signed integer to include in the event message.
   /// @param v5 the fifth 8-bit signed integer to include in the event message.
   /// @param v6 the sixth 8-bit signed integer to include in the event message.
   /// @param v7 the seventh 8-bit signed integer to include in the event message.
   /// @param v8 the eighth 8-bit signed integer to include in the event message.
   EThreadMessage(UInt msgid, Char v1, Char v2=0, Char v3=0, Char v4=0, Char v5=0, Char v6=0, Char v7=0, Char v8=0)
      : _EThreadMessage(EThreadEventMessageData(msgid, v1, v2, v3, v4, v5, v6, v7, v8)) {}
   /// @brief Class constructor.
   /// @param msgid the event message id.
   /// @param v1 the first 8-bit unsigned integer to include in the event message.
   /// @param v2 the second 8-bit unsigned integer to include in the event message.
   /// @param v3 the third 8-bit unsigned integer to include in the event message.
   /// @param v4 the fourth 8-bit unsigned integer to include in the event message.
   /// @param v5 the fifth 8-bit unsigned integer to include in the event message.
   /// @param v6 the sixth 8-bit unsigned integer to include in the event message.
   /// @param v7 the seventh 8-bit unsigned integer to include in the event message.
   /// @param v8 the eighth 8-bit unsigned integer to include in the event message.
   EThreadMessage(UInt msgid, UChar v1, UChar v2=0, UChar v3=0, UChar v4=0, UChar v5=0, UChar v6=0, UChar v7=0, UChar v8=0)
      : _EThreadMessage(EThreadEventMessageData(msgid, v1, v2, v3, v4, v5, v6, v7, v8)) {}
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Defines how a client can access a thread queue.
enum class EThreadQueueMode
{
   /// Allows read only access.
   ReadOnly,
   /// Allows read only access.
   WriteOnly,
   /// Allows read or write access.
   ReadWrite
};

/// @brief Defines the functionality for the thread queue.
/// @details This is a templated class. The template parameter is the message
///   class.  This allows for a developer to provide a custom event message
///   definition.
template <class T>
class EThreadQueueBase
{
public:

   /// @brief Adds the specified message to the thread event queue.
   /// @param msg a reference to the message to add.
   /// @param wait indicates whether this function should wait for space to become
   ///   available in the queue.
   /// @return True indicates that the message was successfully added to the queue, otherwise False.
   ///   This function can only return False if wait is False.
   Bool push(const T &msg, Bool wait = True)
   {
      if (m_mode == EThreadQueueMode::ReadOnly)
         throw EThreadQueueBaseError_NotOpenForWriting();

      if (semFree().Decrement(wait) < 0)
         return False;

      {
         EMutexLock l(mutex());

         data()[msgHead()] = msg;
         data()[msgHead()].data().getTimer().Start();

         msgHead()++;

         if (msgHead() >= msgCnt())
            msgHead() = 0;
      }

      semMsgs().Increment();

      return True;
   }
   /// @brief Removes the next message from the thread event queue.
   /// @param msg a reference to a message object that will be populated with the message.
   /// @param wait indicates whether this function should wait for space to become
   ///   available in the queue.
   /// @return True indicates that a message was successfully popped from the queue, otherwise False.
   Bool pop(T &msg, Bool wait = True)
   {
      if (m_mode == EThreadQueueMode::WriteOnly)
         throw EThreadQueueBaseError_NotOpenForReading();

      if (!semMsgs().Decrement(wait))
         return False;

      msg = data()[msgTail()++];

      if (msgTail() >= msgCnt())
         msgTail() = 0;

      semFree().Increment();

      return True;
   }
   /// @brief Retrievees the next message from the thread event queue without removing
   ///   the message from the queue.
   /// @param msg a reference to a message object that will be populated with the message.
   /// @param wait indicates whether this function should wait for space to become
   ///   available in the queue.
   /// @return True indicates that a message was successfully popped from the queue, otherwise False.
   Bool peek(T &msg, Bool wait = True)
   {
      if (m_mode == EThreadQueueMode::WriteOnly)
         throw EThreadQueueBaseError_NotOpenForReading();

      if (!semMsgs().Decrement(wait))
         return False;

      msg = data()[msgTail()];

      // since we are not pulling the message off, we need to increment
      // the semaphore to put it back where it was
      semMsgs().Increment();

      return True;
   }

   /// @brief Retrieves indication if this queue object has been initialized.
   /// @return True if initialized, otherwise False.
   Bool isInitialized() { return m_initialized; }
   /// @brief Retrieves the access mode associated with this queue object.
   /// @return the access mode associated with this queue object.
   EThreadQueueMode mode() { return m_mode; }

protected:
   /// @cond DOXYGEN_EXCLUDE

   virtual Bool isPublic() = 0;
   virtual Int &msgCnt() = 0;
   virtual Int &msgHead() = 0;
   virtual Int &msgTail() = 0;
   virtual Bool &multipleWriters() = 0;
   virtual Int &numReaders() = 0;
   virtual Int &numWriters() = 0;
   virtual Int &refCnt() = 0;
   virtual T *data() = 0;
   virtual Void allocDataSpace(cpStr sFile, Char cId, Int nSize) = 0;
   virtual Void initMutex() = 0;
   virtual Void initSemFree(UInt initialCount) = 0;
   virtual Void initSemMsgs(UInt initialCount) = 0;

   virtual EMutexData &mutex() = 0;
   virtual ESemaphoreData &semMsgs() = 0;
   virtual ESemaphoreData &semFree() = 0;

   virtual int *getBumpPipe() = 0;

   EThreadQueueBase()
   {
      m_initialized = False;
   }
   ~EThreadQueueBase()
   {
   }

   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters, EThreadQueueMode eMode)
   {
      m_mode = eMode;

      // construct the shared memory name
      Char szName[EPC_FILENAME_MAX];
      epc_sprintf_s(szName, sizeof(szName), "%d", threadId);

      // calcuate the space required
      int nSize = sizeof(T) * nMsgCnt;

      // initialize the shared memory
      allocDataSpace(szName, 'A', nSize);

      // initialize the control block values
      if (refCnt() == 0)
      {
         refCnt() = 0;
         numReaders() = 0;
         numWriters() = 0;
         multipleWriters() = bMultipleWriters;
         msgCnt() = nMsgCnt;
         msgHead() = 0;
         msgTail() = 0;

         // initialize the control mutex and semaphores
         initMutex();
         initSemFree(msgCnt());
         initSemMsgs(0);
      }
      else
      {
      }

      EMutexLock l(mutex());

      if ((eMode == EThreadQueueMode::ReadOnly || eMode == EThreadQueueMode::ReadWrite) && numReaders() > 0)
      {
         throw EThreadQueueBaseError_MultipleReadersNotAllowed();
      }

      refCnt()++;
      numReaders() += (eMode == EThreadQueueMode::ReadOnly || eMode == EThreadQueueMode::ReadWrite) ? 1 : 0;
      numWriters() += (eMode == EThreadQueueMode::WriteOnly || eMode == EThreadQueueMode::ReadWrite) ? 1 : 0;

      m_initialized = True;
   }

   Void destroy()
   {
      Bool destroyMutex = False;

      if (m_initialized)
      {
         EMutexLock l(mutex());

         if (refCnt() == 1)
         {
            semFree().destroy();
            semMsgs().destroy();

            destroyMutex = True;
         }
         else
         {
            refCnt()--;
            numReaders() -= (m_mode == EThreadQueueMode::ReadOnly || m_mode == EThreadQueueMode::ReadWrite) ? 1 : 0;
            numWriters() -= (m_mode == EThreadQueueMode::WriteOnly || m_mode == EThreadQueueMode::ReadWrite) ? 1 : 0;
         }
      }

      if (destroyMutex)
         mutex().destroy();
   }
   /// @endcond

private:
   Bool m_initialized;
   EThreadQueueMode m_mode;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Definition of a public event thread message queue.
/// @details The template parameter to this class template is the class of for
///   the message that will be stored in this event thread message queue.  This
///   class is derived from template <class T> class EThreadQueueBase.
/// @tparam T the event message class name.
template <class T>
class EThreadQueuePublic : public EThreadQueueBase<T>
{
   template <class TQueue, class TMessage> friend class EThreadEvent;
public:
   /// @brief Default constructor.
   EThreadQueuePublic()
   {
      m_pCtrl = NULL;
      m_pData = NULL;
   }
   /// @brief Class destructor.
   ~EThreadQueuePublic()
   {
   }
   /// @brief Initializes this public event thead message queue object.
   /// @param nMsgCnt the maximum number of event messages that can exist
   ///   in the queue at a time.
   /// @param threadId the public thread identifier.
   /// @param bMultipleWriters indicates if more than can be more than
   ///   one instance can write to the queue at a time.  If True, inserting
   ///   messages into the queue will be controled by a mutex.
   /// @param eMode indicates the desired access mode.
   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             EThreadQueueMode eMode)
   {
      EThreadQueueBase<T>::init(nMsgCnt, threadId, bMultipleWriters, eMode);
   }

protected:
   /// @cond DOXYGEN_EXCLUDE
   Bool isPublic() { return True; }
   Int &msgCnt() { return m_pCtrl->m_msgCnt; }
   Int &msgHead() { return m_pCtrl->m_head; }
   Int &msgTail() { return m_pCtrl->m_tail; }
   Bool &multipleWriters() { return m_pCtrl->m_multipleWriters; }
   Int &numReaders() { return m_pCtrl->m_numReaders; }
   Int &numWriters() { return m_pCtrl->m_numWriters; }
   Int &refCnt() { return m_pCtrl->m_refCnt; }
   T *data() { return m_pData; }
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize)
   {
      m_sharedmem.init(sFile, cId, nSize + sizeof(ethreadmessagequeue_ctrl_t));
      m_pCtrl = (ethreadmessagequeue_ctrl_t *)m_sharedmem.getDataPtr();
      m_pData = (T *)(((pChar)m_sharedmem.getDataPtr()) + sizeof(ethreadmessagequeue_ctrl_t));
   }
   Void initMutex()
   {
      EMutexPublic m;
      m_pCtrl->m_mutexid = m.mutexId();
      m.detach();
   }
   Void initSemFree(UInt initialCount)
   {
      ESemaphorePublic s(initialCount);
      m_pCtrl->m_freeSemId = s.semIndex();
      s.detach();
   }
   Void initSemMsgs(UInt initialCount)
   {
      ESemaphorePublic s(initialCount);
      m_pCtrl->m_msgsSemId = s.semIndex();
      s.detach();
   }

   EMutexData &mutex() { return ESynchObjects::getMutex(m_pCtrl->m_mutexid); }
   ESemaphoreData &semFree() { return ESynchObjects::getSemaphore(m_pCtrl->m_freeSemId); }
   ESemaphoreData &semMsgs() { return ESynchObjects::getSemaphore(m_pCtrl->m_msgsSemId); }

   int *getBumpPipe() { return m_pCtrl->m_bumppipe; }
   /// @endcond

private:
   typedef struct
   {
      Int m_refCnt;
      Int m_numReaders;
      Int m_numWriters;
      Bool m_multipleWriters;

      Int m_msgCnt;
      Int m_head; // next location to write
      Int m_tail; // next location to read

      Int m_mutexid;
      Int m_freeSemId;
      Int m_msgsSemId;

      int m_bumppipe[2];
   } ethreadmessagequeue_ctrl_t;

   ESharedMemory m_sharedmem;
   ethreadmessagequeue_ctrl_t *m_pCtrl;
   T *m_pData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Definition of a private event thread message queue.
/// @details The template parameter to this class template is the class of for
///   the message that will be stored in this event thread message queue.  This
///   class is derived from template <class T> class EThreadQueueBase.
/// @tparam T the event message class name.
template <class T>
class EThreadQueuePrivate : public EThreadQueueBase<T>
{
   template <class TQueue, class TMessage> friend class EThreadEvent;
public:
   /// @brief Default constructor.
   EThreadQueuePrivate()
      : m_mutex(False), m_semFree(0, False), m_semMsgs(0, False)
   {
      m_refCnt = 0;
      m_numReaders = 0;
      m_numWriters = 0;
      m_multipleWriters = False;

      m_msgCnt = 0;
      m_head = 0; // next location to write
      m_tail = 0; // next location to read
      m_pData = NULL;
   }
   /// @brief Class destructor.
   ~EThreadQueuePrivate()
   {
      if (m_pData)
      {
         delete[](pChar) m_pData;
         m_pData = NULL;
      }
   }
   /// @brief Initializes this public event thead message queue object.
   /// @param nMsgCnt the maximum number of event messages that can exist
   ///   in the queue at a time.
   /// @param threadId the public thread identifier.
   /// @param bMultipleWriters indicates if more than can be more than
   ///   one instance can write to the queue at a time.  If True, inserting
   ///   messages into the queue will be controled by a mutex.
   /// @param eMode indicates the desired access mode.
   Void init(Int nMsgCnt, Int threadId, Bool bMultipleWriters,
             EThreadQueueMode eMode)
   {
      EThreadQueueBase<T>::init(nMsgCnt, threadId, bMultipleWriters, eMode);
   }

protected:
   /// @cond DOXYGEN_EXCLUDE
   Bool isPublic() { return False; }
   Int &msgCnt() { return m_msgCnt; }
   Int &msgHead() { return m_head; }
   Int &msgTail() { return m_tail; }
   Bool &multipleWriters() { return m_multipleWriters; }
   Int &numReaders() { return m_numReaders; }
   Int &numWriters() { return m_numWriters; }
   Int &refCnt() { return m_refCnt; }
   T *data() { return m_pData; }
   Void allocDataSpace(cpStr sFile, Char cId, Int nSize)
   {
      m_pData = (EThreadMessage *)new Char[nSize];
      memset((pChar)m_pData, 0, nSize);
   }
   Void initMutex()
   {
      m_mutex.init();
   }
   Void initSemFree(UInt initialCount)
   {
      m_semFree.init(initialCount);
   }
   Void initSemMsgs(UInt initialCount)
   {
      m_semMsgs.init(initialCount);
   }

   EMutexData &mutex() { return m_mutex; }
   ESemaphoreData &semFree() { return m_semFree; }
   ESemaphoreData &semMsgs() { return m_semMsgs; }

   int *getBumpPipe() { return m_bumppipe; }
   /// @endcond

private:
   Int m_refCnt;
   Int m_numReaders;
   Int m_numWriters;
   Bool m_multipleWriters;

   Int m_msgCnt;
   Int m_head; // next location to write
   Int m_tail; // next location to read

   EMutexPrivate m_mutex;
   ESemaphorePrivate m_semFree;
   ESemaphorePrivate m_semMsgs;

   int m_bumppipe[2];

   T *m_pData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// thread initialization event
#define EM_INIT 1
/// thread quit event
#define EM_QUIT 2
/// thread suspend event
#define EM_SUSPEND 3
/// thread timer expiration event
#define EM_TIMER 4
/// socket read event see ESocketThread
#define EM_SOCKETSELECT_READ 5
/// socket write event see ESocketThread
#define EM_SOCKETSELECT_WRITE 6
/// socket select error event see ESocketThread
#define EM_SOCKETSELECT_ERROR 7
/// socket exception event see ESocketThread
#define EM_SOCKETSELECT_EXCEPTION 8
/// beginning of user events
#define EM_USER 10000

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Inserts message map declarations into the thread class.
/// @details This macro should be used in the event thread class definition.
#define DECLARE_MESSAGE_MAP()                                  \
protected:                                                     \
   static const msgmap_t *GetThisMessageMap();                 \
   virtual const msgmap_t *GetMessageMap() const;

/// @brief Begins the message map declaration.
/// @details The event message map establishes the relationship of the
///   event ID's to the individual event handlers defined in the class.
///   "theClass" is the name of the class containing the message handlers.
///   "baseClass" is the class name that "theClass" is derived from.
#define BEGIN_MESSAGE_MAP(theClass, baseClass)                 \
   const theClass::msgmap_t *theClass::GetMessageMap() const   \
   {                                                           \
      return GetThisMessageMap();                              \
   }                                                           \
   const theClass::msgmap_t *theClass::GetThisMessageMap()     \
   {                                                           \
      typedef theClass ThisClass;                              \
      typedef baseClass TheBaseClass;                          \
      _Pragma("GCC diagnostic push")                           \
      _Pragma("GCC diagnostic ignored \"-Wpmf-conversions\"")  \
      static const msgentry_t _msgEntries[] =                  \
      {

/// @brief Defines an invidual event handler.
#define ON_MESSAGE(id, memberFxn)                              \
         {id, (msgfxn_t)&memberFxn},

/// @brief Ends the message map declaration.
#define END_MESSAGE_MAP()                                      \
         {0, (msgfxn_t)NULL}                                   \
      };                                                       \
      _Pragma("GCC diagnostic pop")                            \
      static const msgmap_t msgMap =                           \
         {&TheBaseClass::GetThisMessageMap, &_msgEntries[0]};  \
      return &msgMap;                                          \
   }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class _EThreadEventBase
{
public:
   virtual Bool _sendMessage(const _EThreadEventMessageBase &msg, Bool wait = True)
   {
   }
   virtual Void _destroyMessage(_EThreadEventMessageBase *msg)
   {
      delete msg;
   }
};

/// @brief Thread timer class.
///
/// @details EThreadBase::Timer represents an individual timer.  When the
/// timer expires, the EM_TIMER event will be raised.  The application
/// can handle the timer by overrideing the onTimer method.
///
class EThreadEventTimer : public EStatic
{
   friend class EThreadEventTimerHandler;
   template <class TQueue, class TMessage> friend class EThreadEvent;

/// @cond DOXYGEN_EXCLUDE
protected:
   Void init(_EThreadEventBase *thread, _EThreadEventMessageBase *msg)
   {
      m_thread = thread;
      m_msg = msg;

      struct sigevent sev = {};
      sev.sigev_notify = SIGEV_SIGNAL;
      sev.sigev_signo = SIGRTMIN;
      sev.sigev_value.sival_ptr = this;
      if (timer_create(CLOCK_REALTIME, &sev, &m_timer) == -1)
         throw EThreadTimerError_UnableToInitialize();
   }
/// @endcond

public:
   /// @brief Default class constructor.
   EThreadEventTimer()
   {
      // assign the id
      m_id = atomic_inc(m_nextid);
      m_thread = NULL;
      m_msg = NULL;
      m_interval = 0;
      m_oneshot = True;
      m_timer = NULL;
   }
   /// @brief Class constructor with configuration parameters.
   ///
   /// @param milliseconds the number of milliseconds before the timer expires
   /// @param oneshot True - one shot timer, False - periodic (recurring) timer
   ///
   EThreadEventTimer(Long milliseconds, Bool oneshot = False)
   {
      // assign the id
      m_id = atomic_inc(m_nextid);
      m_thread = NULL;
      m_msg = NULL;
      m_interval = milliseconds;
      m_oneshot = oneshot;
      m_timer = NULL;
   }      /// @brief Class destructor.
   ~EThreadEventTimer()
   {
      destroy();
   }
   /// @brief Stops and destroys the underlying timer object.
   ///
   /// @details Calling destroy() will stop the timer and then delete the
   /// underlying timer object.  This method is called by the destructor.
   ///
   Void destroy()
   {
      if (m_timer != NULL)
      {
         stop();
         timer_delete(m_timer);
         m_timer = NULL;
      }
      if (m_msg)
      {
         if (m_thread)
            m_thread->_destroyMessage(m_msg);
         else
            delete m_msg;
         m_msg = NULL;
      }
      m_thread = NULL;
   }
   /// @brief Starts the timer.
   ///
   /// @throws EThreadTimerError_NotInitialized timer not initialized
   /// @throws EThreadTimerError_UnableToStart unable to start the timer
   /// 
   Void start()
   {
      if (m_timer == NULL)
         throw EThreadTimerError_NotInitialized();

      struct itimerspec its;
      its.it_value.tv_sec = m_interval / 1000;              // seconds
      its.it_value.tv_nsec = (m_interval % 1000) * 1000000; // nano-seconds
      its.it_interval.tv_sec = m_oneshot ? 0 : its.it_value.tv_sec;
      its.it_interval.tv_nsec = m_oneshot ? 0 : its.it_value.tv_nsec;
      if (timer_settime(m_timer, 0, &its, NULL) == -1)
         throw EThreadTimerError_UnableToStart();
   }
   /// @brief Stops the timer.
   Void stop()
   {
      if (m_timer != NULL)
      {
         struct itimerspec its;
         its.it_value.tv_sec = 0;  // seconds
         its.it_value.tv_nsec = 0; // nano-seconds
         its.it_interval.tv_sec = its.it_value.tv_sec;
         its.it_interval.tv_nsec = its.it_value.tv_nsec;
         timer_settime(m_timer, 0, &its, NULL);
      }
   }
   /// @brief Returns the timer interval in milliseconds.
   Long getInterval() { return m_interval; }
   /// @brief sets the timer interval
   ///
   /// @param interval the timer interval in milliseconds.
   ///
   Void setInterval(Long interval) { m_interval = interval; }
   /// @brief sets the type of timer
   ///
   /// @param oneshot True - one shot timer, False - periodic (recurring timer)
   ///
   Void setOneShot(Bool oneshot) { m_oneshot = oneshot; }
   /// @brief Returns the unique timer id.
   ///
   /// @details
   /// The timer ID is created internally when the timer object is
   /// instantiated.
   ///
   Long getId() { return m_id; }
   /// @brief Indicates if this timer object has been initialized.
   ///
   /// @details
   /// The timer ID is created internally when the timer object is
   /// instantiated.
   ///
   Bool isInitialized() { return m_timer != NULL; }

protected:
   /// @cond DOXYGEN_EXCLUDE
   static void _timerHandler(int signo, siginfo_t *pinfo, void *pcontext)
   {
      EThreadEventTimer *timer = (EThreadEventTimer*)pinfo->si_value.sival_ptr;
      if (timer)
      {
         timer->m_thread->_sendMessage(*timer->m_msg);
      }
   }
   /// @endcond

private:
   static Long m_nextid;

   Long m_id;
   _EThreadEventBase *m_thread;
   _EThreadEventMessageBase *m_msg;
   Bool m_oneshot;
   Long m_interval;
   timer_t m_timer;
};

/// @cond DOXYGEN_EXCLUDE
class EThreadEventTimerHandler : EStatic
{
public:
   EThreadEventTimerHandler() {}
   ~EThreadEventTimerHandler() {}

   virtual Int getInitType() { return STATIC_INIT_TYPE_THREADS; }
   Void init(EGetOpt &options)
   {
      struct sigaction sa;
      sa.sa_flags = SA_SIGINFO;
      sa.sa_sigaction = EThreadEventTimer::_timerHandler;
      sigemptyset(&sa.sa_mask);
      int signo = SIGRTMIN;
      if (sigaction(signo, &sa, NULL) == -1)
         throw EThreadTimerError_UnableToRegisterTimerHandler();
   }
   Void uninit()
   {
   }
};
/// @endcond

/// @brief base class for EThreadPrivate and EThreadPublic
///
/// @details The EThreadBase class provides event dispatching and timer
/// management functionality that are part of EThreadPrivate and EThreadPublic.
/// the EThreadPrivate and EThreadPublic classes.  This functionality includes
/// event dispatching 
///
template <class TQueue, class TMessage>
class EThreadEvent : public EThreadBasic, public _EThreadEventBase
{
public:
   /// @cond DOXYGEN_EXCLUDE
   typedef Void (EThreadEvent::*msgfxn_t)(TMessage &);
   typedef struct
   {
      UInt     nMessage; // message
      msgfxn_t pFn; // routine to call (or special value)
   } msgentry_t;

   struct msgmap_t
   {
      const msgmap_t *(*pfnGetBaseMap)();
      const msgentry_t *lpEntries;
   };
   /// @endcond

   /// @brief Default class constructor.
   EThreadEvent()
      : EThreadBasic(),
        m_tid(-1),
        m_arg(NULL),
        m_stacksize(0),
        m_suspendCnt(0),
        m_suspendSem(0)
   {
   }
   /// @brief Rhe class destructor.
   ~EThreadEvent()
   {
   }

   /// @brief Sends event message to this thread.
   ///
   /// @param message the message ID
   /// @param wait waits for the message to be sent
   ///
   /// @details
   /// Sends (posts) an event message to this threads event queue.  No
   /// additional data is posted with the event.
   /// 
   Bool sendMessage(UInt message, Bool wait = True)
   {
      TMessage msg(message);
      Bool result = m_queue.push(msg, wait);
      if (result)
         messageQueued();
      return result;
   }
   /// @brief Sends event message to this thread.
   ///
   /// @param message the message ID.
   /// @param voidptr a void pointer to be included with the message.
   /// @param wait waits for the message to be sent.
   ///
   /// @details
   /// Sends (posts) an event message to this threads event queue.  No
   /// additional data is posted with the event.
   /// 
   Bool sendMessage(UInt message, pVoid voidptr, Bool wait = True)
   {
      TMessage msg(message);
      msg.setVoidPtr(voidptr);
      Bool result = m_queue.push(msg, wait);
      if (result)
         messageQueued();
      return result;
   }
   /// @brief Sends event message to this thread.
   ///
   /// @param msg the message thread message object to send.
   /// @param wait waits for the message to be sent
   ///
   /// @details
   /// Sends (posts) the supplied event message to this thread's event queue.
   /// 
   Bool sendMessage(const TMessage &msg, Bool wait = True)
   {
      Bool result = m_queue.push(msg, wait);
      if (result)
         messageQueued();
      return result;
   }

   /// @brief Initializes the thread object.
   /// @param appId identifies the application this thread is associated with.
   /// @param threadId identifies the thread within this application.
   /// @param arg an argument that will be passed through to the internal thread procedure.
   ///   Currently not used.
   /// @param queueSize the maximum number of unprocessed entries in the event queue.
   /// @param suspended if True, the thread is started in a suspended state.
   /// @param stackSize the stack size.
   virtual Void init(Short appId, UShort threadId, pVoid arg, Int queueSize = 16384, Bool suspended = False, Dword stackSize = 0)
   {
      m_appId = appId;
      m_threadId = threadId;
      m_queueSize = queueSize;
      m_stacksize = stackSize;

      long id = m_appId * 10000 + m_threadId;

      m_queue.init(m_queueSize, id, True, EThreadQueueMode::ReadWrite);

      if (!suspended)
         start();
   }

   /// @brief Posts the quit message to this thread.
   Void quit()
   {
      sendMessage(EM_QUIT);
   }
   /// @brief Initializes the thread when it was suspended at init().
   Void start()
   {
      if (!isInitialized())
      {
         EThreadBasic::init(m_arg, m_stacksize);
         sendMessage(EM_INIT);
      }
   }
   /// @brief Suspends a running thread.
   ///
   /// @details
   /// Suspends a running thread by posting the EM_SUSPEND message to the
   /// threads event queue.  The thread will call the onSuspend() method
   /// when the EM_SUSPEND event is processed.  The thread will not process
   /// any more thread messages until the resume() method is called.
   ///
   Void suspend()
   {
      if (atomic_inc(m_suspendCnt) == 1)
         sendMessage(EM_SUSPEND);
   }
   /// @brief Resumes a suspended thread.
   Void resume()
   {
      if (atomic_dec(m_suspendCnt) == 0)
         m_suspendSem.Increment();
   }
   /// @brief Called in the context of the thread when the EM_INIT event is processed.
   virtual Void onInit()
   {
   }
   /// @brief Called in the context of the thread when the EM_QUIT event is processed.
   virtual Void onQuit()
   {
   }
   /// @brief Called in the context of the thread when th EM_SUSPEND event is processed.
   virtual Void onSuspend()
   {
   }
   /// @brief Called in the context of the thread when th EM_TIMER event is processed.
   ///
   /// @param ptimer a pointer to the EThreadBase::Timer object that expired
   ///
   virtual Void onTimer(EThreadEventTimer *ptimer)
   {
   }
   /// @brief Intializes an EThreadEvent::Timer object and associates with this thread.
   ///
   /// @param t the EThreadEvent::Timer object to initialize
   Void initTimer(EThreadEventTimer &t)
   {
      TMessage *msg = new TMessage(EM_TIMER);
      msg->setVoidPtr(&t);
      t.init(this, msg);
   }
   /// @brief Returns the semaphore associated with this thread's event queue.
   ESemaphoreData &getMsgSemaphore()
   {
      return m_queue.semMsgs();
   }

protected:
   /// @cond DOXYGEN_EXCLUDE
   virtual const msgmap_t *GetMessageMap() const
   {
      return GetThisMessageMap();
   }
   static const msgmap_t *GetThisMessageMap()
   {
      return NULL;
   }
   int *getBumpPipe()
   {
      return m_queue.getBumpPipe();
   }
   /// @endcond

   /// @brief Called when an event message is queued.
   ///
   /// @details
   /// This method is called in the context of the thread where sendMessage()
   /// is called after the message has been added to the thread's event queue.
   virtual Void messageQueued()
   {
   }
   /// @brief Dispatches the next thread event message.
   ///
   /// @param msg the EThreadMessage event object
   /// @param wait waits for the next EThreadMessage to be processed
   ///
   /// @details
   /// This method retrieves the next event message from the threads event
   /// queue and processes the event by calling the first event handler as
   /// defined by the class heirarchy.  If no event message is available and
   /// the wait parameter is true, then the thread will block waiting on the
   /// next event message to be sent to the thread.  If there is no event
   /// handler defined in the class heirarchy for a particular event ID, the
   /// default event handler, defMessageHandler(), will be called.
   ///
   Bool pumpMessage(TMessage &msg, Bool wait = true)
   {
      Bool bMsg = m_queue.pop(msg, wait);
      if (bMsg)
         dispatch(msg);

      return bMsg;
   }
   /// @brief Process event messages.
   ///
   /// @throws EError catches and re-throws any exception raised by pumpMessage
   ///
   /// @details
   /// Any overridden version of pumpMessages() must call pumpMessage() to
   /// process each individual message.
   ///
   virtual Void pumpMessages()
   {
      TMessage msg;

      try
      {
         while (True)
         {
            if (pumpMessage(msg))
            {
               if (msg.getMessageId() == EM_QUIT)
                  break;
               if (msg.getMessageId() == EM_SUSPEND)
                  m_suspendSem.Decrement();
            }
         }
      }
      catch (EError &e)
      {
         //std::cout << "t1 - " << e.Name() << std::endl;
         throw;
      }
      catch (...)
      {
         throw;
      }
   }
   /// @brief The default event message handler.
   ///
   /// @param msg the EThreadMessage event object
   ///
   /// This method is called when no event handler has been defined in the
   /// class heirarchy for a specified thread event.
   ///
   virtual Void defaultMessageHandler(TMessage &msg)
   {
   }
   /// @brief Retrieves the internal thread ID.
   /// @return the internal thread ID
   pid_t getThreadId()
   {
      if (m_tid == -1)
         m_tid = syscall(SYS_gettid);
      return m_tid;
   }

private:
   Dword threadProc(pVoid arg)
   {
      pumpMessages();
      return 0;
   }

   Bool dispatch(TMessage &msg)
   {
      Bool res = True;
      Bool keepgoing = True;
      const msgmap_t *pMap;
      const msgentry_t *pEntries;

      if (msg.getMessageId() >= EM_USER)
      {
         // interate through each map
         for (pMap = GetMessageMap(); keepgoing && pMap && pMap->pfnGetBaseMap != NULL; pMap = (*pMap->pfnGetBaseMap)())
         {
            // interate through each entry for the map
            for (pEntries = pMap->lpEntries; pEntries->nMessage; pEntries++)
            {
               if (pEntries->nMessage == msg.getMessageId())
               {
                  (this->*pEntries->pFn)(msg);
                  keepgoing = False;
                  break;
               }
            }
         }

         if (pMap == NULL)
            defaultMessageHandler(msg);
      }
      else
      {
         switch (msg.getMessageId())
         {
            case EM_INIT:
            {
               onInit();
               break;
            }
            case EM_QUIT:
            {
               onQuit();
               break;
            }
            case EM_SUSPEND:
            {
               onSuspend();
               break;
            }
            case EM_TIMER:
            {
               onTimer( (EThreadEventTimer*)msg.getVoidPtr() );
               break;
            }
            default:
            {
               break;
            }
         }
      }

      return keepgoing;
   }

   Bool _sendMessage(const _EThreadEventMessageBase &msg, Bool wait)
   {
      return sendMessage( (const TMessage &)msg, wait );
   }

   Void _destroyMessage(_EThreadEventMessageBase *msg)
   {
      delete (TMessage*)msg;
   }

   Short m_appId;
   UShort m_threadId;
   pVoid m_arg;
   Int m_queueSize;
   size_t m_stacksize;

   pid_t m_tid;
   Int m_suspendCnt;
   ESemaphorePrivate m_suspendSem;
   TQueue m_queue;
};

typedef EThreadEvent<EThreadQueuePublic<EThreadMessage>,EThreadMessage> EThreadPublic;
typedef EThreadEvent<EThreadQueuePrivate<EThreadMessage>,EThreadMessage> EThreadPrivate;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef __ETEVENT_H