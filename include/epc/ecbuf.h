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

#ifndef __ecbuf_h_included
#define __ecbuf_h_included

/// @file
/// @brief Implements a circular buffer.

#include "ebase.h"
#include "esynch.h"
#include "eerror.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
DECLARE_ERROR(ECircularBufferError_HeadAndTailOutOfSync);
DECLARE_ERROR(ECircularBufferError_UsedLessThanZero);
DECLARE_ERROR(ECircularBufferError_TailExceededCapacity);
DECLARE_ERROR(ECircularBufferError_AttemptToExceedCapacity);
DECLARE_ERROR(ECircularBufferError_BufferSizeHasBeenExceeded);
DECLARE_ERROR(ECircularBufferError_HeadHasExceededCapacity);
DECLARE_ERROR(ECircularBufferError_AttemptToModifyDataOutsideBoundsOfCurrentBuffer);
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Implements a circular buffer.
class ECircularBuffer
{
public:
   /// @brief Class constructor.
   /// @param capacity The maximum number of bytes in the buffer.
   ECircularBuffer(Int capacity);
   /// @brief Class destructor.
   ~ECircularBuffer();

   /// @brief Initializes the circular buffer.
   Void initialize();

   /// @brief True - the buffer is empty, False - there is data in the buffer
   Bool isEmpty() { return m_used == 0; }
   /// @brief Returns the maximum capacity of the circular buffer.
   Int capacity() { return m_capacity; }
   /// @brief Returns the number of bytes in use in the buffer.
   Int used() { return m_used; }
   /// @brief Returns the number of bytes that are free in the buffer.
   Int free() { return m_capacity - m_used; }

   /// @brief Reads data from the buffer without removing it from the buffer.
   /// @param dest A pointer to a buffer where the data will be written.
   /// @param offset The offset in the buffer where to start reading from.
   /// @param length The number of bytes to read.
   /// @throws ECircularBufferError_HeadAndTailOutOfSync
   /// @throws ECircularBufferError_UsedLessThanZero
   /// @throws ECircularBufferError_TailExceededCapacity
   Int peekData(pUChar dest, Int offset, Int length)
   {
      return readData(dest, offset, length, true);
   }

   /// @brief Reads and removes data from the buffer.
   /// @param dest A pointer to a buffer where the data will be written.
   /// @param offset The offset in the buffer where to start reading from.
   /// @param length The number of bytes to read.
   Int readData(pUChar dest, Int offset, Int length)
   {
      return readData(dest, offset, length, false);
   }

   /// @brief Writes data to the circular buffer.
   /// @param src A pointer to the data that will be written to the buffer.
   /// @param offset *** NOT USED ***
   /// @param length The number of bytes to write to the buffer.
   /// @param nolock True bypasses the lock, otherwise the object is locked.
   /// @throws ECircularBufferError_AttemptToExceedCapacity
   /// @throws ECircularBufferError_BufferSizeHasBeenExceeded
   /// @throws ECircularBufferError_HeadHasExceededCapacity
   ///
   void writeData(pUChar src, Int offset, Int length, Bool nolock=False);
   /// @brief Modifies data within the buffer.
   /// @param src A pointer to the data that will be written to the buffer.
   /// @param offset The offset within the buffer to start writing.
   /// @param length The number of bytes to write to the buffer.
   /// @param nolock True bypasses the lock, otherwise the object is locked.
   /// @throws ECircularBufferError_AttemptToModifyDataOutsideBoundsOfCurrentBuffer
   /// @details
   /// Generally speaking, data is written to the head and read from the
   /// tail.  modifyData updates data starting at the tail, the next data
   /// to read.  This method was added to support partial socket writes
   /// where the message length, the first 4 bytes, needed to updated to
   /// reflect the remaining bytes to be written.
   /// <b>**** USE WITH EXTREME CAUTION ***</b>
   void modifyData(pUChar src, Int offset, Int length, Bool nolock=False);

   /// @brief Retrieves the mutex;
   /// @return the mutex.
   EMutexPrivate &getMutex() { return m_mutex; }

private:
   Int readData(pUChar dest, Int offset, Int length, Bool peek);

   ECircularBuffer();

   pUChar m_data;
   Int m_capacity;
   Int m_head;
   Int m_tail;
   Int m_used;

   EMutexPrivate m_mutex;
};

#endif // #define __ecbuf_h_included
