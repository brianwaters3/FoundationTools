/*
* Copyright (c) 2017 Sprint
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

#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <poll.h>

#include <fstream>
#include <iostream>
#include <list>

#include "epctools.h"
#include "eerror.h"
#include "etevent.h"
#include "esynch.h"
#include "dnscache.h"
#include "dnsparser.h"

using namespace DNS;

#define RAPIDJSON_NAMESPACE dnsrapidjson
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

using namespace RAPIDJSON_NAMESPACE;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// template<class T>
// class Buffer
// {
// public:
//    Buffer(size_t size) { msize = size; mbuf = new T[msize]; }
//    ~Buffer() { if (mbuf) delete [] mbuf; }
//    T *get() { return mbuf; }
// private:
//    Buffer();
//    size_t msize;
//    T *mbuf;
// };

// static EString string_format( const char *format, ... )
// {
//    va_list args;

//    va_start( args, format );
//    size_t size = vsnprintf( NULL, 0, format, args ) + 1; // Extra space for '\0'
//    va_end( args );

//    Buffer<char> buf( size );

//    va_start( args, format );
//    vsnprintf( buf.get(), size, format, args  );
//    va_end( args );

//    return std::string( buf.get(), size - 1 ); // We don't want the '\0' inside
// }

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace DNS
{
   /// @cond DOXYGEN_EXLCUDE
   QueryProcessorThread::QueryProcessorThread(QueryProcessor &qp)
      : m_shutdown( false ),
        m_qp(qp),
        m_activequeries(0)
   {
   }

   Dword QueryProcessorThread::threadProc(Void *arg)
   {
      while ( true )
      {
         int qcnt = m_activequeries.currCount();

         if ( qcnt == 0 )
         {
            // wait for a new query to be submitted or for shutdown
            decActiveQueries();

            if ( m_shutdown && getActiveQueries() == 0 )
               break;

            incActiveQueries();
         }
         else if ( m_shutdown && qcnt == 1 )
         {
            break;
         }

         wait_for_completion();
      }

      return 0;
   }

   Void QueryProcessorThread::wait_for_completion()
   {
      struct timeval tv;
      int timeout;
      ares_socket_t sockets[ ARES_GETSOCK_MAXNUM ];
      struct pollfd fds[ ARES_GETSOCK_MAXNUM ];

      while( true )
      {
         int rwbits = ares_getsock( m_qp.getChannel(), sockets, ARES_GETSOCK_MAXNUM );
         int fdcnt = 0;

         memset( fds, 0, sizeof(fds) );

         for ( int i = 0; i < ARES_GETSOCK_MAXNUM; i++ )
         {
            fds[fdcnt].fd = sockets[i];
            fds[fdcnt].events |= ARES_GETSOCK_READABLE(rwbits,i) ? (POLLIN | POLLRDNORM) : 0;
            fds[fdcnt].events |= ARES_GETSOCK_WRITABLE(rwbits,i) ? (POLLOUT | POLLWRNORM) : 0;
            if ( fds[fdcnt].events != 0 )
               fdcnt++;
         }

         if ( !fdcnt )
            break;

         memset( &tv, 0, sizeof(tv) );
         ares_timeout( m_qp.getChannel(), NULL, &tv );
         timeout = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

         if ( poll(fds,fdcnt,timeout) != 0 )
         {
            for ( int i = 0; i < fdcnt; i++ )
            {
               if ( fds[i].revents != 0 )
               {
                  EMutexLock l( m_qp.getChannelMutex() );
                  ares_process_fd( m_qp.getChannel(),
                     fds[i].revents & (POLLIN | POLLRDNORM) ? fds[i].fd : ARES_SOCKET_BAD,
                     fds[i].revents & (POLLOUT | POLLWRNORM) ? fds[i].fd : ARES_SOCKET_BAD);
               }
            }
         }
         else
         {
            // timeout
            EMutexLock l( m_qp.getChannelMutex() );
            ares_process_fd( m_qp.getChannel(), ARES_SOCKET_BAD, ARES_SOCKET_BAD );
         }
      }
   }

   Void QueryProcessorThread::shutdown()
   {
      m_shutdown = true;
      incActiveQueries();
      join();
   }

   Void QueryProcessorThread::ares_callback( Void *arg, int status, int timeouts, unsigned char *abuf, int alen )
   {
      QueryPtr *qq = reinterpret_cast<QueryPtr*>(arg);

      QueryProcessor *qp = (*qq)->getQueryProcessor();

      if (qp)
      {
         qp->endQuery();

         try
         {
            Parser p( *qq, abuf, alen );
            p.parse();
         }
         catch (std::exception &ex)
         {
            (*qq)->setError( true );
            (*qq)->setErrorMsg( ex.what() );
         }

         if ( !(*qq)->getError() )
         {
            qp->getCache().updateCache( *qq );
         }

         if ( (*qq)->getCompletionEvent() )
            (*qq)->getCompletionEvent()->set();

         if ( (*qq)->getCallback() )
         {
            const Void *data = (*qq)->getData();
            (*qq)->setData(NULL);
            (*qq)->getCallback()( *qq, false, data );
         }
      }

      delete qq;
   }
   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE
   QueryProcessor::QueryProcessor( Cache &cache )
      : m_cache( cache ),
        m_qpt( *this )
   {
      m_channel = NULL;

      init();

      m_qpt.init( NULL );
   }

   QueryProcessor::~QueryProcessor()
   {
      ares_destroy( m_channel );
   }

   Void QueryProcessor::init()
   {
      int status;
      if ( (status = ares_init(&m_channel)) == ARES_SUCCESS )
      {
         struct ares_options opt;
         opt.timeout = 1000;
         opt.ndots = 0;
         opt.flags = ARES_FLAG_EDNS;
         opt.ednspsz = 8192;
         ares_init_options( &m_channel, &opt, ARES_OPT_TIMEOUTMS | ARES_OPT_NDOTS | ARES_OPT_EDNSPSZ | ARES_OPT_FLAGS );
      }
      else
      {
         EString msg;
         msg.format( "QueryProcessor::init() - ares_init() failed status = %d", status );
         throw EError( EError::Error, msg );
      }
   }

   Void QueryProcessor::shutdown()
   {
      m_qpt.shutdown();
   }

   Void QueryProcessor::addNamedServer(const char *address, int udp_port, int tcp_port)
   {
      union {
        struct in_addr       addr4;
        struct ares_in6_addr addr6;
      } addr;
      struct NamedServer ns;

      if (ares_inet_pton(AF_INET, address, &addr.addr4) == 1)
         ns.family = AF_INET;
      else if (ares_inet_pton(AF_INET6, address, &addr.addr6) == 1)
         ns.family = AF_INET6;
      else
      {
         EString msg;
         msg.format( "QueryProcessor::addNamedServer() - unrecognized address [%s]", address );
         throw EError( EError::Warning, msg );
      }

      ares_inet_ntop( ns.family, &addr, ns.address, sizeof(ns.address) );
      ns.udp_port = udp_port;
      ns.tcp_port = tcp_port;

      m_servers[ns.address] = ns;
   }

   Void QueryProcessor::removeNamedServer(const char *address)
   {
      m_servers.erase( address );
   }

   Void QueryProcessor::applyNamedServers()
   {
      ares_addr_port_node *head = NULL;

      // create linked list of named servers
      for (const auto &kv : m_servers)
      {
         ares_addr_port_node *p = new ares_addr_port_node();

         p->next = head;
         p->family = kv.second.family;
         ares_inet_pton( p->family, kv.second.address, &p->addr );
         p->udp_port = kv.second.udp_port;
         p->tcp_port = kv.second.tcp_port;

         head = p;
      }

      // apply the named server list
      int status = ares_set_servers_ports( m_channel, head );

      // delete the list of named servers
      while (head)
      {
         ares_addr_port_node *p = head;
         head = head->next;
         delete p;
      }
      // throw exception if needed
      if ( status != ARES_SUCCESS )
      {
         EString msg;
         msg.format( "QueryProcessor::updateNamedServers() - ares_set_servers_ports() failed status = %d", status );
         throw EError( EError::Warning, msg );
      }
   }

   Void QueryProcessor::beginQuery( QueryPtr &q )
   {
      m_qpt.incActiveQueries();
      q->setQueryProcessor( this );
      q->setError( false );

      QueryPtr *qq = new QueryPtr(q);

      if ( q->getCallback() || q->getCompletionEvent() )
      {
         EMutexLock l( getChannelMutex() );
         ares_query( m_channel, q->getDomain().c_str(), ns_c_in, q->getType(), QueryProcessorThread::ares_callback, qq );
      }
      else
      {
         EEvent event;
         q->setCompletionEvent( &event );
         {
            EMutexLock l( getChannelMutex() );
            ares_query( m_channel, q->getDomain().c_str(), ns_c_in, q->getType(), QueryProcessorThread::ares_callback, qq );
         }
         event.wait();
         q->setCompletionEvent( NULL );
      }
   }

   Void QueryProcessor::endQuery()
   {
      m_qpt.decActiveQueries();
   }
   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   int Cache::m_ref = 0;
   unsigned int Cache::m_concur = 10;
   int Cache::m_percent = 80;
   long Cache::m_interval = 60;

   Cache::Cache()
      : m_qp( *this ),
        m_refresher( *this, m_concur, m_percent, m_interval )
   {
      if (m_ref == 0)
      {
         int status = ares_library_init( ARES_LIB_INIT_ALL );
         if ( status != ARES_SUCCESS )
         {
            EString msg;
            msg.format( "Cache::Cache() - ares_library_init() failed status = %d", status );
            throw EError( EError::Error, msg );
         }
      }

      m_ref++;
      m_nsid = NS_DEFAULT;
      m_newquerycnt = 0;

      // start the refresh thread
      m_refresher.init(1, 1, NULL);
   }

   Cache::~Cache()
   {
      // stop the refresh thread
      m_refresher.quit();
      m_refresher.join();

      // stop the query processor
      m_qp.shutdown();

      m_ref--;

      if (m_ref == 0)
         ares_library_cleanup();
   }

   class CacheMap
   {
   public:
      ~CacheMap()
      {
         std::map<namedserverid_t, Cache*>::iterator it;

         while ((it = m_map.begin()) != m_map.end())
         {
            Cache *c = it->second;
            m_map.erase( it );
            delete c;
         }
      }

      std::map<namedserverid_t, Cache*> &getMap() { return m_map; }

   private:
      std::map<namedserverid_t, Cache*> m_map;
   };

   Cache& Cache::getInstance(namedserverid_t nsid)
   {
      static CacheMap cm;
      Cache *c;

      auto search = cm.getMap().find(nsid);
      if (search == cm.getMap().end())
      {
         c = new Cache();
         c->m_nsid = nsid;
         cm.getMap()[nsid] = c;
      }
      else
      {
         c = search->second;
      }

      return *c;
   }

   Void Cache::addNamedServer(const char *address, int udp_port, int tcp_port)
   {
      m_qp.addNamedServer(address, udp_port, tcp_port);
   }

   Void Cache::removeNamedServer(const char *address)
   {
      m_qp.removeNamedServer(address);
   }

   Void Cache::applyNamedServers()
   {
      m_qp.applyNamedServers();
   }

   QueryPtr Cache::query( ns_type rtype, const std::string & domain, Bool &cacheHit, Bool ignorecache )
   {
      QueryPtr q = lookupQuery( rtype, domain );

      cacheHit = !( !q || q->isExpired() );

      if ( !cacheHit || ignorecache ) // query not found or expired
      {
         q.reset( new Query( rtype, domain ) );
         m_qp.beginQuery( q );
         if (ignorecache)
            cacheHit = false;
      }

      return q;
   }

   Void Cache::query( ns_type rtype, const std::string &domain, CachedDNSQueryCallback cb, const Void *data, Bool ignorecache )
   {
      QueryPtr q = lookupQuery( rtype, domain );

      Bool cacheHit = !( !q || q->isExpired() );

      if ( cacheHit && !ignorecache )
      {
         if ( cb )
            cb( q, cacheHit, data );
      }
      else
      {
         q.reset( new Query( rtype, domain ) );
         q->setCallback( cb );
         q->setData( data );
         m_qp.beginQuery( q );
      }
   }

   Void Cache::loadQueries(const char *qfn)
   {
      m_refresher.loadQueries( qfn );
   }

   Void Cache::initSaveQueries(const char *qfn, long qsf)
   {
      m_refresher.initSaveQueries( qfn, qsf );
   }

   Void Cache::saveQueries()
   {
      m_refresher.saveQueries();
   }

   Void Cache::forceRefresh()
   {
      m_refresher.forceRefresh();
   }

   /// @cond DOXYGEN_EXCLUDE
   QueryPtr Cache::lookupQuery( ns_type rtype, const std::string &domain )
   {
      QueryCacheKey qck( rtype, domain );
      return lookupQuery( qck );
   }

   QueryPtr Cache::lookupQuery( QueryCacheKey &qck )
   {
      ERDLock l( m_cacherwlock );
      QueryCache::const_iterator it = m_cache.find( qck );
      return it != m_cache.end() ? it->second : QueryPtr();
   }

   Void Cache::updateCache( QueryPtr q )
   {
      if ( !q )
         return;

      if ( !q->getError() )
      {
         QueryCacheKey qck( q->getType(), q->getDomain() );
         EWRLock l( m_cacherwlock );
         if ( m_cache.find(qck) == m_cache.end() )
            atomic_inc_fetch( m_newquerycnt );
         m_cache[qck] = q;
      }
   }

   Void Cache::identifyExpired( std::list<QueryCacheKey> &keys, int percent )
   {
      ERDLock l( m_cacherwlock );

      for (auto val : m_cache)
      {
         QueryPtr q = val.second;
         if ( q )
         {
            if ( !q->isExpired() )
            {
               time_t diff = (q->getTTL() - (q->getExpires() - time(NULL))) * 100;
               int pcnt = diff / q->getTTL();
               if ( pcnt < percent )
                  continue;
            }
            keys.push_back( val.first );
         }
      }
   }

   Void Cache::getCacheKeys( std::list<QueryCacheKey> &keys )
   {
      ERDLock l( m_cacherwlock );

      for (auto val : m_cache )
         keys.push_back( val.first );
   }
   /// @endcond

   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE

   BEGIN_MESSAGE_MAP(CacheRefresher, EThreadPrivate)
      ON_MESSAGE(CR_SAVEQUERIES, CacheRefresher::saveQueries)
      ON_MESSAGE(CR_FORCEREFRESH, CacheRefresher::forceRefresh)
   END_MESSAGE_MAP()

   CacheRefresher::CacheRefresher(Cache &cache, unsigned int maxconcur, int percent, long interval)
      : m_cache( cache ),
        m_sem( maxconcur ),
        m_percent( percent ),
        m_interval( interval ),
        m_running( false ),
        m_qfn( "" ),
        m_qsf( 0 )
   {
   }

   Void CacheRefresher::onInit()
   {
      m_timer.setInterval( m_interval );
      m_timer.setOneShot( false );
      initTimer( m_timer );
      m_timer.start();
   }

   Void CacheRefresher::onQuit()
   {
      m_timer.stop();
   }

   Void CacheRefresher::onTimer( EThreadEventTimer &timer)
   {
      if (timer.getId() == m_timer.getId())
         _refreshQueries();
      else if (timer.getId() == m_qst.getId())
         _saveQueries();
   }

   Void CacheRefresher::callback( QueryPtr q, Bool cacheHit, const Void *data )
   {
      CacheRefresher *ths = (CacheRefresher*)data;
      ths->m_sem.Increment();
   }

   Void CacheRefresher::initSaveQueries(const char *qfn, long qsf)
   {
      m_qfn = qfn;
      m_qsf = qsf;

      if (querySaveFrequency() > 0 && !queryFileName().empty())
      {
         if ( m_qst.isInitialized() )
            m_qst.stop();

         m_qst.setInterval( querySaveFrequency() );
         m_qst.setOneShot( false );

         if ( !m_qst.isInitialized() )
            initTimer( m_qst );

         m_qst.start();
      }
   }

   Void CacheRefresher::_refreshQueries()
   {
      if (m_running)
         return;

      std::list<QueryCacheKey> keys;

      m_cache.identifyExpired( keys, m_percent );


      _submitQueries( keys );
   }

   Void CacheRefresher::_submitQueries( std::list<QueryCacheKey> &keys )
   {
      m_running = true;

      for (auto qck : keys)
      {
         m_sem.Decrement();
         m_cache.query( qck.getType(), qck.getDomain(), callback, this, true );
      }

      m_running = false;
   }

   Void CacheRefresher::_forceRefresh()
   {
      if (m_running)
         return;

      std::list<QueryCacheKey> keys;

      m_cache.getCacheKeys( keys );

      _submitQueries( keys );
   }

   Void CacheRefresher::_saveQueries()
   {
      long nqc = m_cache.resetNewQueryCount();
      if ( nqc == 0 )
      {
         //std::cout << "CacheRefresher::_saveQueries() - no changes to save" << std::endl;
         return;
      }

      std::list<QueryCacheKey> keys;
      Document d;
      Document::AllocatorType &allocator = d.GetAllocator();

      m_cache.getCacheKeys( keys );
      d.SetArray();

      //std::cout << "CacheRefresher::_saveQueries() - there are " << nqc << " new queries, saving " << keys.size() << " queries" << std::endl;

      for (auto qck : keys)
      {
         Value o( kObjectType );
         o.AddMember( SAVED_QUERY_TYPE, qck.getType(), allocator );
         o.AddMember( SAVED_QUERY_DOMAIN, Value(qck.getDomain().c_str(), allocator), allocator );
         d.PushBack( o, allocator );
      }

      FILE *fp = fopen( queryFileName().c_str(), "w" );
      if (fp)
      {
         char buf[65536];
         FileWriteStream fws( fp, buf, sizeof(buf) );
         Writer<FileWriteStream> writer( fws );
         d.Accept( writer );
         fclose( fp );
      }
   }

   Void CacheRefresher::loadQueries(const char *qfn)
   {
      Document doc;
      FILE *fp;
      char buf[65536];

      fp = fopen(qfn, "r");
      if ( fp )
      {
         FileReadStream frs(fp, buf, sizeof(buf));
         doc.ParseStream(frs);
         fclose(fp);

         if ( !doc.IsArray() )
         {
            EString msg;
            msg.format( "CacheRefresher::loadQueries() - root is not an array [%s]", qfn );
            throw EError( EError::Warning, msg );
         }

         for (auto &v : doc.GetArray())
         {
            if ( !v.HasMember(SAVED_QUERY_TYPE) || !v[SAVED_QUERY_TYPE].IsInt() )
               throw EError( EError::Warning, "CacheRefresher::loadQueries() - member \"" SAVED_QUERY_TYPE "\" missing or invalid type" );
            if ( !v.HasMember(SAVED_QUERY_DOMAIN) || !v[SAVED_QUERY_DOMAIN].IsString() )
               throw EError( EError::Warning, "CacheRefresher::loadQueries() - member \"" SAVED_QUERY_DOMAIN "\" missing or invalid type" );

            ns_type typ = (ns_type)v[SAVED_QUERY_TYPE].GetInt();
            const char *dmn = v[SAVED_QUERY_DOMAIN].GetString();

            m_sem.Decrement();
            m_cache.query( typ, dmn, callback, this );
         }
      }
      else
      {
         EString msg;
         msg.format( "CacheRefresher::loadQueries() - unable to open [%s]", qfn );
         throw EError( EError::Warning, msg );
      }
   }

   /// @endcond

   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////

} // namespace DNS
