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

#ifndef __ETIMERPOOL_H
#define __ETIMERPOOL_H

#include <atomic>
#include <unordered_map>

#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include "esynch.h"
#include "etevent.h"
#include "etime.h"

DECLARE_ERROR_ADVANCED(ETimerPoolError_CreatingTimer);
DECLARE_ERROR_ADVANCED(ETimerPoolError_TimerSetTimeFailed);

class ETimerPool
{
protected:
   // forward declarations
   class Timer;
   typedef std::shared_ptr<Timer> TimerPtr;
   typedef std::list<TimerPtr> TimerPtrList;

   class Entry;
   typedef std::shared_ptr<Entry> EntryPtr;
   typedef std::unordered_map<ULong,EntryPtr> EntryMap;

   class ExpirationTime;
   class ExpirationTimeEntry;
   typedef std::shared_ptr<ExpirationTimeEntry> ExpirationTimeEntryPtr;
   typedef std::unordered_map<LongLong,ExpirationTimeEntryPtr> ExpirationTimeEntryMap;
   typedef std::unordered_map<ULong,ExpirationTimeEntryPtr> ExpirationTimeEntryIdMap;

   class Thread;

   friend ExpirationTime;
public:
   /// @brief Defines how rounding will be performed.
   enum class Rounding
   {
      /// Rounds up
      up,
      /// Rounds down
      down
   };

   /// @brief Retrieves the single instance of the ETimerPool object.
   /// @return a reference to the single instance of the ETimerPool object.
   static ETimerPool &Instance()
   {
      if (!m_instance)
         m_instance = new ETimerPool();
      return *m_instance;
   }

   /// @brief Default constructor.
   ETimerPool();
   /// @brief Class destructor.
   ~ETimerPool();

   /// @brief Retrieves the current timer resolution value.
   /// @param raw True retrieves the value in milliseconds, otherwise microseconds.
   /// @return the current timer resolution value.
   LongLong getResolution(Bool raw=False) { return raw ? m_resolution : m_resolution / 1000; }
   /// @brief Retrieves the current rounding value.
   /// @return the current rounding value.
   Rounding getRounding()                 { return m_rounding; }
   /// @brief Retrieves the current timer signal value.
   /// @return the current timer signal value.
   Int getTimerSignal()                   { return m_sigtimer; }
   /// @brief Retrieves the current quit signal value.
   /// @return the current quit signal value.
   Int getQuitSignal()                    { return m_sigquit; }

   /// @brief Assigns the timer resolution value.
   /// @param ms the resolution in milliseconds.
   /// @return a reference to the ETimerPool object.
   ETimerPool &setResolution(LongLong ms) { m_resolution = ms * 1000;   return *this; }
   /// @brief Assigns the timer rounding method.
   /// @param r the timer rounding method.
   /// @return a reference to the ETimerPool object.
   ETimerPool &setRounding(Rounding r)    { m_rounding = r;             return *this; }
   /// @brief Assigns the timer signal value.
   /// @param sig the timer signal value.
   /// @return a reference to the ETimerPool object.
   ETimerPool &setTimerSignal(Int sig)    { m_sigtimer = sig;           return *this; }
   /// @brief Assigns the quit signal value.
   /// @param sig the quit signal value.
   /// @return a reference to the ETimerPool object.
   ETimerPool &setQuitSignal(Int sig)     { m_sigquit = sig;            return *this; }

   /// @brief Registers an expiration timer.
   /// @param ms the length of the timer in milliseconds.
   /// @param msg the thread message to post when the timer expires.
   /// @param thread the thread to post the message to when the timer expires.
   /// @return the ID for this timer.
   ULong registerTimer(LongLong ms, const _EThreadEventMessageBase &msg, _EThreadEventBase &thread);
   /// @brief Unregisters an expiration timer.
   /// @param timerid the ID of the timer to unregister (returned by registerTimer).
   /// @return a reference to the ETimerPool object.
   ETimerPool &unregisterTimer(ULong timerid);
   /// @brief Initializes the ETimerPool.
   Void init();
   /// @brief Uninitializes the ETimerPool.
   Void uninit(Bool dumpit=False);

   /// @brief Prints the contents of the internal collections.
   Void dump();

protected:
   /// @cond DOXYGEN_EXCLUDE
   /////////////////////////////////////////////////////////////////////////////
   
   class Timer
   {
      friend class ETimerPool;

   public:
      Timer();
      ~Timer();

      Bool create(pid_t tid, int sig);

      Timer &setExpirationTimeEntry(ExpirationTimeEntryPtr &etep) { m_etep = etep; return *this; }
      ExpirationTimeEntryPtr &getExpirationTimeEntry() { return m_etep; }
      Timer & clearExpirationTimeEntry() { m_etep.reset(); return *this; }

      Void start();
      Void stop();

      timer_t getHandle() { return m_timer; }

   private:
      timer_t m_timer;
      ExpirationTimeEntryPtr m_etep;
   };

   /////////////////////////////////////////////////////////////////////////////
   
   class ExpirationTime
   {
   public:
      ExpirationTime()
         : m_duration(0),
           m_expiretime(0)
      {
      }
      ExpirationTime(LongLong ms, Rounding rounding = Instance().m_rounding)
         : m_duration(0),
           m_expiretime(0)
      {
         setDuration( ms, rounding );
      }
      ExpirationTime(const ExpirationTime &exptm)
         : m_duration(0),
           m_expiretime(0)
      {
         m_duration = exptm.m_duration;
         m_expiretime = exptm.m_expiretime;
      }

      ExpirationTime &operator=(const ExpirationTime et)
      {
         m_duration = et.m_duration;
         m_expiretime = et.m_expiretime;
         return *this;
      }

      LongLong getDuration() { return m_duration; }
      ExpirationTime &setDuration(LongLong ms, Rounding rounding = Instance().m_rounding)
      {
         LongLong resolution = Instance().getResolution(True);

         m_duration = ms;

         ETime et = ETime() + ETime(m_duration);

         m_expiretime =
            ((et.getTimeVal().tv_sec * 1000000 + et.getTimeVal().tv_usec % 1000000) / resolution +
            (rounding == Rounding::down ? 0 : 1)) * resolution;

         return *this;
      }

      LongLong getExpireTime()
      {
         return m_expiretime;
      }

   private:
      LongLong m_duration; // in milliseconds
      LongLong m_expiretime; // m_expiretime = tv_sec * 1000000 + tv_usec % 1000000;
   };
   
   /////////////////////////////////////////////////////////////////////////////

   class Entry
   {
   public:
      Entry(ULong id, ExpirationTime &exptm, const _EThreadEventMessageBase &msg, _EThreadEventBase &thread)
         : m_id( id ),
           m_exptm( exptm ),
           m_msg( msg ),
           m_thread( thread )
      {
      }
      Entry(const Entry &e)
         : m_id( e.m_id ),
           m_exptm( e.m_exptm ),
           m_msg( e.m_msg ),
           m_thread( e.m_thread )
      {
      }

      ULong getId()                                { return m_id; }
      const _EThreadEventMessageBase &getMessage() { return m_msg; }
      _EThreadEventBase &getThread()               { return m_thread; }
      ExpirationTime &getExpirationTime()          { return m_exptm; }

      Void notify();

   private:
      Entry();

      ULong m_id;
      ExpirationTime m_exptm;
      _EThreadEventMessageBase m_msg;
      _EThreadEventBase &m_thread;
   };

   class ExpirationTimeEntry
   {
   public:
      ExpirationTimeEntry(LongLong expiretime)
         : m_expiretime( expiretime )
      {
      }

      LongLong getExpireTime() { return m_expiretime; }
      ExpirationTimeEntry &setExpireTime(LongLong exp) { m_expiretime = exp; return *this; }

      ExpirationTimeEntry &addEntry(EntryPtr &ep)
      {
         m_map[ep->getId()] = ep;
         return *this;
      }

      Void removeEntry(ULong id)
      {
         m_map.erase( id );
      }

      Bool isEntryMapEmpty()
      {
         return m_map.empty();
      }

      ExpirationTimeEntry &setTimer(TimerPtr &tp)
      {
         m_timer = tp;
         return *this;
      }

      TimerPtr &getTimer()
      {
         return m_timer;
      }

      EntryMap &getEntryMap()
      {
         return m_map;
      }

      ExpirationTimeEntry &notify()
      {
         for (auto it = m_map.begin(); it != m_map.end();)
         {
            // send the notificaiton for the entry
            it->second->notify();

            // remove the entry
            it = m_map.erase( it );
         }
      }

   private:
      ExpirationTimeEntry();
      LongLong m_expiretime;
      TimerPtr m_timer;
      EntryMap m_map;
   };

   /////////////////////////////////////////////////////////////////////////////
   
   class Thread : public EThreadBasic
   {
   public:
      Thread(ETimerPool &tp);

      pid_t getThreadId() { return m_tid; }

      Void quit();

      Dword threadProc(Void *arg);

   private:
      Thread();

      ETimerPool &m_tp;
      pid_t m_tid;
   };

   friend Thread;

   /////////////////////////////////////////////////////////////////////////////

   Void sendNotifications(ExpirationTimeEntryPtr &etep);

   /// @endcond

private:
   static ETimerPool *m_instance;

   ULong assignNextId();
   ETimerPool &removeExpirationTimeEntry(ETimerPool::ExpirationTimeEntryPtr &etep);

   EMutexPrivate m_mutex;
   std::atomic<ULong> m_nextid;
   Int m_sigtimer;
   Int m_sigquit;
   Rounding m_rounding;
   LongLong m_resolution; // in microseconds
   ExpirationTimeEntryMap m_etmap;
   ExpirationTimeEntryIdMap m_etidmap;
   TimerPtrList m_freetimers;
   Thread m_thread;
};

#endif // #define __ETIMERPOOL_H