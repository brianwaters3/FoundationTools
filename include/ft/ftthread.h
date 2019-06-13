////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __ftthread_h_included
#define __ftthread_h_included

#include "fttbase.h"
#include "fttq.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadPublic : public FTThreadBase
{
public:
    FTThreadPublic();
    ~FTThreadPublic();

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
    virtual Void defMessageHandler(FTThreadMessage &msg);
    virtual Void messageQueued();

    FTThreadQueueBase& queue() { return m_queue; }

private:
    Short m_appId;
    UShort m_threadId;
    Int m_queueSize;

    FTThreadQueuePublic m_queue;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadPrivate : public FTThreadBase
{
public:
    FTThreadPrivate();
    ~FTThreadPrivate();

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
    virtual Void defMessageHandler(FTThreadMessage &msg);
    virtual Void messageQueued();

    FTThreadQueueBase& queue() { return m_queue; }

private:
    Short m_appId;
    UShort m_threadId;
    Int m_queueSize;

    FTThreadQueuePrivate m_queue;
};

#endif // #define __ftthread_h_included
