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

#include "ebase.h"
#include "esynch.h"
#include "eerror.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(ECircularBufferError_HeadAndTailOutOfSync);
DECLARE_ERROR(ECircularBufferError_UsedLessThanZero);
DECLARE_ERROR(ECircularBufferError_TailExceededCapacity);
DECLARE_ERROR(ECircularBufferError_AttemptToExceedCapacity);
DECLARE_ERROR(ECircularBufferError_BufferSizeHasBeenExceeded);
DECLARE_ERROR(ECircularBufferError_HeadHasExceededCapacity);
DECLARE_ERROR(ECircularBufferError_AttemptToModifyDataOutsideBoundsOfCurrentBuffer);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ECircularBuffer
{
public:
   ECircularBuffer(Int capacity);
   ~ECircularBuffer();

   Void initialize();

   Bool isEmpty() { return m_used == 0; }
   Int capacity() { return m_capacity; }
   Int used() { return m_used; }
   Int free() { return m_capacity - m_used; }

   Int peekData(pUChar dest, Int offset, Int length)
   {
      return readData(dest, offset, length, true);
   }

   Int readData(pUChar dest, Int offset, Int length)
   {
      return readData(dest, offset, length, false);
   }

   void writeData(pUChar src, Int offset, Int length);
   void modifyData(pUChar src, Int offset, Int length);

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
