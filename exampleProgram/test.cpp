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

#include "stdio.h"
#include <iostream>
#include <locale>
#include <memory.h>
#include "epc/epc.h"
#include "epc/einternal.h"
#include "epc/ebzip2.h"
#include "epc/esocket.h"

std::locale defaultLocale;
std::locale mylocale;

Void ECircularBuffer_test()
{
   pUChar pData = (pUChar) "[1234567890123456789012345678]";
   UChar buf[256];

   memset(buf, 0, sizeof(buf));

   cout << "ECircularBuffer_test() Start" << endl;

   ECircularBuffer cb(50);

   for (int i = 0; i < 20; i++)
   {
      cout << "Pass " << i << " writing 30 bytes...";
      cb.writeData(pData, 0, 30);
      cout << " reading 30 bytes...";
      cb.readData(buf, 0, 30);
      cout << buf << endl;
   }

   cout << "ECircularBuffer_test() Complete" << endl;
}

Void ETimer_test()
{
   cout << "ETimer_test() Start" << endl;

   ETimer t;

   t.Start();

   for (int i = 0; i < 1000000; i++)
      ;

   //t.Stop();

   epctime_t a = t.MicroSeconds();
   epctime_t b = t.MilliSeconds();

   cout << "MicroSeconds() = " << a << endl;
   cout << "MilliSeconds() = " << b << endl;

   cout << "ETimer_test() Complete" << endl;
}

Void EMutexPrivate_test()
{
   cout << "EMutexPrivate_test() Start" << endl;
   cout << "Creating private mutex" << endl;
   EMutexPrivate m;
   {
      EMutexLock l(m);
      cout << "Mutex locked.  Press [Enter] to continue ...";
      getc(stdin);
      cout << endl
           << "Destroying private mutex" << endl;
   }

   cout << "EMutex_test() Complete" << endl;
}

Void EMutexPublic_test()
{
   cout << "EMutexPublic_test() Start" << endl;
   {
      cout << "Creating 1st public mutex" << endl;
      EMutexPublic m;
      {
         cout << "1st Mutex ID = " << m.mutexId() << endl;
         EMutexLock l(m);
         cout << "1st Mutex locked.  Press [Enter] to continue ...";
         getc(stdin);
         cout << endl
              << "Destroying 1st public mutex" << endl;
      }
   }

   {
      cout << "Creating 2nd public mutex" << endl;
      EMutexPublic m;
      {
         cout << "2nd Mutex ID = " << m.mutexId() << endl;
         EMutexLock l(m);
         cout << "2nd Mutex locked.  Press [Enter] to continue ...";
         getc(stdin);
         cout << endl
              << "Destroying 2nd public mutex" << endl;
      }
   }

   cout << "EMutex_test() Complete" << endl;
}

Void ESemaphorePrivate_test()
{
   cout << "ESemaphorePrivate_test() Start" << endl;

   {
      cout << "Creating semaphore initial/max count of 5" << endl;
      ESemaphorePrivate s1(5);
      cout << "Decrementing";
      for (int i = 1; i <= 5; i++)
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

   cout << "ESemaphorePrivate_test() Complete" << endl;
}

Void ESemaphorePublic_test()
{
   cout << "ESemaphorePublic_test() Start" << endl;

   {
      cout << "Creating semaphore initial/max count of 5" << endl;
      ESemaphorePublic s1(5);
      cout << "Decrementing";
      for (int i = 1; i <= 5; i++)
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

   cout << "ESemaphorePublic_test() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define EM_MUTEXTEST (EM_USER + 1)

class EThreadMutexTest2 : public EThreadPrivate
{
public:
   EThreadMutexTest2(EMutexData &m1, int loop)
       : m_mutex1(m1),
         m_loop(loop)
   {
   }

   virtual Void onInit()
   {
      m_timer.Start();

      for (int i = 0; i < m_loop; i++)
      {
         EMutexLock l1(m_mutex1);
      }

      m_timer.Stop();

      quit();
   }

   virtual Void onQuit()
   {
      EThreadPrivate::onQuit();
      cout << "EThreadMutexTest2 elapsed time " << m_timer.MicroSeconds() - 1000000 << " microseconds" << endl;
   }

   DECLARE_MESSAGE_MAP()

private:
   EThreadMutexTest2();

   ETimer m_timer;
   EMutexData &m_mutex1;
   int m_loop;
};

BEGIN_MESSAGE_MAP(EThreadMutexTest2, EThreadPrivate)
END_MESSAGE_MAP()

Void EMutex_test2()
{
   cout << "EMutex_test2() Start" << endl;

   int loop = 1000000;
   EMutexPrivate m1;

   EThreadMutexTest2 t1(m1, loop);
   EThreadMutexTest2 t2(m1, loop);

   {
      EMutexLock l1(m1);

      t1.init(1, 1, NULL, 200000);
      t2.init(1, 2, NULL, 200000);

      cout << "Sleeping for 1 sercond ... ";
      EThreadBasic::sleep(1000);
   }

   cout << "test started" << endl;

   t1.join();
   t2.join();

   cout << "EMutex_test2() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class EThreadBasicCancelWaitTest : public EThreadBasic
{
public:
   Bool decrementResult;
   ESemaphorePrivate runningSemaphore;

   EThreadBasicCancelWaitTest()
   {
      decrementResult = False;
   }

   Dword threadProc(Void *arg)
   {
      int oldstate = 0;
      //		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
      //		cout << "PTHREAD_CANCEL_ENABLE=" << PTHREAD_CANCEL_ENABLE << " PTHREAD_CANCEL_DISABLE=" << PTHREAD_CANCEL_DISABLE << " old cancel state [" << oldstate << "]" << endl;
      //		pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);
      //		cout << ((oldstate==PTHREAD_CANCEL_DEFERRED) ? "cancellation was deferred" : "cancellation was NOT deferred") << endl;
      ESemaphorePrivate s;
      runningSemaphore.Increment();
      try
      {
         decrementResult = s.Decrement();
         cout << "after the decrement in the test thread" << endl;
      }
      catch (EError &e)
      {
         cout << e << endl;
      }
      catch (...)
      {
         cout << "an exception occurred" << endl;
         throw;
      }
      return 0;
   }
};

Void EThread_cancel_wait()
{
   cout << "EThread_cancel_wait() Start" << endl;

   EThreadBasicCancelWaitTest t;

   t.init((Void *)"this is the thread argument\n", true);
   //cout << "before resume" << endl;
   //t.resume();
   cout << "waiting for test thread to start" << endl;
   t.runningSemaphore.Decrement();
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

   cout << "EThread_cancel_wait() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void ESemaphore_test_cancel_wait()
{
   cout << "ESemaphore_test() Start" << endl;

   {
      cout << "Creating semaphore initial/max count of 5" << endl;
      ESemaphorePrivate s1(5);
      cout << "Decrementing";
      for (int i = 1; i <= 5; i++)
      {
         if (s1.Decrement() < 0)
            cout << "Error decrementing semaphore on pass " << i << endl;
         cout << ".";
      }
      cout << "Checking for decrement action at zero...";
      if (s1.Decrement(False))
         cout << " failed - Decrement returned true" << endl;
      else
         cout << "success" << endl;
   }

   cout << "ESemaphore_test() Complete" << endl;
}

Void EError_test()
{
   cout << "EError_test() Start" << endl;

   cout << "Creating error object" << endl;
   EError e1;
   e1.appendTextf("error object #%d", 1);
   cout << e1 << endl;

   cout << "EError_test() Complete" << endl;
}

class EThreadBasicTest : public EThreadBasic
{
public:
   EThreadBasicTest()
   {
      m_timetoquit = false;
   }

   Dword threadProc(Void *arg)
   {
      while (!m_timetoquit)
      {
         cout << "Inside the thread " << (cpStr)arg << endl;
         sleep(1000);
      }
      cout << "Exiting EThreadTest::threadProc()" << endl;
      return 0;
   }

   Void setTimeToQuit()
   {
      m_timetoquit = true;
   }

private:
   bool m_timetoquit;
};

Void EThreadBasic_test()
{
   cout << "EThread_test() Start" << endl;

   EThreadBasicTest t;

   t.init((Void *)"this is the thread argument\n", true);
   //cout << "before resume" << endl;
   //t.resume();
   cout << "before 5 second sleep sleep" << endl;
   t.sleep(5000);
   cout << "before setTimeToQuit()" << endl;
   t.setTimeToQuit();
   cout << "before 2nd 5 second sleep" << endl;
   t.sleep(5000);
   cout << "before join" << endl;
   t.join();

   cout << "EThread_test() Complete" << endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define EM_USER1 (EM_USER + 1)
#define EM_USER2 (EM_USER + 2)
#define EM_USER3 (EM_USER + 3)
#define EM_USER4 (EM_USER + 4)

class EThreadTest4 : public EThreadPrivate
{
public:
   EThreadTest4(int mm)
   {
      msg1snt = 0;
      msg1rcv = 0;
      msgmax = mm;
   }

   Void userFunc1(EThreadMessage &msg)
   {
      msg1rcv++;
      if (msg1rcv == msgmax)
         quit();
      else if (msg1snt < msgmax)
         sendMessage(EM_USER1, 0, (Long)msg1snt++);
   }

   Void onInit()
   {
      printf("Inside onInit()\n");

      te.Start();

      //sendMessage(EM_USER1, 0, (Long)msg1snt++);
      sendMessage(EM_USER1, 0, (Long)msg1snt++);
   }

   virtual Void onQuit()
   {
      te.Stop();

      EThreadPrivate::onQuit();
      cout << "EThreadTest4 sent     " << msg1snt << " EM_USER1 messages" << endl;
      cout << "EThreadTest4 received " << msg1rcv << " EM_USER1 messages" << endl;
   }

   Void onSuspend()
   {
      cout << "Inside onSuspend()" << endl;
      EThreadPrivate::onSuspend();
   }

   ETimer &getTimer() { return te; }

   DECLARE_MESSAGE_MAP()

private:
   EThreadTest4();

   int msg1snt;
   int msg1rcv;
   int msgmax;
   ETimer te;
};

class EThreadTest : public EThreadPublic
{
public:
   EThreadTest(EMutexPublic &mutex)
      : mtx(mutex)
   {
      msg1cnt = 0;
      msg2cnt = 0;
   }

   Void userFunc1(EThreadMessage &msg)
   {
      msg1cnt++;
   }

   Void userFunc2(EThreadMessage &msg)
   {
      msg2cnt++;
   }

   //	Void onInit()
   //	{
   //		printf("Inside onInit()\n");
   //	}

   virtual Void onQuit()
   {
      EThreadPublic::onQuit();
      EMutexLock l(mtx);
      cout << "EThreadTest received " << msg1cnt << " EM_USER1 messages" << endl;
      cout << "EThreadTest received " << msg2cnt << " EM_USER2 messages" << endl;
   }

   Void onSuspend()
   {
      cout << "Inside onSuspend()" << endl;
      EThreadPublic::onSuspend();
   }

   DECLARE_MESSAGE_MAP()

private:
   EMutexPublic &mtx;
   int msg1cnt;
   int msg2cnt;

   EThreadTest();
};

class EThreadTest2 : public EThreadTest
{
public:
   EThreadTest2(EMutexPublic &mutex)
       : EThreadTest(mutex), mtx(mutex)
   {
      msg3cnt = 0;
      msg4cnt = 0;
   }

   Void userFunc3(EThreadMessage &msg)
   {
      msg3cnt++;
   }

   Void userFunc4(EThreadMessage &msg)
   {
      msg4cnt++;
   }

   virtual Void onQuit()
   {
      EThreadTest::onQuit();
      EMutexLock l(mtx);
      cout << "EThreadTest2 received " << msg3cnt << " EM_USER3 messages" << endl;
      cout << "EThreadTest2 received " << msg4cnt << " EM_USER4 messages" << endl;
   }

   DECLARE_MESSAGE_MAP()

private:
   EMutexPublic &mtx;
   int msg3cnt;
   int msg4cnt;

   EThreadTest2();
};

class EThreadTest3 : public EThreadPrivate
{
public:
   EThreadTest3()
   {
      msg1cnt = 0;
      msg2cnt = 0;
      msg3cnt = 0;
      msg4cnt = 0;
   }

   Void userFunc1(EThreadMessage &msg)
   {
      msg1cnt++;
   }

   Void userFunc2(EThreadMessage &msg)
   {
      msg2cnt++;
   }

   Void userFunc3(EThreadMessage &msg)
   {
      msg3cnt++;
   }

   Void userFunc4(EThreadMessage &msg)
   {
      msg4cnt++;
   }

   virtual Void onQuit()
   {
      EThreadPrivate::onQuit();
      cout << "EThreadTest3 received " << msg1cnt << " EM_USER1 messages" << endl;
      cout << "EThreadTest3 received " << msg2cnt << " EM_USER2 messages" << endl;
      cout << "EThreadTest3 received " << msg3cnt << " EM_USER3 messages" << endl;
      cout << "EThreadTest3 received " << msg4cnt << " EM_USER4 messages" << endl;
   }

   DECLARE_MESSAGE_MAP()

private:
   int msg1cnt;
   int msg2cnt;
   int msg3cnt;
   int msg4cnt;
};

BEGIN_MESSAGE_MAP(EThreadTest, EThreadPublic)
   ON_MESSAGE(EM_USER1, EThreadTest::userFunc1)
   ON_MESSAGE(EM_USER2, EThreadTest::userFunc2)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(EThreadTest2, EThreadTest)
   ON_MESSAGE(EM_USER3, EThreadTest2::userFunc3)
   ON_MESSAGE(EM_USER4, EThreadTest2::userFunc4)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(EThreadTest3, EThreadPrivate)
   ON_MESSAGE(EM_USER1, EThreadTest3::userFunc1)
   ON_MESSAGE(EM_USER2, EThreadTest3::userFunc2)
   ON_MESSAGE(EM_USER3, EThreadTest3::userFunc3)
   ON_MESSAGE(EM_USER4, EThreadTest3::userFunc4)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(EThreadTest4, EThreadPrivate)
   ON_MESSAGE(EM_USER1, EThreadTest4::userFunc1)
END_MESSAGE_MAP()

template <class T>
std::string numberFormatWithCommas(T value, int precision = 6)
{
   struct Numpunct : public std::numpunct<char>
   {
   protected:
      virtual char do_thousands_sep() const { return ','; }
      virtual std::string do_grouping() const { return "\03"; }
   };
   std::stringstream ss;
   ss.imbue({std::locale(), new Numpunct});
   ss << std::setprecision(precision) << std::fixed << value;
   return ss.str();
}

Void EThread_test4()
{
   int maxmsg = 100000000;

   EThreadTest4 t1(maxmsg);

   t1.init(1, 1, NULL, 200000);

   EThreadBasic::yield();

   t1.join();

   double persec = ((double)maxmsg) / (((double)t1.getTimer().MicroSeconds()) / 1000000);
   std::string s = numberFormatWithCommas<double>(persec);
   cout << "Processed " << maxmsg << " messages in " << ((double)t1.getTimer().MicroSeconds()) / 1000000 << " seconds (" << s << " per second)" << endl;
   //" seconds (" << (((double)maxmsg) / (((double)te.MicroSeconds())/1000000)) << " per second)" << endl;
}

class EThreadTest5rcv : public EThreadPrivate
{
public:
   EThreadTest5rcv(int senders)
   {
      msg1rcv = 0;
      msg2rcv = 0;
      msg3rcv = 0;
      msgsenders = senders;
      maxpending = 0;
   }

   Void rcv1(EThreadMessage &msg)
   {
      msg1rcv++;

      if (msg1rcv == 1)
         te.Start();
   }

   Void rcv2(EThreadMessage &msg)
   {
      UInt pending = getMsgSemaphore().currCount();
      if (pending > maxpending)
         maxpending = pending;

      msg2rcv++;

      if (msg2rcv % 1000000 == 0)
         cout << "\rEThreadTest5 received " << msg2rcv << " max pending " << maxpending << flush;
   }

   Void rcv3(EThreadMessage &msg)
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
      EThreadPrivate::onQuit();
      cout << "\rEThreadTest5rcv received " << msg2rcv << " EM_USER2 messages with max pending of " << maxpending << endl
           << flush;
   }

   ETimer &getTimer() { return te; }

   DECLARE_MESSAGE_MAP()

private:
   EThreadTest5rcv();

   int msg1rcv;
   int msg2rcv;
   int msg3rcv;
   int msgsenders;
   Long maxpending;
   ETimer te;
};

BEGIN_MESSAGE_MAP(EThreadTest5rcv, EThreadPrivate)
   ON_MESSAGE(EM_USER1, EThreadTest5rcv::rcv1)
   ON_MESSAGE(EM_USER2, EThreadTest5rcv::rcv2)
   ON_MESSAGE(EM_USER3, EThreadTest5rcv::rcv3)
END_MESSAGE_MAP()

class EThreadTest5snd : public EThreadBasic
{
public:
   EThreadTest5snd(EThreadTest5rcv *receiver, int cnt)
   {
      msgreceiver = receiver;
      msgcnt = cnt;
      msgsnt = 0;
   }

   Dword threadProc(Void *arg)
   {
      msgreceiver->sendMessage(EM_USER1);

      for (msgsnt = 0; msgsnt < msgcnt; msgsnt++)
      {
         if (!msgreceiver->sendMessage(EM_USER2))
            cout << endl
                 << "sendMessage returned FALSE for message # " << msgsnt << endl
                 << flush;
      }

      msgreceiver->sendMessage(EM_USER3);

      return 0;
   }

   int getMessagesSent() { return msgsnt; }

private:
   EThreadTest5rcv *msgreceiver;
   int msgcnt;
   int msgsnt;
};

Void EThread_test5()
{
   static Int nSenders = 1;
   static Int nMessages = 1000000;
   Char buffer[128];
   EThreadTest5snd **sendthrds = NULL;

   cout << "Enter number of sending threads [" << nSenders << "]: ";
   cin.getline(buffer, sizeof(buffer));
   nSenders = buffer[0] ? atoi(buffer) : nSenders;

   cout << "Enter message count [" << nMessages << "]: ";
   cin.getline(buffer, sizeof(buffer));
   nMessages = buffer[0] ? atoi(buffer) : nMessages;

   EThreadTest5rcv t1(nSenders);

   t1.init(1, 1, NULL, 200000);

   sendthrds = new EThreadTest5snd *[nSenders];

   for (int i = 0; i < nSenders; i++)
   {
      sendthrds[i] = new EThreadTest5snd(&t1, nMessages);
      sendthrds[i]->init(NULL);
   }

   EThreadBasic::yield();

   t1.join();
   for (int i = 0; i < nSenders; i++)
   {
      sendthrds[i]->join();
      delete sendthrds[i];
   }
   delete[] sendthrds;

   double persec = ((double)(nSenders * nMessages)) / (((double)t1.getTimer().MicroSeconds()) / 1000000);
   std::string s = numberFormatWithCommas<double>(persec);
   cout << "Processed " << (nSenders * nMessages) << " messages in " << ((double)t1.getTimer().MicroSeconds()) / 1000000 << " seconds (" << s << " per second)" << endl;
}

Void EThread_test()
{
   EMutexPublic mtx;
   EThreadTest2 t1(mtx);

   t1.init(1, 1, NULL, 200000);
   EThreadBasic::yield();

   ETimer te;

   te.Start();
   int i;

   for (i = 0; i < 4000000; i++)
   {
      t1.sendMessage(EM_USER1, 0, (Long)i);
      t1.sendMessage(EM_USER2, 0, (Long)i);
      t1.sendMessage(EM_USER3, 0, (Long)i);
      t1.sendMessage(EM_USER4, 0, (Long)i);
      //        EThreadBasic::yield();
   }

   t1.quit();
   t1.join();

   i *= 4;
   cout << "Processed " << i << " messages in " << ((double)te.MicroSeconds()) / 1000000 << " seconds (" << ((double)i) / (((double)te.MicroSeconds()) / 1000000) << " per second)" << endl;
}

Void EThread_test2()
{
   EMutexPublic mtx;
   EThreadTest2 t1(mtx);
   EThreadTest2 t2(mtx);
   EThreadTest2 t3(mtx);
   EThreadTest2 t4(mtx);

   t1.init(1, 1, NULL, 200000);
   t2.init(1, 2, NULL, 200000);
   t3.init(1, 3, NULL, 200000);
   t4.init(1, 4, NULL, 200000);
   t1.yield();

   ETimer te;

   te.Start();
   int i;

   for (i = 0; i < 1000000; i++)
   {
      for (int i = 0; i < 4; i++)
      {
         try
         {
            switch (i)
            {
            case 0:
               t1.sendMessage(EM_USER1, 0, (Long)i);
               t2.sendMessage(EM_USER1, 0, (Long)i);
               t3.sendMessage(EM_USER1, 0, (Long)i);
               t4.sendMessage(EM_USER1, 0, (Long)i);
               break;
            case 1:
               t1.sendMessage(EM_USER2, 0, (Long)i);
               t2.sendMessage(EM_USER2, 0, (Long)i);
               t3.sendMessage(EM_USER2, 0, (Long)i);
               t4.sendMessage(EM_USER2, 0, (Long)i);
               break;
            case 2:
               t1.sendMessage(EM_USER3, 0, (Long)i);
               t2.sendMessage(EM_USER3, 0, (Long)i);
               t3.sendMessage(EM_USER3, 0, (Long)i);
               t4.sendMessage(EM_USER3, 0, (Long)i);
               break;
            case 3:
               t1.sendMessage(EM_USER4, 0, (Long)i);
               t2.sendMessage(EM_USER4, 0, (Long)i);
               t3.sendMessage(EM_USER4, 0, (Long)i);
               t4.sendMessage(EM_USER4, 0, (Long)i);
               break;
            }
         }
         catch (EError &e)
         {
            cout << "Processing exception " << e.Name() << " - " << e << endl;
            throw;
         }
      }
      //        EThreadBasic::yield();
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
   cout << "Processed " << i << " messages in " << ((double)te.MicroSeconds()) / 1000000 << " seconds (" << ((double)i) / (((double)te.MicroSeconds()) / 1000000) << " per second)" << endl;
}

Void EThread_test3()
{
   EThreadTest3 t1;
   EThreadTest3 t2;
   EThreadTest3 t3;
   EThreadTest3 t4;

   t1.init(1, 1, NULL, 200000);
   t2.init(1, 2, NULL, 200000);
   t3.init(1, 3, NULL, 200000);
   t4.init(1, 4, NULL, 200000);
   t1.yield();

   ETimer te;

   te.Start();
   int i;

   for (i = 0; i < 1000000; i++)
   {
      for (int i = 0; i < 4; i++)
      {
         try
         {
            switch (i)
            {
            case 0:
               t1.sendMessage(EM_USER1, 0, (Long)i);
               t2.sendMessage(EM_USER1, 0, (Long)i);
               t3.sendMessage(EM_USER1, 0, (Long)i);
               t4.sendMessage(EM_USER1, 0, (Long)i);
               break;
            case 1:
               t1.sendMessage(EM_USER2, 0, (Long)i);
               t2.sendMessage(EM_USER2, 0, (Long)i);
               t3.sendMessage(EM_USER2, 0, (Long)i);
               t4.sendMessage(EM_USER2, 0, (Long)i);
               break;
            case 2:
               t1.sendMessage(EM_USER3, 0, (Long)i);
               t2.sendMessage(EM_USER3, 0, (Long)i);
               t3.sendMessage(EM_USER3, 0, (Long)i);
               t4.sendMessage(EM_USER3, 0, (Long)i);
               break;
            case 3:
               t1.sendMessage(EM_USER4, 0, (Long)i);
               t2.sendMessage(EM_USER4, 0, (Long)i);
               t3.sendMessage(EM_USER4, 0, (Long)i);
               t4.sendMessage(EM_USER4, 0, (Long)i);
               break;
            }
         }
         catch (EError &e)
         {
            cout << "Processing exception " << e.Name() << " - " << e << endl;
            throw;
         }
      }
      //        EThreadBasic::yield();
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
   cout << "Processed " << i << " messages in " << ((double)te.MicroSeconds()) / 1000000 << " seconds (" << ((double)i) / (((double)te.MicroSeconds()) / 1000000) << " per second)" << endl;
}

void ESharedMemory_test()
{
   ESharedMemory m("test", 1, 1024 * 1024);
}

class testmessage : public EQueueMessage
{
public:
   testmessage()
   {
      epc_strcpy_s(m_data, sizeof(m_data),
                  "This is a shared queue test. Four score and 7 years ago, our fathers");
   }

   ~testmessage() {}

   virtual Void getLength(ULong &length)
   {
      EQueueMessage::getLength(length);
      elementLength(m_data, length);
   }
   virtual Void serialize(pVoid pBuffer, ULong &nOffset)
   {
      EQueueMessage::serialize(pBuffer, nOffset);
      pack(m_data, pBuffer, nOffset);
   }
   virtual Void unserialize(pVoid pBuffer, ULong &nOffset)
   {
      EQueueMessage::unserialize(pBuffer, nOffset);
      unpack(m_data, pBuffer, nOffset);
   }

   Char m_data[128];
};

class TestPublicQueue : public EQueuePublic
{
public:
   EQueueMessage *allocMessage(Long msgType)
   {
      return &msg;
   }

   testmessage &getMessage()
   {
      return msg;
   }

private:
   testmessage msg;
};

void EQueuePublic_test(Bool bWriter)
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
      q.init(nQueueId, bWriter ? EQueueBase::WriteOnly : EQueueBase::ReadOnly);
   }
   catch (EError &e)
   {
      cout << e.Name() << " - " << e << endl;
      return;
   }

   testmessage msg;
   ETimer te;

   Int cnt = 0;

   if (bWriter)
   {
      // writer
      for (cnt = 0; cnt < nMsgCnt; cnt++)
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

   cout << "Processed " << cnt << " messages in " << ((double)te.MicroSeconds()) / 1000000
        << " seconds (" << ((double)cnt) / (((double)te.MicroSeconds()) / 1000000) << " per second)" << endl;
}

#if 0
#define LOG_MASK_1 0x0000000000000001
#define LOG_MASK_2 0x0000000000000002
#define LOG_MASK_3 0x0000000000000004
#define LOG_MASK_4 0x0000000000000008

Void ELogger_test()
{
    cout << "ELogger_test start" << endl;

    ELogger::setGroupMask(1, 0);
    ELogger::setGroupMask(2, 0);

    ELogger::logError(1, LOG_MASK_1, "ELogger_test", "This line should not be logged, because it is not enabled.");
    ELogger::enableGroupMask(1, LOG_MASK_1);
    ELogger::logError(1, LOG_MASK_1, "ELogger_test", "This line should be logged.");

    cout << "Testing single file log rollover." << endl;
    EString s;
    Int i;
    for (i = 0; i<19; i++)
        ELogger::logInfo(1, LOG_MASK_1, "ELogger_test", "LOG_MASK_1 Line %d", i + 1);
    cout << "  Review log file 1.  There should be 10 lines in the log file, numbers 10-19." << endl;

    ELogger::disableGroupMask(1, LOG_MASK_1);
    ELogger::logInfo(1, LOG_MASK_1, "ELogger_test", "This line should not be logged because it has been disabled.");

    cout << "Testing log rollover with 2 files." << endl;
    ELogger::enableGroupMask(2, LOG_MASK_2);
    for (i = 0; i<20; i++)
        ELogger::logInfo(2, LOG_MASK_2, "ELogger_test", "LOG_MASK_2 Line %d", i + 1);
    cout << "  Review log file 2.  There should be 2 segments with 10 lines each." << endl;

    cout << "Testing syslog." << endl;
    ELogger::enableGroupMask(3, LOG_MASK_2);
    ELogger::logInfo(3, LOG_MASK_2, "ELogger_test", "Info message");
    ELogger::logWarning(3, LOG_MASK_2, "ELogger_test", "Warning message");
    ELogger::logError(3, LOG_MASK_2, "ELogger_test", "Error message");
    cout << "  Review log file 3.  There should be 2 segments with 10 lines each." << endl;

    cout << "ELogger_test complete" << endl;
}
#endif

Void EDateTime_test()
{
   ETime t;

   t.Now();

   EString s;
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

class EThreadTimerTest : public EThreadPrivate
{
public:
   EThreadTimerTest()
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
         m_timer1.setInterval(5000);
         m_timer1.setOneShot(m_oneshot);
         initTimer(m_timer1);
         m_elapsed.Start();
         m_timer1.start();
      }
   }

   Void onTimer(EThreadBase::Timer *pTimer)
   {
      cout << m_elapsed.MilliSeconds(True) << " milliseconds has elapsed." << endl;
      if (pTimer->getId() == m_timer1.getId())
      {
         m_cnt++;
         if (m_cnt == 5)
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
   EThreadBase::Timer m_timer1;
   EThreadBase::Timer m_timer2;
   EThreadBase::Timer m_timer3;
   ETimer m_elapsed;
};

BEGIN_MESSAGE_MAP(EThreadTimerTest, EThreadPrivate)
END_MESSAGE_MAP()

Void EThreadTimerPeriodic_test()
{
   EThreadTimerTest t;
   t.setOneShot(False);
   t.init(1, 1, NULL, 2000);
   t.join();
}

Void EThreadTimerOneShot_test()
{
   EThreadTimerTest t;
   t.setOneShot(True);
   t.init(1, 1, NULL, 2000);
   t.join();
}

class EThreadTestSuspendResume : public EThreadPrivate
{
public:
   EThreadTestSuspendResume()
   {
      m_timer.Start();
   }

   Void userFunc1(EThreadMessage &msg)
   {
      cout << "received EM_USER1 - elapsed time " << m_timer.MicroSeconds() << "us" << endl;
      m_timer.Start();
   }

   Void onInit()
   {
      cout << "received EM_INIT - elapsed time " << m_timer.MicroSeconds() << "us" << endl;
      m_timer.Start();
   }

   Void onSuspend()
   {
      cout << "received EM_SUSPEND - elapsed time " << m_timer.MicroSeconds() << "us" << endl;
      m_timer.Start();
   }

   Void onQuit()
   {
      EThreadPrivate::onQuit();
      cout << "received EM_QUIT - elapsed time " << m_timer.MicroSeconds() << "us" << endl;
      m_timer.Start();
   }

   DECLARE_MESSAGE_MAP()

private:
   ETimer m_timer;
};

BEGIN_MESSAGE_MAP(EThreadTestSuspendResume, EThreadPrivate)
   ON_MESSAGE(EM_USER1, EThreadTestSuspendResume::userFunc1)
END_MESSAGE_MAP()

Void EThreadSuspendResume_test()
{
   cout << "EThreadSuspendResume_test - start" << endl;

   EThreadTestSuspendResume t;

   t.init(1, 1, NULL, 2000);
   EThreadBasic::sleep(1000);

   cout << "start sending EM_USER1 messages" << endl;
   for (Int i=0; i<5; i++)
   {
      t.sendMessage(EM_USER1);
      EThreadBasic::sleep(1000);
   }

   for (Int i=0; i<5; i++)
   {
      Int ms = 3000;
      if (!i)
      {
         cout << "suspending the thread for " << ms/1000 << "seconds" << endl;
         t.suspend();
         cout << "sending EM_USER1 while thread is suspended " << endl;
      }

      t.sendMessage(EM_USER1);

      if (!i)
      {
         EThreadBasic::sleep(ms);
         cout << "resuming the thread" << endl;
         t.resume();
      }

      EThreadBasic::sleep(1000);
   }

   t.quit();
   t.join();
   cout << "EThreadSuspendResume_test - complete" << endl;
}

Void EHash_test()
{
   cout << "Ehash_test start" << endl;

   ULong hash;
   EString s;

   // EString short
   {
      EString s;
      s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456";
      hash = EHash::getHash(s);
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for EString [" << s << "]" << endl;
   }

   {
      EString s;
      s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123457";
      hash = EHash::getHash(s);
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for EString [" << s << "]" << endl;
   }

   // EString long
   {
      EString s;
      s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
      hash = EHash::getHash(s);
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for EString [" << s << "]" << endl;
   }

   {
      EString s;
      s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456780";
      hash = EHash::getHash(s);
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for EString [" << s << "]" << endl;
   }

   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////

   {
      cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456";
      hash = EHash::getHash(s, strlen(s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpChar [" << s << "]" << endl;
   }

   {
      cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123457";
      hash = EHash::getHash(s, strlen(s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpChar [" << s << "]" << endl;
   }

   {
      cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
      hash = EHash::getHash(s, strlen(s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpChar [" << s << "]" << endl;
   }

   {
      cpChar s = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456780";
      hash = EHash::getHash(s, strlen(s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpChar [" << s << "]" << endl;
   }

   ////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////

   {
      cpUChar s = (cpUChar) "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456";
      hash = EHash::getHash(s, strlen((cpChar)s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpUChar [" << s << "]" << endl;
   }

   {
      cpUChar s = (cpUChar) "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123457";
      hash = EHash::getHash(s, strlen((cpChar)s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpUChar [" << s << "]" << endl;
   }

   {
      cpUChar s = (cpUChar) "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
      hash = EHash::getHash(s, strlen((cpChar)s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpUChar [" << s << "]" << endl;
   }

   {
      cpUChar s = (cpUChar) "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456780";
      hash = EHash::getHash(s, strlen((cpChar)s));
      cout << std::hex << std::uppercase << hash << std::nouppercase << std::dec << " for cpUChar [" << s << "]" << endl;
   }

   cout << "Ehash_test complete" << endl;
}

Void EDirectory_test()
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
      EDirectory d;
      cpStr fn = d.getFirstEntry(path, mask);
      while (fn)
      {
         cout << fn << endl;
         fn = d.getNextEntry();
      }
   }
   catch (EError &e)
   {
      cout << e.getText() << endl;
   }
}

class EThreadDeadlock : public EThreadBasic
{
public:
   EThreadDeadlock(const char *name, EMutexData &m1, const char *m1name, EMutexData &m2, const char *m2name)
       : m_m1(m1),
         m_m2(m2)
   {
      m_name = name;
      m_m1_name = m1name;
      m_m2_name = m2name;
   }

   Dword threadProc(Void *arg)
   {
      cout << "Thread " << m_name << " is attempting to lock " << m_m1_name << endl;
      EMutexLock l1(m_m1, false);
      if (l1.acquire(false))
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
      EMutexLock l2(m_m2, false);
      if (l2.acquire(false))
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
   EString m_name;
   EString m_m1_name;
   EString m_m2_name;
   EMutexData &m_m1;
   EMutexData &m_m2;
};

Void deadlock()
{
   EMutexPrivate m1;
   EMutexPrivate m2;
   EThreadDeadlock t1("THREAD_1", m1, "MUTEX_1", m2, "MUTEX_2");
   EThreadDeadlock t2("THREAD_2", m2, "MUTEX_2", m1, "MUTEX_1");

   t1.init(NULL);
   t2.init(NULL);

   t1.join();
   t2.join();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class Listener;
class Talker;

class Worker : public ESocketThread
{
public:
	Worker() { m_listen = False; m_port = 0; m_cnt = 0; m_talker = NULL; }

	Void onInit();
	Void onQuit();
	Void onClose();

	Void errorHandler(EError &err, ESocket* psocket);

	Talker* createTalker();

	Void setListen(Bool v) { m_listen = v; }
	Bool getListen() { return m_listen; }

   Void setCount(Int cnt) { m_cnt = cnt; }
   Int getCnt() { return m_cnt; }

   Void setPort(UShort port) { m_port = port; }
   UShort getPort() { return m_port; }

private:
	Bool		   m_listen;
   UShort      m_port;
   Int         m_cnt;
	Listener*	m_listener;
	Talker*		m_talker;

};

class Listener : public ESocketListen
{
public:
	Listener(Worker* pthread) : ESocketListen(pthread, 1048576) {}
	~Listener() {}

    ESocketConverse* createSocket(ESocketThread* pthread);

    Void onClose();
    Void onError();
};

class Talker : public ESocketConverse
{
public:
	Talker(Worker* pthread) : ESocketConverse(pthread, 1048576) {}
	~Talker() {}

	Void onConnect();
	Bool onReceive();
	Void onClose();

private:
	Talker();
};

///////////////////////////////////////////////////////////////////////////////////

ESocketConverse* Listener::createSocket(ESocketThread* pthread)
{

	return ((Worker*)pthread)->createTalker();
}

Void Listener::onClose()
{
   std::cout << "listening socket closed" << std::endl << std::flush;
}

Void Listener::onError()
{
   std::cout << "socket error " << getError() << " occurred on listening socket during select" << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////////////

Void Talker::onConnect()
{
	ESocketConverse::onConnect();

   std::cout << "socket connected" << std::endl << std::flush;

	Int val = 1;
	write((pUChar)&val, sizeof(val));
}

Bool Talker::onReceive()
{
	UChar buffer[1024];
   Int *pval = (Int*)buffer;

	while (true)
	{
		if (bytesPending() < 4 || read(buffer,4) != 4)
			break;
		if ((*pval) % 10000 == 0)
         std::cout << "\r" << *pval << std::flush;

      if (*pval != -1)
      {
         *pval = (((Worker*)getThread())->getCnt() > 0 && *pval >= ((Worker*)getThread())->getCnt()) ? -1 : (*pval + 1);
         write(buffer, 4);
      }

      if (*pval == -1)
      {
         if (((Worker*)getThread())->getListen())
            disconnect();
         break;
      }
	}

   return True;
}

Void Talker::onClose()
{
   std::cout << std::endl << "socket closed" << std::endl << std::flush;
	((Worker*)getThread())->onClose();
}

///////////////////////////////////////////////////////////////////////////////////

Talker* Worker::createTalker()
{
	return m_talker = new Talker(this);
}

Void Worker::onInit()
{
   UShort port = 12345;
	if (getListen())
	{
		m_listener = new Listener(this);
		m_listener->listen(port, 10);
      std::cout.imbue(defaultLocale);
      std::cout << "waiting for client to attach on port " << port << std::endl << std::flush;
      std::cout.imbue(mylocale);
	}
	else
	{
      std::cout.imbue(defaultLocale);
      std::cout << "connecting to server on port " << port << std::endl << std::flush;
      std::cout.imbue(mylocale);
		createTalker()->connect("127.0.0.1", 12345);
	}

   std::cout << std::endl << std::flush;
}

Void Worker::onQuit()
{
}

Void Worker::onClose()
{
	if (m_talker)
	{
		Talker* t = m_talker;
		m_talker = NULL;

		//t->close();
		delete t;
		quit();
	}
}

Void Worker::errorHandler(EError &err, ESocket* psocket)
{
   //std::cout << "Socket exception - " << err << std::endl << std::flush;
}

///////////////////////////////////////////////////////////////////////////////////

Void sockettest(Bool server)
{
   static Int messages = 1000000;
   static UShort port = 12345;
   Worker* pWorker = new Worker();
   Char buffer[128];

   cout.imbue(defaultLocale);
   cout << "Enter the port number for the connection [" << port << "]: ";
   cout.imbue(mylocale);
   cin.getline(buffer, sizeof(buffer));
   port = buffer[0] ? (UShort)std::stoi(buffer) : port;
   pWorker->setPort(port);

   if (server)
   {
      cout << "Enter number of messages to exchange with the client [" << messages << "]: ";
      cin.getline(buffer, sizeof(buffer));
      messages = buffer[0] ? std::stoi(buffer) : messages;
      pWorker->setCount(messages);
   }

   pWorker->setListen(server);

   pWorker->init(1, 1, NULL);
   pWorker->join();
   delete pWorker;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

Void usage()
{
   const char *msg =
       "USAGE:  epctest [--help] [--file optionfile]\n";

   cout << msg;
}

Void printMenu()
{
   printf(
       "\n"
       "                       Enhanced Packet Core Tools Test Menu                     \n"
       "\n"
       "1.  Semaphore/thread cancellation              14. Basic thread test            \n"
       "2.  DateTime object tests                      15. Thread suspend/resume        \n"
       "3.  Public thread test (1 writer, 1 reader)    16. Thread periodic timer test   \n"
       "4.  Public thread test (1 writer, 4 readers)   17. Thread one shot timer test   \n"
       "5.  Private thread test (1 writer, 4 readers)  18. Circular buffer test         \n"
       "6.  Public queue test (reader)                 19. Directory test               \n"
       "7.  Public queue test (writer)                 20. Hash test                    \n"
       "8.  Elapsed timer                              21. Thread test (1 reader/writer)\n"
       "9.  Error handling                             22. Deadlock                     \n"
       "10. Private Mutex text                         23. Thread Test (4 writers)      \n"
       "11. Public Mutex text                          24. Mutex performance test       \n"
       "12. Private Semaphore test                     25. Socket server                \n"
       "13. Public Semaphore test                      26. Socket client                \n"
       "\n");
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
         case 1:
            EThread_cancel_wait();
            break;
         case 2:
            EDateTime_test();
            break;
         case 3:
            EThread_test();
            break;
         case 4:
            EThread_test2();
            break;
         case 5:
            EThread_test3();
            break;
         case 6:
            EQueuePublic_test(False);
            break;
         case 7:
            EQueuePublic_test(True);
            break;
         case 8:
            ETimer_test();
            break;
         case 9:
            EError_test();
            break;
         case 10:
            EMutexPrivate_test();
            break;
         case 11:
            EMutexPublic_test();
            break;
         case 12:
            ESemaphorePrivate_test();
            break;
         case 13:
            ESemaphorePublic_test();
            break;
         case 14:
            EThreadBasic_test();
            break;
         case 15:
            EThreadSuspendResume_test();
            break;
         case 16:
            EThreadTimerPeriodic_test();
            break;
         case 17:
            EThreadTimerOneShot_test();
            break;
         case 18:
            ECircularBuffer_test();
            break;
            //                case 17:  OpenDBX_test();                   break;
         case 19:
            EDirectory_test();
            break;
         case 20:
            EHash_test();
            break;
         case 21:
            EThread_test4();
            break;
         case 22:
            deadlock();
            break;
         case 23:
            EThread_test5();
            break;
         case 24:
            EMutex_test2();
            break;
         case 25:
            sockettest(True);
            break;
         case 26:
            sockettest(False);
            break;
         default:
            cout << "Invalid Selection" << endl
                 << endl;
            break;
         }
      }
      catch (EError &e)
      {
         cout << e.Name() << " - " << e << endl;
      }
   }
}

#define BUFFER_SIZE 262144
int main(int argc, char *argv[])
{
   //EString s;
   //ETime t1, t2;
   //t1.Format(s, "%i", True);	cout << s << endl;
   //t2 = t1.add(1,0,0,0,0);
   //t1.Format(s, "%i", True);	cout << s << endl;
   //t2.Format(s, "%i", True);	cout << s << endl;
   //LongLong chk;
   //chk = t1.year() * 10000000000LL + t1.month() * 100000000LL + t1.day() * 1000000LL + t1.hour() * 10000LL + t1.minute() * 100LL + t1.second();
   //cout << chk << endl;

   //try
   //{
   //	EBzip2 bz;

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
   //catch (EError  e)
   //{
   //	cout << e->getText() << endl;
   //}

   //{
   //   EMutexData d;
   //   EMutexDataPublic dp;
   //
   //   cout << "sizeof EMutexPrivate = " << sizeof(EMutexPrivate) << endl;
   //   cout << "sizeof EMutexPublic = " << sizeof(EMutexPublic) << endl;
   //   cout << "sizeof EMutexData = " << sizeof(EMutexData) << endl;
   //   cout << "sizeof EMutexData.m_initialized = " << sizeof(d.initialized()) << endl;
   //   cout << "sizeof EMutexData.m_mutex = " << sizeof(d.mutex()) << endl;
   //   cout << "sizeof EMutexDataPublic = " << sizeof(dp) << endl;
   //   cout << "sizeof EMutexDataPublic.m_initialized = " << sizeof(dp.initialized()) << endl;
   //   cout << "sizeof EMutexDataPublic.m_mutex = " << sizeof(dp.mutex()) << endl;
   //   cout << "sizeof EMutexDataPublic.m_nextIndex = " << sizeof(dp.nextIndex()) << endl;
   //   cout << "sizeof EMutexDataPublic.m_mutexId = " << sizeof(dp.mutexId()) << endl;
   //}

   EGetOpt::Option options[] = {
       {"-h", "--help", EGetOpt::no_argument, EGetOpt::dtNone},
       {"-f", "--file", EGetOpt::required_argument, EGetOpt::dtString},
       {"", "", EGetOpt::no_argument, EGetOpt::dtNone},
   };

   EGetOpt opt;
   opt.loadCmdLine(argc, argv, options);
   if (opt.getCmdLine("-h,--help", false))
   {
      usage();
      exit(0);
   }

   EString optFile;
   optFile.format("%s.json", argv[0]);
   opt.loadFile(optFile.c_str());

   optFile = opt.getCmdLine("-f,--file", "");
   if (!optFile.empty())
      opt.loadFile(optFile.c_str());

   defaultLocale = std::cout.getloc();
   mylocale = std::locale("");
   
   std::cout.imbue(mylocale);

   try
   {
      EpcTools::Initialize(opt);

      run();

      EpcTools::UnInitialize();
   }
   catch (EError &e)
   {
      cout << (cpStr)e << endl;
   }

   return 0;
}
