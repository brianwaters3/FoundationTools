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

   typedef std::shared_ptr<Query> QueryPtr;
   typedef std::map<QueryCacheKey, QueryPtr> QueryCache;
   extern "C" typedef void(*CachedDNSQueryCallback)(QueryPtr q, bool cacheHit, const void *data);

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

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

      bool operator<( const QueryCacheKey &r ) const
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

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class Query
   {
      friend Cache;
      friend QueryProcessor;
      friend QueryProcessorThread;

   public:
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

      ~Query()
      {
      }
   
      void addQuestion( Question *q )
      {
         if ( q )
            m_question.push_back( q );
      }

      void addAnswer( ResourceRecord *a )
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

      void addAuthority( ResourceRecord *a )
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

      void addAdditional( ResourceRecord *a )
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

      ns_type getType() { return m_type; }
      const EString &getDomain() { return m_domain; }

      uint32_t getTTL() { return m_ttl; }
      time_t getExpires() { return m_expires; }
      bool isExpired() { return time(NULL) >= m_expires; }
      bool ignoreCache() { return m_ignorecache; }

      const std::list<Question*> &getQuestions() { return m_question; }
      const ResourceRecordList &getAnswers() { return m_answer; }
      const ResourceRecordList &getAuthorities() { return m_authority; }
      const ResourceRecordList &getAdditional() { return m_additional; }

      void dump()
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

      EEvent *getCompletionEvent() { return m_event; }
      EEvent *setCompletionEvent(EEvent *event) { return m_event = event; }

      CachedDNSQueryCallback getCallback() { return m_cb; }
      CachedDNSQueryCallback setCallback(CachedDNSQueryCallback cb) { return m_cb = cb; }

      bool getError() { return m_err; }
      bool setError(bool err) { return m_err = err; }

      EString &getErrorMsg() { return m_errmsg; }
      EString &setErrorMsg(const char *errmsg) { return m_errmsg = errmsg; }
      EString &setErrorMsg(const std::string &errmsg) { return m_errmsg = errmsg; }

      bool getIgnoreCache() { return m_ignorecache; }
      bool setIgnoreCache(bool ignorecache) { return m_ignorecache = ignorecache; }

   protected:
      QueryProcessor *getQueryProcessor() { return m_qp; }
      QueryProcessor *setQueryProcessor(QueryProcessor *qp) { return m_qp = qp; }

      const void *getData() { return m_data; }
      const void *setData(const void *data) { return m_data = data; }

   private:
      QueryProcessor *m_qp;
      CachedDNSQueryCallback m_cb;
      EEvent *m_event;
      const void *m_data;

      ns_type m_type;
      EString m_domain;
      QuestionList m_question;
      ResourceRecordList m_answer;
      ResourceRecordList m_authority;
      ResourceRecordList m_additional;
      uint32_t m_ttl;
      time_t m_expires;
      bool m_ignorecache;

      bool m_err;
      EString m_errmsg;
   };
}

#endif // #ifndef __DNSQUERY_H
