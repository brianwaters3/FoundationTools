////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftqpriv_h_included
#define __ftqpriv_h_included

#include "ftqbase.h"

class FTQueuePrivate : virtual public FTQueueBase
{
public:
    FTQueuePrivate();
    ~FTQueuePrivate();

    Void init(Int nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
        Bool bMultipleWriters, FTQueueBase::Mode eMode)
    {
        FTQueueBase::init(nMsgSize, nMsgCnt, queueId, bMultipleReaders, bMultipleWriters, eMode);
    }

protected:
    ULong& msgSize();
    Int& msgCnt();
    Long& msgHead();
    Long& msgTail();
    Bool& multipleReaders();
    Bool& multipleWriters();
    Int& numReaders();
    Int& numWriters();
    Int& refCnt();
    Int& semFreeId() { return m_semFree.getSemid(); }
    Int& semMsgsId() { return m_semMsgs.getSemid(); }
    pChar data();
    Int ctrlSize();
    Void allocDataSpace(cpStr sFile, Char cId, Int nSize);

    virtual FTQueueMessage* allocMessage(Long msgType) = 0;

private:
    Int m_refCnt;
    Int m_numReaders;
    Int m_numWriters;
    Bool m_multipleReaders;
    Bool m_multipleWriters;

    ULong m_msgSize;
    Int m_msgCnt;
    Long m_head; // next location to write
    Long m_tail; // next location to read

    FTMutex m_rmutex;
    FTMutex m_wmutex;
    FTSemaphore m_semFree;
    FTSemaphore m_semMsgs;

    FTMutex &readMutex() { return m_rmutex; }
    FTMutex &writeMutex() { return m_wmutex; }
    FTSemaphore &semFree() { return m_semFree; }
    FTSemaphore &semMsgs() { return m_semMsgs; }

    pChar m_pData;
};

#endif // #define __ftqpriv_h_included
