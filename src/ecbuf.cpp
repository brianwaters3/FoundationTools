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

#include "ecbuf.h"

ECircularBuffer::ECircularBuffer(Int capacity)
{
    m_capacity = capacity;
    m_data = NULL;

    initialize();
}

ECircularBuffer::~ECircularBuffer()
{
    if (m_data)
    {
        delete [] m_data;
        m_data = NULL;
    }
}

Void ECircularBuffer::initialize()
{
    m_head = 0;
    m_tail = 0;
    m_used = 0;

    if (m_data)
        delete [] m_data;

    m_data = new UChar[m_capacity];
}

Int ECircularBuffer::readData(pUChar dest, Int offset, Int length, Bool peek)
{
    EMutexLock lockMutex(m_mutex);

    // check to see if there is anything in the buffer
    if (m_used == 0)
    {
        if (m_head != m_tail)
            throw ECircularBufferError_HeadAndTailOutOfSync();
        return 0;
    }

    // check to see if there is any data based on the offset
    if (offset >= m_used)
        return 0;

    Int amtRead = 0;
    Int newTail = m_tail + offset;
    Int newUsed = m_used;

    if (newTail >= m_capacity)
        newTail -= m_capacity;

    while (amtRead < length && newUsed > 0)
    {
        Int readLen = (newTail < m_head) ? m_head - newTail : m_capacity - newTail;
        if (readLen > (length - amtRead))
            readLen = length - amtRead;

        if (dest)
            memcpy(&dest[amtRead], &m_data[newTail], readLen);

        amtRead += readLen;
        newUsed -= readLen;
        if (newUsed < 0)
            throw ECircularBufferError_UsedLessThanZero();

        newTail += readLen;
        if (newTail == m_capacity)
            newTail = 0;
        else if (newTail > m_capacity)
            throw ECircularBufferError_TailExceededCapacity();
    }

    if (!peek)
    {
        m_used = newUsed;
        m_tail = newTail;
    }

    return amtRead;
}

void ECircularBuffer::writeData(pUChar src, Int offset, int length, Bool nolock)
{
    EMutexLock lockMutex(m_mutex, False);

    if (!nolock)
      lockMutex.acquire();

    if (m_used + length > m_capacity)
        throw ECircularBufferError_AttemptToExceedCapacity();

    Int amtWritten = 0;

    while (amtWritten < length)
    {
        Int writeLen = (m_head < m_tail) ? m_tail - m_head : m_capacity - m_head;
        if (writeLen > (length - amtWritten))
            writeLen = length - amtWritten;

        memcpy(&m_data[m_head], &src[amtWritten], writeLen);

        amtWritten += writeLen;
        m_used += writeLen;
        if (m_used > m_capacity)
            throw ECircularBufferError_BufferSizeHasBeenExceeded();

        m_head += writeLen;
        if (m_head == m_capacity)
            m_head = 0;
        else if (m_head > m_capacity)
            throw ECircularBufferError_HeadHasExceededCapacity();
    }
}

void ECircularBuffer::modifyData(pUChar src, Int offset, int length, Bool nolock)
{
    EMutexLock lockMutex(m_mutex, False);

    if (!nolock)
      lockMutex.acquire();

    //
    // Generally speaking, data is written to the head and read from the
    // tail.  modifyData updates data starting at the tail, the next data
    // to read.  This method was added to support partial socket writes
    // where the message length, the first 4 bytes, needed to updated to
    // reflect the remaining bytes to be written.
    //
    //                   **** USE WITH EXTREME CAUTION ***
    //

    // check to see if the data to be modified fits within the currently
    // used space of the circular buffer
    if (offset + length > used())
    {
        EString s;
        s.format("offset=%d length=%d used=%d", offset, length, used());
        ECircularBufferError_AttemptToModifyDataOutsideBoundsOfCurrentBuffer e;
        e.appendText(s.c_str());
        throw e;
    }

    Int idx = m_tail + offset;
    if (idx >= m_capacity)
        idx -= m_capacity;
    Int amtWritten = 0;

    while (amtWritten < length)
    {
        Int writeLen = length - amtWritten;
        if (idx + writeLen > m_capacity)
            writeLen = m_capacity - idx;

        memcpy(&m_data[idx], &src[amtWritten], writeLen);

        amtWritten += writeLen;

        idx += writeLen;
        if (idx == m_capacity)
            idx = 0;
    }
}
