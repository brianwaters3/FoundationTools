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

/// @file
/// @brief Defines classes related to the DNS cache.

#include <list>
#include <map>
#include <ares.h>

#include "dnsquery.h"
#include "eatomic.h"
#include "esynch.h"
#include "etevent.h"

namespace DNS
{
   /// @cond DOXYGEN_EXCLUDE

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

      Bool m_shutdown;
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
      virtual Void onTimer( EThreadEventTimer &timer );
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
      static Void callback( QueryPtr q, Bool cacheHit, const Void *data );
      Void _submitQueries( std::list<QueryCacheKey> &keys );
      Void _refreshQueries();
      Void _saveQueries();
      Void _forceRefresh();

      Cache &m_cache;
      ESemaphorePrivate m_sem;
      int m_percent;
      EThreadEventTimer m_timer;
      long m_interval;
      Bool m_running;
      EString m_qfn;
      long m_qsf;
      EThreadEventTimer m_qst;
   };

   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Defines the functionality associated with a DNS cache
   class Cache
   {
      friend QueryProcessor;
      friend QueryProcessorThread;

      friend CacheRefresher;

   public:
      /// @brief Default constructor.
      Cache();
      /// @brief Class destructor.
      ~Cache();

      /// @brief Retrieves/creates the Cache instance associated with the named server ID.
      /// @param nsid the named server ID.
      /// @return a reference to the Cache object.
      static Cache& getInstance(namedserverid_t nsid);
      /// @brief Retrieves/creates the default Cache instance.
      /// @return a reference to the Cache object.
      static Cache& getInstance() { return getInstance(NS_DEFAULT); }

      /// @brief Retrieves the current setting of the maximum number of conncurrent DNS
      ///   queries that can be performed while refreshing the DNS cache.
      static unsigned int getRefreshConcurrent() { return m_concur; }
      /// @brief Sets the maximum number of conncurrent DNS
      ///   queries that can be performed while refreshing the DNS cache.
      /// @param concur the maximum number of concurrent DNS queries.
      /// @return the current maximum number of concurrent DNS queries.
      static unsigned int setRefreshConcurrent(unsigned int concur) { return m_concur = concur; }

      /// @brief Retrieves the current refresh percentage value.
      /// @return the current refresh percenting value.
      static int getRefreshPercent() { return m_percent; }
      /// @brief Assigns the refresh percentage value.
      /// @param percent the refresh percentage value.
      /// @return the refresh percenting value.
      static int setRefreshPercent(int percent) { return m_percent = percent; }

      /// @brief Retrieves the refresh interval.
      /// @return the refresh interval.
      static long getRefeshInterval() { return m_interval; }
      /// @brief Assigns the refresh interval.
      /// @param interval the new refresh interval.
      /// @return the refresh interval.
      static long setRefreshInterval(long interval) { return m_interval = interval; }

      /// @brief Adds a named server to this DNS cache object.
      /// @param address the address of the named server.
      /// @param udp_port the UDP port to communicate with the DNS server on.
      /// @param tcp_port the TCP port to communicate with the DNS server on.
      Void addNamedServer(const char *address, int udp_port=53, int tcp_port=53);
      /// @brief Removes the specified named server from this DNS cache.
      /// @param address the address of the named server to remove.
      Void removeNamedServer(const char *address);
      /// @brief Updates the named servers as a set in the underlying c-ares library.
      Void applyNamedServers();

      /// @brief Performs a DNS query synchronously.
      /// @param rtype the named server type of the query.
      /// @param domain the domain name of the query.
      /// @param cacheHit updated with an indication if the result came from the local DNS cache.
      /// @param ignorecache directs the query to optionally ignore any results in the local DNS cache.
      /// @return a QueryPtr with the DNS query results.
      QueryPtr query( ns_type rtype, const std::string &domain, Bool &cacheHit, Bool ignorecache=false );
      /// @brief Performs a DNS query asynchronously.
      /// @param rtype the named server type of the query.
      /// @param domain the domain name of the query.
      /// @param cb a callback function pointer that will be called when the query is complete.
      /// @param data a Void pointer that will be passed to the callback function when the query is complete.
      /// @param ignorecache directs the query to optionally ignore any results in the local DNS cache.
      Void query( ns_type rtype, const std::string &domain, CachedDNSQueryCallback cb, const Void *data=NULL, Bool ignorecache=false );

      /// @brief Executes the DNS queries at startup from the suppoied file.
      /// @param qfn the DNS query file name to load.
      Void loadQueries(const char *qfn);
      /// @brief Executes the DNS queries at startup from the suppoied file.
      /// @param qfn the DNS query file name to load.
      Void loadQueries(const std::string &qfn) { loadQueries(qfn.c_str()); }
      /// @brief Initializes the settings used to save queries.
      /// @param qfn the file name to save the DNS queries to.
      /// @param qsf the frequency in milliseconds to save the DNS queries.
      Void initSaveQueries(const char *qfn, long qsf);
      /// @brief Saves the DNS queries that are part of the cache.
      Void saveQueries();
      /// @brief Forces a refresh of the DNS cache.
      Void forceRefresh();

      /// @brief Retrieves the named server ID associated with this DNS cache.
      /// @return the named server ID associated with this DNS cache.
      namedserverid_t getNamedServerId() { return m_nsid; }

      /// @brief Resets the number of new queries (not saved) to zero.
      /// @return the previous the number of new queries (not saved).
      long resetNewQueryCount() { return atomic_swap(m_newquerycnt, 0); }

   protected:
      /// @cond DOXYGEN_EXCLUDE
      Void updateCache( QueryPtr q );
      QueryPtr lookupQuery( ns_type rtype, const std::string &domain );
      QueryPtr lookupQuery( QueryCacheKey &qck );

      Void identifyExpired( std::list<QueryCacheKey> &keys, int percent );
      Void getCacheKeys( std::list<QueryCacheKey> &keys );
      /// @endcond

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
