////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftqbase_h_included
#define __ftqbase_h_included

#include "ftbase.h"
#include "fttime.h"
#include "fttimer.h"
#include "fttq.h"
#include "ftmessage.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR(FTQueueBaseError_NotOpenForWriting);
DECLARE_ERROR(FTQueueBaseError_NotOpenForReading);
DECLARE_ERROR(FTQueueBaseError_MultipleReadersNotAllowed);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTQueueBase;

class FTQueueMessage : public FTMessage
{
    friend class FTQueueBase;

public:
    FTQueueMessage()
    {
    }

    ~FTQueueMessage()
    {
    }

    FTQueueMessage& operator=(const FTQueueMessage& a)
    {
        m_timer = a.m_timer;
        m_msgType = a.m_msgType;
        return *this;
    }

    virtual Void getLength(ULong &length);
    virtual Void serialize(pVoid pBuffer, ULong &nOffset);
    virtual Void unserialize(pVoid pBuffer, ULong &nOffset);

    FTTime& getTimer()
    {
        return m_timer;
    }

    Void setMsgType(Long msgType) { m_msgType = msgType; }
    Long getMsgType() { return m_msgType; }

private:
    FTTime m_timer;
    Long m_msgType;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTQueuePublic;
class FTQueuePrivate;

class FTQueueBase
{
    friend class FTQueuePublic;
    friend class FTQueuePrivate;

public:
    enum Mode { ReadOnly, WriteOnly, ReadWrite };

    Bool push(FTQueueMessage &msg, Bool wait = True);
    FTQueueMessage* pop(Bool wait = True);

    FTSemaphore& getMsgSemaphore() { return semMsgs(); }

    Void destroy();

protected:
    virtual ULong& msgSize() = 0;
    virtual Int& msgCnt() = 0;
    virtual Long& msgHead() = 0;
    virtual Long& msgTail() = 0;
    virtual Bool& multipleReaders() = 0;
    virtual Bool& multipleWriters() = 0;
    virtual Int& numReaders() = 0;
    virtual Int& numWriters() = 0;
    virtual Int& refCnt() = 0;
    virtual pChar data() = 0;
    virtual Void allocDataSpace(cpStr sFile, Char cId, Int nSize) = 0;

    virtual FTMutex& writeMutex() = 0;
    virtual FTMutex& readMutex() = 0;
    virtual FTSemaphore& semMsgs() = 0;
    virtual FTSemaphore& semFree() = 0;
    virtual Int& semMsgsId() = 0;
    virtual Int& semFreeId() = 0;

    virtual FTQueueMessage* allocMessage(Long msgType) = 0;

    Mode mode() { return m_mode; }

    FTQueueBase();
    ~FTQueueBase();

    Void init(ULong nMsgSize, Int nMsgCnt, Int queueId, Bool bMultipleReaders,
        Bool bMultipleWriters, FTQueueBase::Mode eMode);

private:
    Bool m_initialized;
    Mode m_mode;
    Char m_rBuffer[USHRT_MAX];
    Char m_wBuffer[USHRT_MAX];
//    FTSemaphoreNotice m_semFreeNotice;
};

#endif // #define __ftqbase_h_included
