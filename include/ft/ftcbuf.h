////////////////////////////////////////////////////
// (c) 2012 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftcbuf_h_included
#define __ftcbuf_h_included

#include "ftbase.h"
#include "ftsynch.h"
#include "fterror.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(FTCircularBufferError_HeadAndTailOutOfSync);
DECLARE_ERROR(FTCircularBufferError_UsedLessThanZero);
DECLARE_ERROR(FTCircularBufferError_TailExceededCapacity);
DECLARE_ERROR(FTCircularBufferError_AttemptToExceedCapacity);
DECLARE_ERROR(FTCircularBufferError_BufferSizeHasBeenExceeded);
DECLARE_ERROR(FTCircularBufferError_HeadHasExceededCapacity);
DECLARE_ERROR(FTCircularBufferError_AttemptToModifyDataOutsideBoundsOfCurrentBuffer);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTCircularBuffer
{
public:
    FTCircularBuffer(Int capacity);
    ~FTCircularBuffer();

    Void initialize();

    Bool isEmpty()  { return m_used == 0; }
    Int capacity()  { return m_capacity; }
    Int used()      { return m_used; }
    Int free()      { return m_capacity - m_used; }

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

    FTCircularBuffer() {}

    pUChar m_data;
    Int m_capacity;
    Int m_head;
    Int m_tail;
    Int m_used;

    FTMutex m_mutex;
};

#endif // __ftcbuf_h_included
