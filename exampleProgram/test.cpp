/////////////////////////////////////////////////////////////////////////
// (c) 2009 Brian Waters (bdwaters@sbcglobal.net) All rights reserved. //
/////////////////////////////////////////////////////////////////////////
#include "stdio.h"
#include <iostream>
#include <memory.h>
#include "ft/ft.h"
#include "ft/ftinternal.h"
#include "ft/ftbzip2.h"
#include <locale>
//#include "odbx.h"
//#include "opendbx/api"

Void FTCircularBuffer_test()
{
    pUChar pData = (pUChar)"[1234567890123456789012345678]";
    UChar buf[256];

    memset(buf, 0, sizeof(buf));

    cout << "FTCircularBuffer_test() Start" << endl;

    FTCircularBuffer cb(50);

    for (int i=0; i<20; i++)
    {
        cout << "Pass " << i << " writing 30 bytes...";
        cb.writeData(pData, 0, 30);
        cout << " reading 30 bytes...";
        cb.readData(buf, 0, 30);
        cout << buf << endl;
    }

    cout << "FTCircularBuffer_test() Complete" << endl;
}

Void FTTimer_test()
{
    cout << "FTTimer_test() Start" << endl;

    FTTimerElapsed t;

    t.Start();

    for(int i=0; i<1000000; i++);

    //t.Stop();

    fttime_t a = t.MicroSeconds();
    fttime_t b = t.MilliSeconds();

    cout << "MicroSeconds() = " << a << endl;
    cout << "MilliSeconds() = " << b << endl;

    cout << "FTTimer_test() Complete" << endl;
}

Void FTMutex_test()
{
    cout << "FTMutex_test() Start" << endl;
    cout << "Creating unnamed (private) mutex" << endl;
    FTMutex m;
    {
        FTMutexLock l(m);
        cout << "Mutex locked.  Press [Enter] to continue ...";
        getc(stdin);
        cout << endl << "Destroying unnamed (private) mutex" << endl;
    }

    cout << "FTMutex_test() Complete" << endl;
}

Void FTSemaphore_test()
{
    cout << "FTSemaphore_test() Start" << endl;

    {
        cout << "Creating semaphore initial/max count of 5" << endl;
        FTSemaphore s1(5,5,"TestSemaphore");
        cout << "Decrementing";
        for (int i=1; i<=5; i++)
        {
            if (!s1.Decrement())
                cout << "Error decrementing semaphore on pass " << i << endl;
            cout << ".";
        }
        cout << "Checking for decrement action at zero...";
        if (s1.Decrement(False))
            cout << " failed - Decrement returned true" << endl;
        else
            cout << "success" << endl;
    }

    cout << "FTSemaphore_test() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define FTM_MUTEXTEST	(FTM_USER + 1)

class FTThreadMutexTest2 : public FTThreadPrivate
{
public:
    FTThreadMutexTest2(FTMutex &m1, int loop)
        : m_mutex1(m1),
          m_loop(loop)
    {
    }

    virtual Void onInit()
    {
        m_timer.Start();

        for (int i=0; i<m_loop; i++)
        {
            FTMutexLock l1(m_mutex1);
        }

        m_timer.Stop();

        quit();
    }

    virtual Void onQuit()
    {
        FTThreadPrivate::onQuit();
        cout << "FTThreadMutexTest2 elapsed time " << m_timer.MicroSeconds() - 1000000 << " microseconds" << endl;
    }

    DECLARE_MESSAGE_MAP()

private:
    FTThreadMutexTest2();

    FTTimerElapsed m_timer;
    FTMutex &m_mutex1;
    int m_loop;
};

BEGIN_MESSAGE_MAP(FTThreadMutexTest2, FTThreadPrivate)
END_MESSAGE_MAP()

Void FTMutex_test2()
{
    cout << "FTMutex_test2() Start" << endl;
    
    int loop = 1000000;
    FTMutex m1;

    FTThreadMutexTest2 t1(m1, loop);
    FTThreadMutexTest2 t2(m1, loop);

    {
        FTMutexLock l1(m1);

        t1.init(1, 1, NULL, 200000);
        t2.init(1, 2, NULL, 200000);

        cout << "Sleeping for 1 sercond ... ";
        FTThreadBasic::sleep(1000);
    }

    cout << "test started" << endl;

    t1.join();
    t2.join();

    cout << "FTMutex_test2() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTThreadBasicCancelWaitTest : public FTThreadBasic
{
public:
    Bool decrementResult;
    FTSemaphore* runningSemaphore;

	FTThreadBasicCancelWaitTest()
	{
        decrementResult = False;
        runningSemaphore = new FTSemaphore(0,1,"RunningSemaphore");
	}

	Dword threadProc(Void* arg)
	{
		int oldstate = 0;
//		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
//		cout << "PTHREAD_CANCEL_ENABLE=" << PTHREAD_CANCEL_ENABLE << " PTHREAD_CANCEL_DISABLE=" << PTHREAD_CANCEL_DISABLE << " old cancel state [" << oldstate << "]" << endl;
//		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);
//		cout << ((oldstate==PTHREAD_CANCEL_DEFERRED) ? "cancellation was deferred" : "cancellation was NOT deferred") << endl;
        FTSemaphore s(0,1,"TestSemaphore");
        runningSemaphore->Increment();
        try
        {
        	decrementResult = s.Decrement();
        	cout << "after the decrement in the test thread" << endl;
        }
        catch (FTError * e)
        {
            cout << e->str() << endl;
        }
        catch (...)
        {
        	cout << "an exception occurred" << endl;
        }
        return 0;
	}
};

Void FTThread_cancel_wait()
{
    cout << "FTThread_cancel_wait() Start" << endl;

    FTThreadBasicCancelWaitTest t;

    t.init((Void*)"this is the thread argument\n", true);
    cout << "before resume" << endl;
    t.resume();
    cout << "waiting for test thread to start" << endl;
    t.runningSemaphore->Decrement();
    Int x = 10000;
    cout << "sleeping for " << x / 1000 << " seconds" << endl;
    t.sleep(x);
    cout << "before cancelWait()" << endl;
    cout << "cancelWait() returned " << t.cancelWait() << endl;
    cout << "before join" << endl;
    t.join();

    if (t.decrementResult)
		cout << "The cancel wait test semaphore decrement returned True (error)" << endl;
    else
		cout << "The cancel wait test semaphore decrement returned False (success)" << endl;

    cout << "FTThread_cancel_wait() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void FTSemaphore_test_cancel_wait()
{
    cout << "FTSemaphore_test() Start" << endl;

    {
        cout << "Creating semaphore initial/max count of 5" << endl;
        FTSemaphore s1(5,5,"TestSemaphore");
        cout << "Decrementing";
        for (int i=1; i<=5; i++)
        {
            if (!s1.Decrement())
                cout << "Error decrementing semaphore on pass " << i << endl;
            cout << ".";
        }
        cout << "Checking for decrement action at zero...";
        if (s1.Decrement(False))
            cout << " failed - Decrement returned true" << endl;
        else
            cout << "success" << endl;
    }

    cout << "FTSemaphore_test() Complete" << endl;
}

Void FTError_test()
{
    cout << "FTError_test() Start" << endl;

    cout << "Creating error object" << endl;
    FTError e1;
    e1.appendTextf("error object #%d", 1);
    cout << e1.str() << endl;

    cout << "FTMutex_test() Complete" << endl;
}

class FTThreadBasicTest : public FTThreadBasic
{
public:
	FTThreadBasicTest()
	{
		m_timetoquit = false;
	}

	Dword threadProc(Void* arg)
	{
		while(!m_timetoquit)
		{
			cout << "Inside the thread " << (cpStr)arg << endl;
			sleep(1000);
		}
		cout << "Exiting FTThreadTest::threadProc()" << endl;
		return 0;
	}

	Void setTimeToQuit()
	{
		m_timetoquit = true;
	}

private:
    bool m_timetoquit;
};

Void FTThreadBasic_test()
{
    cout << "FTThread_test() Start" << endl;

    FTThreadBasicTest t;

    t.init((Void*)"this is the thread argument\n", true);
    cout << "before resume" << endl;
    t.resume();
    cout << "before 5 second sleep sleep" << endl;
    t.sleep(5000);
    cout << "before setTimeToQuit()" << endl;
    t.setTimeToQuit();
    cout << "before 2nd 5 second sleep" << endl;
    t.sleep(5000);
    cout << "before join" << endl;
    t.join();

    cout << "FTThread_test() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define FTM_USER1	(FTM_USER + 1)
#define FTM_USER2	(FTM_USER + 2)
#define FTM_USER3	(FTM_USER + 3)
#define FTM_USER4	(FTM_USER + 4)

class FTThreadTest4 : public FTThreadPrivate
{
public:
    FTThreadTest4(int mm)
    {
        msg1snt = 0;
        msg1rcv = 0;
        msgmax = mm;
    }

    Void userFunc1(FTThreadMessage& msg)
    {
        msg1rcv++;
        if (msg1rcv == msgmax)
            quit();
        else if (msg1snt < msgmax)
            sendMessage(FTM_USER1, 0, (Long)msg1snt++);
    }

    Void onInit()
    {
        printf("Inside onInit()\n");

        te.Start();

        //sendMessage(FTM_USER1, 0, (Long)msg1snt++);
        sendMessage(FTM_USER1, 0, (Long)msg1snt++);
    }

    virtual Void onQuit()
    {
        te.Stop();

        FTThreadPrivate::onQuit();
        cout << "FTThreadTest4 sent     " << msg1snt << " FTM_USER1 messages" << endl;
        cout << "FTThreadTest4 received " << msg1rcv << " FTM_USER1 messages" << endl;
    }

    Void onSuspend()
    {
        cout << "Inside onSuspend()" << endl;
        FTThreadPrivate::onSuspend();
    }

    FTTimerElapsed &getTimer() { return te; }

    DECLARE_MESSAGE_MAP()

private:
    FTThreadTest4();

    int msg1snt;
    int msg1rcv;
    int msgmax;
    FTTimerElapsed te;
};

class FTThreadTest : public FTThreadPublic
{
public:
    FTThreadTest()
    {
        msg1cnt = 0;
        msg2cnt = 0;
    }

    Void userFunc1(FTThreadMessage& msg)
    {
        msg1cnt++;
    }

    Void userFunc2(FTThreadMessage& msg)
    {
        msg2cnt++;
    }

    //	Void onInit()
    //	{
    //		printf("Inside onInit()\n");
    //	}

    virtual Void onQuit()
    {
        FTThreadPublic::onQuit();
        cout << "FTThreadTest received " << msg1cnt << " FTM_USER1 messages" << endl;
        cout << "FTThreadTest received " << msg2cnt << " FTM_USER2 messages" << endl;
    }

    Void onSuspend()
    {
        cout << "Inside onSuspend()" << endl;
        FTThreadPublic::onSuspend();
    }

    DECLARE_MESSAGE_MAP()

private:
    int msg1cnt;
    int msg2cnt;
};

class FTThreadTest2 : public FTThreadTest
{
public:
    FTThreadTest2()
    {
        msg3cnt = 0;
        msg4cnt = 0;
    }

    Void userFunc3(FTThreadMessage& msg)
    {
        msg3cnt++;
    }

    Void userFunc4(FTThreadMessage& msg)
    {
        msg4cnt++;
    }

    virtual Void onQuit()
    {
        FTThreadTest::onQuit();
        cout << "FTThreadTest2 received " << msg3cnt << " FTM_USER3 messages" << endl;
        cout << "FTThreadTest2 received " << msg4cnt << " FTM_USER4 messages" << endl;
    }

    DECLARE_MESSAGE_MAP()

private:
    int msg3cnt;
    int msg4cnt;
};

class FTThreadTest3 : public FTThreadPrivate
{
public:
    FTThreadTest3()
    {
        msg1cnt = 0;
        msg2cnt = 0;
        msg3cnt = 0;
        msg4cnt = 0;
    }

    Void userFunc1(FTThreadMessage& msg)
    {
        msg1cnt++;
    }

    Void userFunc2(FTThreadMessage& msg)
    {
        msg2cnt++;
    }

    Void userFunc3(FTThreadMessage& msg)
    {
        msg3cnt++;
    }

    Void userFunc4(FTThreadMessage& msg)
    {
        msg4cnt++;
    }

    virtual Void onQuit()
    {
        FTThreadPrivate::onQuit();
        cout << "FTThreadTest3 received " << msg1cnt << " FTM_USER1 messages" << endl;
        cout << "FTThreadTest3 received " << msg2cnt << " FTM_USER2 messages" << endl;
        cout << "FTThreadTest3 received " << msg3cnt << " FTM_USER3 messages" << endl;
        cout << "FTThreadTest3 received " << msg4cnt << " FTM_USER4 messages" << endl;
    }

    DECLARE_MESSAGE_MAP()

private:
    int msg1cnt;
    int msg2cnt;
    int msg3cnt;
    int msg4cnt;
};

BEGIN_MESSAGE_MAP(FTThreadTest, FTThreadPublic)
    ON_MESSAGE(FTM_USER1, FTThreadTest::userFunc1)
    ON_MESSAGE(FTM_USER2, FTThreadTest::userFunc2)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(FTThreadTest2, FTThreadTest)
    ON_MESSAGE(FTM_USER3, FTThreadTest2::userFunc3)
    ON_MESSAGE(FTM_USER4, FTThreadTest2::userFunc4)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(FTThreadTest3, FTThreadPrivate)
    ON_MESSAGE(FTM_USER1, FTThreadTest3::userFunc1)
    ON_MESSAGE(FTM_USER2, FTThreadTest3::userFunc2)
    ON_MESSAGE(FTM_USER3, FTThreadTest3::userFunc3)
    ON_MESSAGE(FTM_USER4, FTThreadTest3::userFunc4)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(FTThreadTest4, FTThreadPrivate)
    ON_MESSAGE(FTM_USER1, FTThreadTest4::userFunc1)
END_MESSAGE_MAP()

template<class T>
std::string numberFormatWithCommas(T value, int precision=6){
    struct Numpunct: public std::numpunct<char>{
    protected:
        virtual char do_thousands_sep() const{return ',';}
        virtual std::string do_grouping() const{return "\03";}
    };
    std::stringstream ss;
    ss.imbue({std::locale(), new Numpunct});
    ss << std::setprecision(precision) << std::fixed << value;
    return ss.str();
}

Void FTThread_test4()
{
    int maxmsg = 100000000;

    FTThreadTest4 t1(maxmsg);

    t1.init(1, 1, NULL, 200000);

    FTThreadBasic::yield();

    t1.join();

    double persec = ((double)maxmsg)/(((double)t1.getTimer().MicroSeconds())/1000000);
    std::string s = numberFormatWithCommas<double>(persec);
    cout << "Processed " << maxmsg << " messages in " << ((double)t1.getTimer().MicroSeconds())/1000000 <<
        " seconds (" << s << " per second)" << endl;
        //" seconds (" << (((double)maxmsg) / (((double)te.MicroSeconds())/1000000)) << " per second)" << endl;
}

class FTThreadTest5rcv : public FTThreadPrivate
{
public:
    FTThreadTest5rcv(int senders)
    {
        msg1rcv = 0;
        msg2rcv = 0;
        msg3rcv = 0;
		msgsenders = senders;
		maxpending = 0;
    }

    Void rcv1(FTThreadMessage& msg)
    {
        msg1rcv++;

        if (msg1rcv == 1)
			te.Start();
    }

	Void rcv2(FTThreadMessage& msg)
	{
		Long pending = getMsgSemaphore().getCurrCount();
		if (pending > maxpending)
			maxpending = pending;

        msg2rcv++;

		if (msg2rcv % 1000000 == 0)
			cout << "\rFTThreadTest5 received " << msg2rcv << " max pending " << maxpending << flush;
	}

	Void rcv3(FTThreadMessage& msg)
	{
        msg3rcv++;

		if (msg3rcv == msgsenders)
		{
			te.Stop();
			quit();
		}
	}

    virtual Void onQuit()
    {
        FTThreadPrivate::onQuit();
        cout << "\rFTThreadTest5rcv received " << msg2rcv << " FTM_USER2 messages with max pending of " << maxpending << endl << flush;
    }

    FTTimerElapsed &getTimer() { return te; }

    DECLARE_MESSAGE_MAP()

private:
    FTThreadTest5rcv();

    int msg1rcv;
    int msg2rcv;
    int msg3rcv;
    int msgsenders;
	Long maxpending;
    FTTimerElapsed te;
};

BEGIN_MESSAGE_MAP(FTThreadTest5rcv, FTThreadPrivate)
    ON_MESSAGE(FTM_USER1, FTThreadTest5rcv::rcv1)
    ON_MESSAGE(FTM_USER2, FTThreadTest5rcv::rcv2)
    ON_MESSAGE(FTM_USER3, FTThreadTest5rcv::rcv3)
END_MESSAGE_MAP()

class FTThreadTest5snd : public FTThreadBasic
{
public:
	FTThreadTest5snd(FTThreadTest5rcv* receiver, int cnt)
	{
		msgreceiver = receiver;
		msgcnt = cnt;
		msgsnt = 0;
	}

	Dword threadProc(Void* arg)
	{
        msgreceiver->sendMessage(FTM_USER1);

		for (msgsnt=0; msgsnt<msgcnt; msgsnt++)
		{
           if (!msgreceiver->sendMessage(FTM_USER2))
			   cout << endl << "sendMessage returned FALSE for message # " << msgsnt << endl << flush;
		}

        msgreceiver->sendMessage(FTM_USER3);

		return 0;
	}

	int getMessagesSent() { return msgsnt; }

private:
	FTThreadTest5rcv* msgreceiver;
	int msgcnt;
	int msgsnt;
};

Void FTThread_test5()
{
    static Int nSenders = 1;
    static Int nMessages = 1000000;
    Char buffer[128];
	FTThreadTest5snd** sendthrds = NULL;

    cout << "Enter number of sending threads [" << nSenders << "]: ";
    cin.getline(buffer, sizeof(buffer));
    nSenders = buffer[0] ? atoi(buffer) : nSenders;

    cout << "Enter message count [" << nMessages << "]: ";
    cin.getline(buffer, sizeof(buffer));
    nMessages = buffer[0] ? atoi(buffer) : nMessages;


    FTThreadTest5rcv t1(nSenders);

    t1.init(1, 1, NULL, 200000);

	sendthrds = new FTThreadTest5snd*[nSenders];

	for (int i=0; i<nSenders; i++)
	{
		sendthrds[i] = new FTThreadTest5snd(&t1, nMessages);
		sendthrds[i]->init(NULL);
	}

    FTThreadBasic::yield();

    t1.join();
	for (int i=0; i<nSenders; i++)
	{
		sendthrds[i]->join();
		delete sendthrds[i];
	}
	delete [] sendthrds;

    double persec = ((double)(nSenders*nMessages))/(((double)t1.getTimer().MicroSeconds())/1000000);
    std::string s = numberFormatWithCommas<double>(persec);
    cout << "Processed " << (nSenders*nMessages) << " messages in " << ((double)t1.getTimer().MicroSeconds())/1000000 <<
        " seconds (" << s << " per second)" << endl;
}

Void FTThread_test()
{
    FTThreadTest2 t1;

    t1.init(1, 1, NULL, 200000);
    FTThreadBasic::yield();

    FTTimerElapsed te;

    te.Start();
    int i;

    for (i=0; i<4000000; i++)
    {
        t1.sendMessage(FTM_USER1, 0, (Long)i);
        t1.sendMessage(FTM_USER2, 0, (Long)i);
        t1.sendMessage(FTM_USER3, 0, (Long)i);
        t1.sendMessage(FTM_USER4, 0, (Long)i);
        //        FTThreadBasic::yield();
    }

    t1.quit();
    t1.join();

    i *= 4;
    cout << "Processed " << i << " messages in " << ((double)te.MicroSeconds())/1000000 <<
        " seconds (" << ((double)i) / (((double)te.MicroSeconds())/1000000) << " per second)" << endl;
}

Void FTThread_test2()
{
    FTThreadTest2 t1;
    FTThreadTest2 t2;
    FTThreadTest2 t3;
    FTThreadTest2 t4;

    t1.init(1, 1, NULL, 200000);
    t2.init(1, 2, NULL, 200000);
    t3.init(1, 3, NULL, 200000);
    t4.init(1, 4, NULL, 200000);
    t1.yield();

    FTTimerElapsed te;

    te.Start();
    int i;

    for (i=0; i<1000000; i++)
    {
        for (int i=0; i<4; i++)
        {
            try
            {
                switch (i)
                {
                    case 0:
                        t1.sendMessage(FTM_USER1, 0, (Long)i);
                        t2.sendMessage(FTM_USER1, 0, (Long)i);
                        t3.sendMessage(FTM_USER1, 0, (Long)i);
                        t4.sendMessage(FTM_USER1, 0, (Long)i);
                        break;
                    case 1:
                        t1.sendMessage(FTM_USER2, 0, (Long)i);
                        t2.sendMessage(FTM_USER2, 0, (Long)i);
                        t3.sendMessage(FTM_USER2, 0, (Long)i);
                        t4.sendMessage(FTM_USER2, 0, (Long)i);
                        break;
                    case 2:
                        t1.sendMessage(FTM_USER3, 0, (Long)i);
                        t2.sendMessage(FTM_USER3, 0, (Long)i);
                        t3.sendMessage(FTM_USER3, 0, (Long)i);
                        t4.sendMessage(FTM_USER3, 0, (Long)i);
                        break;
                    case 3:
                        t1.sendMessage(FTM_USER4, 0, (Long)i);
                        t2.sendMessage(FTM_USER4, 0, (Long)i);
                        t3.sendMessage(FTM_USER4, 0, (Long)i);
                        t4.sendMessage(FTM_USER4, 0, (Long)i);
                        break;
                }
            }
            catch (FTError *e)
            {
                cout << "Processing exception " << e->Name() << " - " << *e << endl;
                throw;
            }
        }
//        FTThreadBasic::yield();
    }

    t1.quit();
    t2.quit();
    t3.quit();
    t4.quit();
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    i *= 16;
    cout << "Processed " << i << " messages in " << ((double)te.MicroSeconds())/1000000 <<
        " seconds (" << ((double)i) / (((double)te.MicroSeconds())/1000000) << " per second)" << endl;
}

Void FTThread_test3()
{
    FTThreadTest3 t1;
    FTThreadTest3 t2;
    FTThreadTest3 t3;
    FTThreadTest3 t4;

    t1.init(1, 1, NULL, 200000);
    t2.init(1, 2, NULL, 200000);
    t3.init(1, 3, NULL, 200000);
    t4.init(1, 4, NULL, 200000);
    t1.yield();

    FTTimerElapsed te;

    te.Start();
    int i;

    for (i=0; i<1000000; i++)
    {
        for (int i=0; i<4; i++)
        {
            try
            {
                switch (i)
                {
                    case 0:
                        t1.sendMessage(FTM_USER1, 0, (Long)i);
                        t2.sendMessage(FTM_USER1, 0, (Long)i);
                        t3.sendMessage(FTM_USER1, 0, (Long)i);
                        t4.sendMessage(FTM_USER1, 0, (Long)i);
                        break;
                    case 1:
                        t1.sendMessage(FTM_USER2, 0, (Long)i);
                        t2.sendMessage(FTM_USER2, 0, (Long)i);
                        t3.sendMessage(FTM_USER2, 0, (Long)i);
                        t4.sendMessage(FTM_USER2, 0, (Long)i);
                        break;
                    case 2:
                        t1.sendMessage(FTM_USER3, 0, (Long)i);
                        t2.sendMessage(FTM_USER3, 0, (Long)i);
                        t3.sendMessage(FTM_USER3, 0, (Long)i);
                        t4.sendMessage(FTM_USER3, 0, (Long)i);
                        break;
                    case 3:
                        t1.sendMessage(FTM_USER4, 0, (Long)i);
                        t2.sendMessage(FTM_USER4, 0, (Long)i);
                        t3.sendMessage(FTM_USER4, 0, (Long)i);
                        t4.sendMessage(FTM_USER4, 0, (Long)i);
                        break;
                }
            }
            catch (FTError *e)
            {
                cout << "Processing exception " << e->Name() << " - " << *e << endl;
                throw;
            }
        }
//        FTThreadBasic::yield();
    }

    t1.quit();
    t2.quit();
    t3.quit();
    t4.quit();
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    i *= 16;
    cout << "Processed " << i << " messages in " << ((double)te.MicroSeconds())/1000000 <<
        " seconds (" << ((double)i) / (((double)te.MicroSeconds())/1000000) << " per second)" << endl;
}

void FTSharedMemory_test()
{
    FTSharedMemory m("test", 1, 1024 * 1024);
}

class testmessage : public FTQueueMessage
{
public:
    testmessage()
    {
        ft_strcpy_s(m_data, sizeof(m_data),
            "This is a shared queue test. Four score and 7 years ago, our fathers");
    }

    ~testmessage() {}

    virtual Void getLength(ULong &length)
    {
        FTQueueMessage::getLength(length);
        elementLength(m_data, length);
    }
    virtual Void serialize(pVoid pBuffer, ULong& nOffset)
    {
        FTQueueMessage::serialize(pBuffer, nOffset);
        pack(m_data, pBuffer, nOffset);
    }
    virtual Void unserialize(pVoid pBuffer, ULong& nOffset)
    {
        FTQueueMessage::unserialize(pBuffer, nOffset);
        unpack(m_data, pBuffer, nOffset);
    }

    Char m_data[128];
};

class TestPublicQueue : public FTQueuePublic
{
public:

    FTQueueMessage* allocMessage(Long msgType)
    {
        return & msg;
    }

    testmessage& getMessage()
    {
        return msg;
    }

private:
    testmessage msg;
};

void FTQueuePublic_test(Bool bWriter)
{
    static Int nQueueId = 1;
    static Int nMsgCnt = 100000;
    Char buffer[128];
    TestPublicQueue q;

    cout << "Enter Queue Id [" << nQueueId << "]: ";
    cin.getline(buffer, sizeof(buffer));
    nQueueId = buffer[0] ? atoi(buffer) : nQueueId;

    cout << "Enter message count [" << nMsgCnt << "]: ";
    cin.getline(buffer, sizeof(buffer));
    nMsgCnt = buffer[0] ? atoi(buffer) : nMsgCnt;

    try
    {
        q.init(nQueueId, bWriter ? FTQueueBase::WriteOnly : FTQueueBase::ReadOnly);
    }
    catch (FTError *e)
    {
        cout << e->Name() << " - " << *e << endl;
        delete e;
        return;
    }

    testmessage msg;
    FTTimerElapsed te;

    Int cnt = 0;

    if (bWriter)
    {
        // writer
        for (cnt=0; cnt<nMsgCnt; cnt++)
            q.push(msg);
    }
    else
    {
        // reader
        while (cnt < nMsgCnt)
        {
            cnt++;
            q.pop();
        }
    }

    cout << "Processed " << cnt << " messages in " << ((double)te.MicroSeconds())/1000000
        << " seconds (" << ((double)cnt) / (((double)te.MicroSeconds())/1000000) << " per second)" << endl;
}

//class TestSemNoticeQueue : public FTQueuePublic
//{
//public:
//    FTQueueMessage* allocMessage(Long msgType)
//    {
//        return & msg;
//    }
//
//    testmessage& getMessage()
//    {
//        return msg;
//    }
//
//private:
//    testmessage msg;
//};
//
//Void FTSemaphoreNotice_test_reader()
//{
//    cout << "FTSemaphoreNotice_test_reader start" << endl;
//    testmessage *pMsg;
//
//    TestSemNoticeQueue q1;
//    TestSemNoticeQueue q3;
//    TestSemNoticeQueue q2;
//
//    try
//    {
//        cout << "Initializing queue 1" << endl;
//        q1.init(1, FTQueueBase::ReadOnly);
//        cout << "Initializing queue 2" << endl;
//        q2.init(2, FTQueueBase::ReadOnly);
//        cout << "Initializing queue 3" << endl;
//        q3.init(3, FTQueueBase::ReadOnly);
//    }
//    catch (FTError *e)
//    {
//        cout << e->Name() << " - " << *e << endl;
//        delete e;
//        return;
//    }
//
//    FTSemaphoreNotice n;
//
//    cout << "Initializing semaphore notification object" << endl;
//    n.init();
//
//    cout << "Adding queue 1 message semaphore to notification object" << endl;
//    n.add(q1.getMsgSemaphore());
//    cout << "Adding queue 2 message semaphore to notification object" << endl;
//    n.add(q2.getMsgSemaphore());
//    cout << "Adding queue 3 message semaphore to notification object" << endl;
//    n.add(q3.getMsgSemaphore());
//
//    n.startNotify();
//    do
//    {
//        if (q1.pop())
//        {
//            pMsg = &q1.getMessage();
//            cout << "queue 1 message: " << q1.getMessage().m_data << endl;
//        }
//        else if (q2.pop())
//        {
//            pMsg = &q1.getMessage();
//            cout << "queue 2 message: " << q2.getMessage().m_data << endl;
//        }
//        else if (q3.pop())
//        {
//            pMsg = &q1.getMessage();
//            cout << "queue 3 message: " << q3.getMessage().m_data << endl;
//        }
//        else
//        {
//            n.wait();
//        }
//    } while (pMsg->m_data[0] != 'q' && pMsg->m_data[0] != 'Q');
//    n.stopNotify();
//
//    cout << "FTSemaphoreNotice_test_reader complete" << endl;
//}
//
//Void FTSemaphoreNotice_test_writer()
//{
//    cout << "FTSemaphoreNotice_test_writer start" << endl;
//
//    Bool bQuit = False;
//    Char queueid[128];
//    Char queuemsg[128];
//    testmessage msg;
//    TestSemNoticeQueue q1;
//    TestSemNoticeQueue q3;
//    TestSemNoticeQueue q2;
//
//    try
//    {
//        cout << "Initializing queue 1" << endl;
//        q1.init(1, FTQueueBase::ReadOnly);
//        cout << "Initializing queue 2" << endl;
//        q2.init(2, FTQueueBase::ReadOnly);
//        cout << "Initializing queue 3" << endl;
//        q3.init(3, FTQueueBase::ReadOnly);
//    }
//    catch (FTError *e)
//    {
//        cout << e->Name() << " - " << *e << endl;
//        delete e;
//        return;
//    }
//
//    while (True)
//    {
//        while (1)
//        {
//            cout << "Queue Id : ";
//            cin.getline(queueid, sizeof(queueid));
//            switch (queueid[0])
//            {
//                case '1':
//                case '2':
//                case '3':
//                    break;
//                case 'q':
//                case 'Q':
//                    bQuit = True;
//                    break;
//                default:
//                    continue;
//            }
//            break;
//        }
//
//        if (bQuit)
//            break;
//
//        cout << "Message  : ";
//        cin.getline(queuemsg, sizeof(queuemsg));
//
//        ft_strcpy_s(msg.m_data, sizeof(msg.m_data), queuemsg);
//
//        switch (queueid[0])
//        {
//            case '1':
//                q1.push(msg);
//                break;
//            case '2':
//                q2.push(msg);
//                break;
//            case '3':
//                q3.push(msg);
//                break;
//        }
//    }
//
//    cout << "FTSemaphoreNotice_test_writer complete" << endl;
//}

#define LOG_MASK_1 0x0000000000000001
#define LOG_MASK_2 0x0000000000000002
#define LOG_MASK_3 0x0000000000000004
#define LOG_MASK_4 0x0000000000000008

Void FTLogger_test()
{
    cout << "FTLogger_test start" << endl;

    FTLogger::setGroupMask(1, 0);
    FTLogger::setGroupMask(2, 0);

    FTLogger::logError(1, LOG_MASK_1, "FTLogger_test", "This line should not be logged, because it is not enabled.");
    FTLogger::enableGroupMask(1, LOG_MASK_1);
    FTLogger::logError(1, LOG_MASK_1, "FTLogger_test", "This line should be logged.");

    cout << "Testing single file log rollover." << endl;
    FTString s;
    Int i;
    for (i = 0; i<19; i++)
        FTLogger::logInfo(1, LOG_MASK_1, "FTLogger_test", "LOG_MASK_1 Line %d", i + 1);
    cout << "  Review log file 1.  There should be 10 lines in the log file, numbers 10-19." << endl;

    FTLogger::disableGroupMask(1, LOG_MASK_1);
    FTLogger::logInfo(1, LOG_MASK_1, "FTLogger_test", "This line should not be logged because it has been disabled.");

    cout << "Testing log rollover with 2 files." << endl;
    FTLogger::enableGroupMask(2, LOG_MASK_2);
    for (i = 0; i<20; i++)
        FTLogger::logInfo(2, LOG_MASK_2, "FTLogger_test", "LOG_MASK_2 Line %d", i + 1);
    cout << "  Review log file 2.  There should be 2 segments with 10 lines each." << endl;

    cout << "Testing syslog." << endl;
    FTLogger::enableGroupMask(3, LOG_MASK_2);
    FTLogger::logInfo(3, LOG_MASK_2, "FTLogger_test", "Info message");
    FTLogger::logWarning(3, LOG_MASK_2, "FTLogger_test", "Warning message");
    FTLogger::logError(3, LOG_MASK_2, "FTLogger_test", "Error message");
    cout << "  Review log file 3.  There should be 2 segments with 10 lines each." << endl;

    cout << "FTLogger_test complete" << endl;
}

Void FTDateTime_test()
{
    FTTime t;

    t.Now();

    FTString s;
    t.Format(s, "%Y-%m-%d %H:%M:%S.%0", False);
    cout << s << endl;
    t.Format(s, "%Y-%m-%d %H:%M:%S.%0", True);
    cout << s << endl;

    cpStr pszDate;

    pszDate = "2009-12-09 09:12:56.373";
    t.ParseDateTime(pszDate, True);
    t.Format(s, "%Y-%m-%d %H:%M:%S.%0", True);
    cout << "Is [" << pszDate << "] equal to [" << s << "]" << endl;

    pszDate = "09 Dec 2009 11:42:13:767";
    t.ParseDateTime(pszDate, False);
    t.Format(s, "%Y-%m-%d %H:%M:%S.%0", False);
    cout << " Is [" << pszDate << "] equal to [" << s << "]" << endl;
}

class FTThreadTimerTest : public FTThreadPrivate
{
public:
    FTThreadTimerTest()
    {
    }

    Void onInit()
    {
		if (m_oneshot)
		{
			m_cnt = 0;
			m_timer1.setInterval(5000);
			m_timer1.setOneShot(True);
			m_timer2.setInterval(10000);
			m_timer2.setOneShot(True);
			m_timer3.setInterval(15000);
			m_timer3.setOneShot(True);

			initTimer(m_timer1);
			initTimer(m_timer2);
			initTimer(m_timer3);

			m_elapsed.Start();

			m_timer1.start();
			m_timer2.start();
			m_timer3.start();
		}
		else
		{
			m_cnt = 0;
			m_timer1.setInterval(15000);
			m_timer1.setOneShot(m_oneshot);
			initTimer(m_timer1);
			m_elapsed.Start();
			m_timer1.start();
		}
    }

    Void onTimer(FTThreadBase::Timer* pTimer)
    {
        cout << m_elapsed.MilliSeconds(True) << " milliseconds has elapsed." << endl;
		if (pTimer->getId() == m_timer1.getId())
		{
			m_cnt++;
			if (m_cnt == 10)
				quit();
		}
		else if (pTimer->getId() == m_timer2.getId())
		{
		}
		else if (pTimer->getId() == m_timer3.getId())
		{
			quit();
		}
    }

    Void onQuit()
    {
    }

    Void setOneShot(Bool oneshot) { m_oneshot = oneshot; }

    DECLARE_MESSAGE_MAP()

private:
    Int m_cnt;
    Bool m_oneshot;
    FTThreadBase::Timer m_timer1;
    FTThreadBase::Timer m_timer2;
    FTThreadBase::Timer m_timer3;
    FTTimerElapsed m_elapsed;
};

BEGIN_MESSAGE_MAP(FTThreadTimerTest, FTThreadPrivate)
END_MESSAGE_MAP()

Void FTThreadTimerPeriodic_test()
{
    FTThreadTimerTest t;
    t.setOneShot(False);
    t.init(1, 1, NULL, 2000);
    t.join();
}

Void FTThreadTimerOneShot_test()
{
    FTThreadTimerTest t;
    t.setOneShot(True);
    t.init(1, 1, NULL, 2000);
    t.join();
}

//Void OpenDBX_test()
//{
//    try
//    {
//        OpenDBX::Conn db("mssql", "sqlstandby", "1433");
//        db.bind("gy_dev", "sagetelecom_net\\bwaters", "Liefje15", ODBX_BIND_SIMPLE);
////        cpStr sql = "sp_who";
//        cpStr sql = "EXEC P_TEST @CATEGORY='FinalUnitAction'";
//        OpenDBX::Result r = db.create(sql).execute();
//
//        odbxres stat;
//
//        while ((stat = r.getResult()) != ODBX_RES_DONE)
//        {
//            switch(stat)
//            {
//            case ODBX_RES_TIMEOUT:
//                continue;
//            case ODBX_RES_NOROWS:
//                continue;
//            }
//
//            Int spid;
//            FTString sCategory;
//            Int nReferenceId;
//            FTString sDescription;
//
//            while (r.getRow() != ODBX_ROW_DONE)
//            {
//                sCategory = r.fieldValue(0);
//                nReferenceId = (Int)strtol(r.fieldValue(1), NULL, 0);
//                sDescription = r.fieldValue(2);
//                cout << sCategory << "|" << nReferenceId << "|" << sDescription << endl;
//            }
//        }
//    }
//    catch (OpenDBX::Exception ex)
//    {
//        cout << "EXCEPTION: "<< ex.what() << endl;
//    }
//}

Void FTHash_test()
{
    cout << "FThash_test start" << endl;

    ULong hash;
    FTString s;

    // FTString short
    {
    	FTString s;
    	s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456";
        hash = FTHash::getHash(s);
        cout << std::hex << std::uppercase << hash << " for FTString [" << s << "]" << endl;
    }

    {
    	FTString s;
    	s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123457";
        hash = FTHash::getHash(s);
        cout << std::hex << std::uppercase << hash << " for FTString [" << s << "]" << endl;
    }

    // FTString long
    {
		FTString s;
		s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		hash = FTHash::getHash(s);
		cout << std::hex << std::uppercase << hash << " for FTString [" << s << "]" << endl;
    }

    {
		FTString s;
		s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456780";
		hash = FTHash::getHash(s);
		cout << std::hex << std::uppercase << hash << " for FTString [" << s << "]" << endl;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    {
    	cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456";
        hash = FTHash::getHash(s, strlen(s));
        cout << std::hex << std::uppercase << hash << " for cpChar [" << s << "]" << endl;
    }

    {
    	cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123457";
        hash = FTHash::getHash(s, strlen(s));
        cout << std::hex << std::uppercase << hash << " for cpChar [" << s << "]" << endl;
    }

    {
		cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		hash = FTHash::getHash(s, strlen(s));
		cout << std::hex << std::uppercase << hash << " for cpChar [" << s << "]" << endl;
    }

    {
		cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456780";
		hash = FTHash::getHash(s, strlen(s));
		cout << std::hex << std::uppercase << hash << " for cpChar [" << s << "]" << endl;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    {
    	cpUChar s = (cpUChar)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456";
        hash = FTHash::getHash(s, strlen((cpChar)s));
        cout << std::hex << std::uppercase << hash << " for cpUChar [" << s << "]" << endl;
    }

    {
    	cpUChar s = (cpUChar)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123457";
        hash = FTHash::getHash(s, strlen((cpChar)s));
        cout << std::hex << std::uppercase << hash << " for cpUChar [" << s << "]" << endl;
    }

    {
    	cpUChar s = (cpUChar)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		hash = FTHash::getHash(s, strlen((cpChar)s));
		cout << std::hex << std::uppercase << hash << " for cpUChar [" << s << "]" << endl;
    }

    {
    	cpUChar s = (cpUChar)"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456780";
		hash = FTHash::getHash(s, strlen((cpChar)s));
		cout << std::hex << std::uppercase << hash << " for cpUChar [" << s << "]" << endl;
    }

    cout << "FThash_test complete" << endl;
}

Void FTDirectory_test()
{
    Char path[512];
	Char mask[512];

	cout << "Path to Search : ";
    cin.getline(path, sizeof(path));
    cout << endl;

	cout << "Search Mask : ";
    cin.getline(mask, sizeof(mask));
    cout << endl;

    if (path[0] == 'q' || path[0] == 'Q')
        return;

	try
	{
		FTDirectory d;
		cpStr fn = d.getFirstEntry(path, mask);
		while (fn)
		{
			cout << fn << endl;
			fn = d.getNextEntry();
		}
	}
	catch (FTError* e)
	{
		cout << e->getText() << endl;
	}
}


class FTThreadDeadlock : public FTThreadBasic
{
public:
	FTThreadDeadlock(const char *name, FTMutex &m1, const char *m1name, FTMutex &m2, const char *m2name)
		: m_m1( m1 ),
		  m_m2( m2 )
	{
		m_name = name;
		m_m1_name = m1name;
		m_m2_name = m2name;
	}

	Dword threadProc(Void* arg)
	{
                cout << "Thread " << m_name << " is attempting to lock " << m_m1_name << endl;
        	FTMutexLock l1(m_m1, false);
		if ( l1.acquire(false) )
		{
                	cout << "Thread " << m_name << " has locked " << m_m1_name << ", sleeping for 1 second" << endl;
		}
		else
		{
                	cout << "Thread " << m_name << " was unable to lock " << m_m1_name << ", sleeping for 1 second" << endl;
			return 0;
		}

		sleep(1000);

                cout << "Thread " << m_name << " is attempting to lock " << m_m2_name << endl;
        	FTMutexLock l2(m_m2, false);
		if ( l2.acquire(false) )
		{
                	cout << "Thread " << m_name << " has locked " << m_m2_name << endl;
		}
		else
		{
                	cout << "Thread " << m_name << " was unable to lock " << m_m2_name << endl;
			return 0;
		}

		return 0;
	}

private:
    FTString m_name;
    FTString m_m1_name;
    FTString m_m2_name;
    FTMutex &m_m1;
    FTMutex &m_m2;
};

Void deadlock()
{
    FTMutex m1;
    FTMutex m2;
    FTThreadDeadlock t1( "THREAD_1", m1, "MUTEX_1", m2, "MUTEX_2" );
    FTThreadDeadlock t2( "THREAD_2", m2, "MUTEX_2", m1, "MUTEX_1" );

    t1.init(NULL);
    t2.init(NULL);

    t1.join();
    t2.join();
}

Void usage()
{
    const char* msg =
    "USAGE:  fttest [--help] [--file optionfile]\n";

    cout << msg;
}

Void printMenu()
{
    printf(
        "\n"
        "                          Foundation Tools Test Menu                           \n"
        "\n"
        "1.  Semaphore/thread cancellation              16. Circular buffer test         \n"
        "2.  DateTime object tests                      17. Directory test               \n"
        "3.  Public thread test (1 writer, 1 reader)    18. Hash test                    \n"
        "4.  Public thread test (1 writer, 4 readers)   19. Thread test (1 reader/writer)\n"
        "5.  Private thread test (1 writer, 4 readers)  20. Deadlock                     \n"
        "6.  Public queue test (reader)                 21. Thread Test (4 writers)      \n"
        "7.  Public queue test (writer)                 22. Mutex performance test       \n"
//        "8.  Elapsed timer                              17. OpenDBX tests                \n"
        "8.  Elapsed timer                                                               \n"
        "9.  Error handling                                                              \n"
        "10. Mutex text                                                                  \n"
        "11. Semaphore test                                                              \n"
        "12. Basic thread test                                                           \n"
        "13. Log test                                                                    \n"
        "14. Thread periodic timer test                                                  \n"
        "15. Thread one shot timer test                                                  \n"
        "\n"
    );
}

Void run()
{
    Int opt;
    Char selection[128];

    while (1)
    {
        printMenu();
        cout << "Selection : ";
        cin.getline(selection, sizeof(selection));
        cout << endl;

        if (selection[0] == 'q' || selection[0] == 'Q')
            break;

        opt = atoi(selection);

        try
        {
            switch (opt)
            {
                case 1: FTThread_cancel_wait();             break;
                //case 1:   FTSemaphoreNotice_test_writer();  break;
                //case 2:   FTSemaphoreNotice_test_reader();  break;
                case 2:   FTDateTime_test();                break;
                case 3:   FTThread_test();                  break;
                case 4:   FTThread_test2();                 break;
                case 5:   FTThread_test3();                 break;
                case 6:   FTQueuePublic_test(False);        break;
                case 7:   FTQueuePublic_test(True);         break;
                case 8:   FTTimer_test();                   break;
                case 9:   FTError_test();                   break;
                case 10:  FTMutex_test();                   break;
                case 11:  FTSemaphore_test();               break;
                case 12:  FTThreadBasic_test();             break;
                case 13:  FTLogger_test();                  break;
                case 14:  FTThreadTimerPeriodic_test();     break;
                case 15:  FTThreadTimerOneShot_test();      break;
                case 16:  FTCircularBuffer_test();          break;
//                case 17:  OpenDBX_test();                   break;
                case 17:  FTDirectory_test();               break;
                case 18:  FTHash_test();                    break;
                case 19:  FTThread_test4();                 break;
                case 20:  deadlock();                       break;
                case 21:  FTThread_test5();                 break;
                case 22:  FTMutex_test2();                  break;
                default:
                    cout << "Invalid Selection" << endl << endl;
                    break;
            }
        }
        catch (FTError *e)
        {
            cout << e->Name() << " - " << *e << endl;
            delete e;
        }
    }
}

#define BUFFER_SIZE	262144
int main(int argc, char* argv[])
{
	//FTString s;
	//FTTime t1, t2;
	//t1.Format(s, "%i", True);	cout << s << endl;
	//t2 = t1.add(1,0,0,0,0);
	//t1.Format(s, "%i", True);	cout << s << endl;
	//t2.Format(s, "%i", True);	cout << s << endl;
	//LongLong chk;
	//chk = t1.year() * 10000000000LL + t1.month() * 100000000LL + t1.day() * 1000000LL + t1.hour() * 10000LL + t1.minute() * 100LL + t1.second();
	//cout << chk << endl;

	//try
	//{
	//	FTBzip2 bz;

	//	Int block = 0;
	//	ULongLong tamt = 0;
	//	pChar buf = new Char[BUFFER_SIZE];
	//	memset(buf, 0x7f, BUFFER_SIZE);
	//	//bz.readOpen("C:\\Users\\bwaters\\Downloads\\cdr1.txt.20140730110101.bz2");
	//	//for (;;)
	//	//{
	//	//	Int amt = bz.read((pUChar)buf, BUFFER_SIZE);
	//	//	cout << "\r" << block++ << ":" << amt;
	//	//	tamt += (ULong)amt;
	//	//	if (amt < sizeof(buf))
	//	//		break;
	//	//}
	//	//cout << "\r" << block << ":" << tamt << endl;
	//	//bz.close();

	//	Int amt=1;
	//	block = 0;
	//	bz.readOpen("C:\\Users\\bwaters\\Downloads\\cdr1.txt.20140730110101.bz2");
	//	while (amt)
	//	{
	//		amt = bz.readLine(buf, BUFFER_SIZE);
	//		if (block % 1000 == 0)
	//			cout << "\r" << block << ":" << amt;
	//		block++;
	//	}
	//	cout << "\r" << block++ << ":" << amt << endl;
	//	bz.close();
	//}
	//catch (FTError* e)
	//{
	//	cout << e->getText() << endl;
	//}


    FTGetOpt cl(argc, argv);
    if( cl.search(2, "-h", "--help") )
    {
	    usage();
	    exit(0);
    }

    FTString optFile;
    optFile.format("%s.opt", argv[0]);
    FTGetOpt options(optFile);

    cl.init_multiple_occurrence();
    string arg;
    while ((arg = cl.follow("finished",2,"-f","--file")) != "finished")
    {
        FTGetOpt o(arg.c_str());
        options.absorb(o);
    }

    cout.imbue(std::locale(""));

    try
    {
        FoundationTools::Initialize(options);

        run();

        FoundationTools::UnInitialize();
    }
    catch (FTError *e)
    {
        cout << (cpStr)(*e) << endl;
    }

    return 0;
}

