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

#ifndef __DNSQUERY_H
#define __DNSQUERY_H

/// @file
/// @brief Contains the definition of the DNS query related classes.

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <list>

#include "estring.h"
#include "esynch.h"
#include "dnsrecord.h"

namespace DNS
{
   class Cache;
   class Query;
   class QueryProcessor;
   class QueryProcessorThread;
   class QueryCacheKey;

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE
   typedef std::shared_ptr<Query> QueryPtr;
   typedef std::map<QueryCacheKey, QueryPtr> QueryCache;
   extern "C" typedef Void(*CachedDNSQueryCallback)(QueryPtr q, Bool cacheHit, const Void *data);
   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE
   class QueryCacheKey
   {
   public:
      QueryCacheKey( ns_type rtype, const std::string &domain )
         : m_type( rtype ),
           m_domain( domain )
      {
      }

      QueryCacheKey( const QueryCacheKey &other )
      {
         m_type = other.m_type;
         m_domain = other.m_domain;
      }

      const QueryCacheKey& operator=( const QueryCacheKey &r )
      {
         m_type = r.m_type;
         m_domain = r.m_domain;
         return *this;
      }

      Bool operator<( const QueryCacheKey &r ) const
      {
         return
            this->m_type < r.m_type ? true :
            this->m_type > r.m_type ? false :
            this->m_domain < r.m_domain ? true : false;
      }

      const ns_type getType() { return m_type; }
      const EString &getDomain() { return m_domain; }

   private:
      ns_type m_type;
      EString m_domain;
   };
   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Defines a DNS query.
   class Query
   {
      friend Cache;
      friend QueryProcessor;
      friend QueryProcessorThread;

   public:
      /// @brief Class constructor.
      /// @param rtype the named server type for the query.
      /// @param domain the domain for the query.
      Query( ns_type rtype, const std::string &domain )
         : m_qp( NULL ),
           m_cb( NULL ),
           m_event( NULL ),
           m_data( NULL ),
           m_type( rtype ),
           m_domain( domain ),
           m_ttl( UINT32_MAX ),
           m_expires( LONG_MAX ),
           m_ignorecache( false )
      {
      }
      /// @brief Class destructor.
      ~Query()
      {
      }
   
      /// @brief Adds a question record to a query results.
      /// @param q a pointer to the question record to add.
      Void addQuestion( Question *q )
      {
         if ( q )
            m_question.push_back( q );
      }
      /// @brief Adds an answer record to the query results.
      /// @param a a pointer to the answer record to add.
      Void addAnswer( ResourceRecord *a )
      {
         if ( a )
         {
            if ( m_expires == 0 || a->getTTL() != 0 )
            {
               if (a->getTTL() != 0)
               {
                  if (a->getExpires() < m_expires)
                     m_expires = a->getExpires();
                  if (a->getTTL() < m_ttl)
                     m_ttl = a->getTTL();
               }
            }
            m_answer.push_back( a );
         }
      }
      /// @brief Adds an authority record to the query results.
      /// @param a a pointer to the authority record to add.
      Void addAuthority( ResourceRecord *a )
      {
         if ( a )
         {
            if ( m_expires == 0 || a->getTTL() != 0 )
            {
               if (a->getTTL() != 0)
               {
                  if (a->getExpires() < m_expires)
                     m_expires = a->getExpires();
                  if (a->getTTL() < m_ttl)
                     m_ttl = a->getTTL();
               }
            }
            m_authority.push_back( a );
         }
      }
      /// @brief Adds an additional record to the query results.
      /// @param a a pointer to the additional record to add.
      Void addAdditional( ResourceRecord *a )
      {
         if ( a )
         {
            if ( m_expires == 0 || a->getTTL() != 0 )
            {
               if (a->getTTL() != 0)
               {
                  if (a->getExpires() < m_expires)
                     m_expires = a->getExpires();
                  if (a->getTTL() < m_ttl)
                     m_ttl = a->getTTL();
               }
            }
            m_additional.push_back( a );
         }
      }

      /// @brief Retrieves the named service type associated with this query.
      /// @return the named service type associated with this query.
      ns_type getType() { return m_type; }
      /// @brief Retrieves the domain associated with this query.
      /// @return a reference to the domain.
      const EString &getDomain() { return m_domain; }

      /// @brief Retrieves the time to live (TTL) value associated with the query results.
      /// @return the time to live (TTL) value associated with the query results.
      uint32_t getTTL() { return m_ttl; }
      /// @brief retrieves the expiration time associated with the query results.
      /// @return the expiration time associated with the query results.
      time_t getExpires() { return m_expires; }
      /// @brief Retrieves an indication if the query results have expired.
      /// @return True indicates the query results have expired, otherwise False.
      Bool isExpired() { return time(NULL) >= m_expires; }
      /// @brief Retrieves an indication if the DNS cache for this query should be ignored.
      /// @return an indication if the DNS cache for this query should be ignored.
      Bool ignoreCache() { return m_ignorecache; }

      /// @brief Retrieves the collection of query result question records.
      /// @return a reference to the collection of query result question records.
      const std::list<Question*> &getQuestions() { return m_question; }
      /// @brief Retrieves the collection of query result answer records.
      /// @return a reference to the collection of query result answer records.
      const ResourceRecordList &getAnswers() { return m_answer; }
      /// @brief Retrieves the collection of query result authority records.
      /// @return a reference to the collection of query result authority records.
      const ResourceRecordList &getAuthorities() { return m_authority; }
      /// @brief Retrieves the collection of query result additional records.
      /// @return a reference to the collection of query result additional records.
      const ResourceRecordList &getAdditional() { return m_additional; }

      /// @brief Prints the information associated with this DNS query object.
      Void dump()
      {
         std::cout << "QUERY type=" << getType() << " domain=" << getDomain() << std::endl;
         std::cout << "QUESTION:" << std::endl;
         for (QuestionList::const_iterator it = getQuestions().begin();
              it != getQuestions().end();
              ++it )
         {
            (*it)->dump();
         }

         std::cout << "ANSWER:" << std::endl;
         for (ResourceRecordList::const_iterator it = getAnswers().begin();
              it != getAnswers().end();
              ++it )
         {
            (*it)->dump();
         }

         std::cout << "AUTHORITY:" << std::endl;
         for (ResourceRecordList::const_iterator it = getAuthorities().begin();
              it != getAuthorities().end();
              ++it )
         {
            (*it)->dump();
         }

         std::cout << "ADDITIONAL:" << std::endl;
         for (ResourceRecordList::const_iterator it = getAdditional().begin();
              it != getAdditional().end();
              ++it )
         {
            (*it)->dump();
         }
      }

      /// @brief Retrieves the completion event associated with this query.
      /// @return a pointer to the completion event associated with this query.
      EEvent *getCompletionEvent() { return m_event; }
      /// @brief Assigns the query completion event.
      /// @return a pointer to the query completion event.
      EEvent *setCompletionEvent(EEvent *event) { return m_event = event; }

      /// @brief Retrieves the query completion callback function.
      /// @return the query completion callback function.
      CachedDNSQueryCallback getCallback() { return m_cb; }
      /// @brief Assigns the query completion callback function for this DNS query.
      /// @return the query completion callback function for this DNS query.
      CachedDNSQueryCallback setCallback(CachedDNSQueryCallback cb) { return m_cb = cb; }

      /// @brief Retrieves the error indication.
      /// @return True indicats an error occurred, otherwise False.
      Bool getError() { return m_err; }
      /// @brief Assigns the error indication.
      /// @param err the error indication.
      /// @return the assigned error indication.
      Bool setError(Bool err) { return m_err = err; }

      /// @brief Retrieves the error message associated with this DNS query.
      /// @return the error message associated with this DNS query.
      EString &getErrorMsg() { return m_errmsg; }
      /// @brief Assigns the error message associated with this DNS query.
      /// @param errmsg the error message.
      /// @return the error message associated with this DNS query.
      EString &setErrorMsg(const char *errmsg) { return m_errmsg = errmsg; }
      /// @copydoc setErrorMsg(const char *)
      EString &setErrorMsg(const std::string &errmsg) { return m_errmsg = errmsg; }

      /// @brief Retrieves the current ignore cache value.
      /// @return the current ignore cache value.
      Bool getIgnoreCache() { return m_ignorecache; }
      /// @brief Assigns the ignore cache value.
      /// @param ignorecache the ignore cache value.
      /// @return the ignore cache value.
      Bool setIgnoreCache(Bool ignorecache) { return m_ignorecache = ignorecache; }

   protected:
      /// @cond DOXYGEN_EXCLUDE
      QueryProcessor *getQueryProcessor() { return m_qp; }
      QueryProcessor *setQueryProcessor(QueryProcessor *qp) { return m_qp = qp; }

      const Void *getData() { return m_data; }
      const Void *setData(const Void *data) { return m_data = data; }
      /// @endcond

   private:
      QueryProcessor *m_qp;
      CachedDNSQueryCallback m_cb;
      EEvent *m_event;
      const Void *m_data;

      ns_type m_type;
      EString m_domain;
      QuestionList m_question;
      ResourceRecordList m_answer;
      ResourceRecordList m_authority;
      ResourceRecordList m_additional;
      uint32_t m_ttl;
      time_t m_expires;
      Bool m_ignorecache;

      Bool m_err;
      EString m_errmsg;
   };
}

#endif // #ifndef __DNSQUERY_H
