/*
* Copyright (c) 2017 Sprint
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

#ifndef __DNSCACHE_H
#define __DNSCACHE_H

#include <list>
#include <map>
#include <ares.h>

#include "dnsquery.h"
#include "eatomic.h"
#include "esynch.h"
#include "ethread.h"

namespace DNS
{
   typedef int namedserverid_t;

   class Cache;
   class QueryProcessor;

   const namedserverid_t NS_DEFAULT = 0;

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////
   
   class QueryProcessorThread : public EThreadBasic
   {
      friend QueryProcessor;

   public:
      QueryProcessorThread(QueryProcessor &qp);

      Void incActiveQueries() { m_activequeries.Increment(); }
      Void decActiveQueries() { m_activequeries.Decrement(); }
      int getActiveQueries()  { return m_activequeries.currCount(); }

      virtual Dword threadProc(Void *arg);

      Void shutdown();

   protected:
      static Void ares_callback( Void *arg, int status, int timeouts, unsigned char *abuf, int alen );

   private:
      QueryProcessorThread();
      Void wait_for_completion();

      bool m_shutdown;
      QueryProcessor &m_qp;
      ESemaphorePrivate m_activequeries;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   struct NamedServer
   {
      char address[128];
      int family;
      int udp_port;
      int tcp_port;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class QueryProcessor
   {
      friend Cache;
      friend QueryProcessorThread;
   public:

      QueryProcessor( Cache &cache );
      ~QueryProcessor();

      Cache &getCache() { return m_cache; }

      Void shutdown();

      QueryProcessorThread *getQueryProcessorThread() { return &m_qpt; }

      Void addNamedServer(const char *address, int udp_port, int tcp_port);
      Void removeNamedServer(const char *address);
      Void applyNamedServers();

      EMutexPrivate &getChannelMutex() { return m_mutex; }

   protected:
      ares_channel getChannel() { return m_channel; }

      Void beginQuery( QueryPtr &q );
      Void endQuery();

   private:
      QueryProcessor();
      Void init();

      Cache &m_cache;
      QueryProcessorThread m_qpt;
      ares_channel m_channel;
      std::map<const char *,NamedServer> m_servers;
      EMutexPrivate m_mutex;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   #define SAVED_QUERY_TYPE "type"
   #define SAVED_QUERY_DOMAIN "domain"

   const uint16_t CR_SAVEQUERIES = EM_USER + 1;
   const uint16_t CR_FORCEREFRESH = EM_USER + 2;

   class CacheRefresher : EThreadPrivate
   {
      friend Cache;

   protected:
      CacheRefresher(Cache &cache, unsigned int maxconcur, int percent, long interval);

      virtual Void onInit();
      virtual Void onQuit();
      virtual Void onTimer( EThreadBase::Timer &timer );
      Void saveQueries( EThreadMessage &msg ) { _saveQueries(); }
      Void forceRefresh( EThreadMessage &msg ) { _forceRefresh(); }

      const EString &queryFileName() { return m_qfn; }
      long querySaveFrequency() { return m_qsf; }

      Void loadQueries(const char *qfn);
      Void loadQueries(const std::string &qfn) { loadQueries(qfn.c_str()); }
      Void initSaveQueries(const char *qfn, long qsf);
      Void saveQueries() { sendMessage(CR_SAVEQUERIES); }
      Void forceRefresh() { sendMessage(CR_FORCEREFRESH); }

      DECLARE_MESSAGE_MAP()

   private:
      CacheRefresher();
      static Void callback( QueryPtr q, bool cacheHit, const Void *data );
      Void _submitQueries( std::list<QueryCacheKey> &keys );
      Void _refreshQueries();
      Void _saveQueries();
      Void _forceRefresh();

      Cache &m_cache;
      ESemaphorePrivate m_sem;
      int m_percent;
      EThreadBase::Timer m_timer;
      long m_interval;
      bool m_running;
      EString m_qfn;
      long m_qsf;
      EThreadBase::Timer m_qst;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class Cache
   {
      friend QueryProcessor;
      friend QueryProcessorThread;

      friend CacheRefresher;

   public:
      Cache();
      ~Cache();

      static Cache& getInstance(namedserverid_t nsid);
      static Cache& getInstance() { return getInstance(NS_DEFAULT); }

      static unsigned int getRefreshConcurrent() { return m_concur; }
      static unsigned int setRefreshConcurrent(unsigned int concur) { return m_concur = concur; }

      static int getRefreshPercent() { return m_percent; }
      static int setRefreshPercent(int percent) { return m_percent = percent; }

      static long getRefeshInterval() { return m_interval; }
      static long setRefreshInterval(long interval) { return m_interval = interval; }

      Void addNamedServer(const char *address, int udp_port=53, int tcp_port=53);
      Void removeNamedServer(const char *address);
      Void applyNamedServers();

      QueryPtr query( ns_type rtype, const std::string &domain, bool &cacheHit, bool ignorecache=false );
      Void query( ns_type rtype, const std::string &domain, CachedDNSQueryCallback cb, const Void *data=NULL, bool ignorecache=false );

      Void loadQueries(const char *qfn);
      Void loadQueries(const std::string &qfn) { loadQueries(qfn.c_str()); }
      Void initSaveQueries(const char *qfn, long qsf);
      Void saveQueries();
      Void forceRefresh();

      namedserverid_t getNamedServerId() { return m_nsid; }

      long resetNewQueryCount() { return atomic_swap(m_newquerycnt, 0); }

   protected:
      Void updateCache( QueryPtr q );
      QueryPtr lookupQuery( ns_type rtype, const std::string &domain );
      QueryPtr lookupQuery( QueryCacheKey &qck );

      Void identifyExpired( std::list<QueryCacheKey> &keys, int percent );
      Void getCacheKeys( std::list<QueryCacheKey> &keys );


   private:

      static int m_ref;
      static unsigned int m_concur;
      static int m_percent;
      static long m_interval;

      QueryProcessor m_qp;
      CacheRefresher m_refresher;
      QueryCache m_cache;
      namedserverid_t m_nsid;
      ERWLock m_cacherwlock;
      long m_newquerycnt;
   };
}

#endif // #ifndef __DNSCACHE_H
