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

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "etimerpool.h"

#include <unistd.h>
#include <sys/syscall.h>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
ETimerPoolError_CreatingTimer::ETimerPoolError_CreatingTimer()
{
   setSevere();
   setTextf("%s: Error executing timer_create() - ", Name());
   appendLastOsError();
}

ETimerPoolError_TimerSetTimeFailed::ETimerPoolError_TimerSetTimeFailed()
{
   setSevere();
   setTextf("%s: Error executing timer_settime() - ", Name());
   appendLastOsError();
}
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ETimerPool *ETimerPool::m_instance = NULL;

ETimerPool::ETimerPool()
   : m_thread(*this)
{
   m_nextid = 0;
   m_sigtimer = SIGRTMIN + 2;
   m_sigquit = SIGRTMIN + 3;
   m_resolution = 5000;
   m_rounding = Rounding::down;
}

ETimerPool::~ETimerPool()
{
}

ULong ETimerPool::registerTimer(LongLong ms, const _EThreadEventMessageBase &msg, _EThreadEventBase &thread)
{
   EMutexLock l(m_mutex);

   ExpirationTime et(ms, getRounding());
   ExpirationTimeEntryPtr etep;
   EntryPtr ep;

   // create the entry pointer
   ep = std::make_shared<Entry>(assignNextId(), et, msg, thread);

   // see if the expiration time already exists
   auto srch = m_etmap.find( et.getExpireTime() );
   if (srch == m_etmap.end()) // the expiration time does not exist, so add a one
   {
      TimerPtr tp;

      // get the timer 
      if (m_freetimers.empty())
      {
         tp = std::make_shared<Timer>();
         if (!tp->create(m_thread.getThreadId(), getTimerSignal()))
            throw ETimerPoolError_CreatingTimer();
      }
      else
      {
         tp = m_freetimers.front();
         m_freetimers.pop_front();
      }

      // create the expiration time entry
      etep = std::make_shared<ExpirationTimeEntry>( et.getExpireTime() );

      // associate the timer with the expiration time entry
      etep->setTimer( tp );

      // associate the expiration time entry with the timer
      tp->setExpirationTimeEntry( etep );

      // start the timer
      tp->start();

      // add the expiration time entry to the expiration time entry map
      m_etmap[etep->getExpireTime()] = etep;
   }
   else // the expiration time exists
   {
      etep = srch->second;
   }

   // add the entry to the expiration time entry object
   etep->addEntry( ep );

   // add the expiration time entry to the entry ID map
   m_etidmap[ep->getId()] = etep;

   return ep->getId();
}

ETimerPool &ETimerPool::unregisterTimer(ULong id)
{
   EMutexLock l(m_mutex);

   // retrieve the expiration time entry from the id map
   auto idsrch = m_etidmap.find( id );
   if (idsrch != m_etidmap.end()) // if the entry was found
   {
      ExpirationTimeEntryPtr etep = idsrch->second;
      
      // remove from expiration time entry's entry map
      etep->removeEntry( id );

      // remove the expiration entry if necessary
      removeExpirationTimeEntry( etep );

      // remove trom the expiration time entry ID map
      m_etidmap.erase( idsrch );
   }

   return *this;
}

ETimerPool &ETimerPool::removeExpirationTimeEntry(ETimerPool::ExpirationTimeEntryPtr &etep)
{
   if (etep->isEntryMapEmpty())
   {
      // get the timer
      auto t = etep->getTimer();

      // stop the timer
      t->stop();

      // remove from the expiration time entry map
      m_etmap.erase( etep->getExpireTime() );

      // clear the expiration time entry ptr in the timer
      t->clearExpirationTimeEntry();

      // add the timer to the free timer list
      m_freetimers.push_back( t );
   }

   return *this;
}

/// @cond DOXYGEN_EXCLUDE
Void ETimerPool::sendNotifications(ETimerPool::ExpirationTimeEntryPtr &etep)
{
   EMutexLock l(m_mutex);

   for (auto entry = etep->getEntryMap().begin(); entry != etep->getEntryMap().end();)
   {
      // send the notification
      entry->second->notify();

      // remove from the expiration time entry ID map
      m_etidmap.erase( entry->second->getId() );

      // remove from the entry map
      entry = etep->getEntryMap().erase( entry );
   }

   if (etep->isEntryMapEmpty())
      removeExpirationTimeEntry( etep );
}
/// @cond DOXYGEN_EXCLUDE

Void ETimerPool::init()
{
   EEvent evnt;

   // start the thread
   m_thread.init(&evnt);

   // wait for the thread id to be populated
   evnt.wait();
}

Void ETimerPool::uninit(Bool dumpit)
{
   while (!m_etidmap.empty())
   {
      ULong id = m_etidmap.begin()->first;
      unregisterTimer( id );
   }

   m_thread.quit();
   m_thread.join();

   while (!m_freetimers.empty())
      m_freetimers.pop_front();
   
   if (dumpit)
      dump();

   m_instance = NULL;
   delete this;
}

ULong ETimerPool::assignNextId()
{
   ULong id = ++m_nextid;

   if (id == 0)
      id = ++m_nextid;
   
   return id;
}

Void ETimerPool::dump()
{
   std::cout << std::string(80,'*') << std::endl;
   std::cout << "ETimerPool::dump() - m_etidmap.size() = " << m_etidmap.size() << std::endl;
   for (auto &a : m_etidmap)
   {
      std::cout << "\tID=" << a.first << " entrymap size=" << a.second->getEntryMap().size() << std::endl;
      for (auto &x : a.second->getEntryMap())
      {
         std::cout << "\t\tID=" << x.second->getId()
            << " duration=" << x.second->getExpirationTime().getDuration()
            << " expiretime=" << x.second->getExpirationTime().getExpireTime()
            << std::endl;
      }
   }

   std::cout << "ETimerPool::dump() - m_etmap.size() = " << m_etmap.size() << std::endl;
   for (auto &b : m_etmap)
   {
      std::cout << "\texpiretime=" << b.first
         << " timerhandle=" << b.second->getTimer()->getHandle()
         << " entrymap size=" << b.second->getEntryMap().size() << std::endl;
      for (auto &x : b.second->getEntryMap())
      {
         std::cout << "\t\tID=" << x.second->getId()
            << " duration=" << x.second->getExpirationTime().getDuration()
            << " expiretime=" << x.second->getExpirationTime().getExpireTime()
            << std::endl;
      }
   }

   std::cout << "ETimerPool::dump() - m_freetimers.size() = " << m_freetimers.size() << std::endl;

   std::cout << std::string(80,'*') << std::endl;
   std::cout << std::flush;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

ETimerPool::Timer::Timer()
{
   m_timer = 0;
}

ETimerPool::Timer::~Timer()
{
   if (m_timer)
   {
      timer_delete( m_timer );
      m_timer = 0;
   }
}

Bool ETimerPool::Timer::create(pid_t tid, int sig)
{
   int status;
   struct sigevent se;

   memset( &se, 0, sizeof(se) );
   se.sigev_notify = SIGEV_THREAD_ID;
   se._sigev_un._tid = tid;
   se.sigev_signo = sig;
   se.sigev_value.sival_ptr = this;

   status = timer_create(CLOCK_REALTIME, &se, &m_timer);

   return status == -1 ? False : True;
}

Void ETimerPool::Timer::start()
{
   struct itimerspec ts = {};

   ts.it_interval.tv_sec = 0;
   ts.it_interval.tv_nsec = 0;
   ts.it_value.tv_sec = m_etep->getExpireTime() / 1000000;
   ts.it_value.tv_nsec = m_etep->getExpireTime() % 1000000 * 1000;

   if (timer_settime(m_timer, TIMER_ABSTIME, &ts, NULL) == -1)
      throw ETimerPoolError_TimerSetTimeFailed();
}

Void ETimerPool::Timer::stop()
{
   struct itimerspec ts = {};

   timer_settime(m_timer, 0, &ts, NULL);
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE
Void ETimerPool::Entry::notify()
{
   m_thread._sendMessage( m_msg );
}
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

ETimerPool::Thread::Thread(ETimerPool &tp)
   : m_tp(tp)
{
}

Void ETimerPool::Thread::quit()
{
   signal( m_tp.getQuitSignal() );
}

Dword ETimerPool::Thread::threadProc(Void *arg)
{
   Bool run = True;
   EEvent *evnt = (EEvent*)arg;
   sigset_t set;
   siginfo_t si;
   ETimerPool &tp( ETimerPool::Instance() );

   // retrieve the thread for this thread
   m_tid = syscall(SYS_gettid);
   evnt->set();

   sigemptyset( &set );
   sigaddset( &set, tp.getTimerSignal() );
   sigaddset( &set, tp.getQuitSignal() );

   while (run)
   {
      int sig = sigwaitinfo( &set, &si );

      if (sig == tp.getTimerSignal())
      {
         Instance().sendNotifications( ((Timer*)si.si_value.sival_ptr)->getExpirationTimeEntry() );
      }
      else if (sig == tp.getQuitSignal())
      {
         run = False;
      }
   }

   return 0;
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
