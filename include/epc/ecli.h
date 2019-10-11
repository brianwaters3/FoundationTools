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

#ifndef __ECLI_H
#define __ECLI_H

#include <iostream>
#include <pistache/endpoint.h>
#include <pistache/router.h>

#define RAPIDJSON_NAMESPACE epctoolsrapidjson

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
// #include "rapidjson/writer.h"
// #include "rapidjson/prettywriter.h"
// #include "rapidjson/stringbuffer.h"

#include "elogger.h"
#include "estring.h"
#include "etime.h"

class ECliHandler
{
   friend class ECliEndpoint;

public:
   enum class HttpMethod
   {
      httpGet,
      httpPost,
      httpPut,
      httpDelete,
      httpPatch,
      httpOptions
   };

   ECliHandler(HttpMethod mthd, cpStr pth, ELogger &audit)
      : m_path(pth),
        m_method(mthd),
        m_audit(audit)
   {
   }

   ECliHandler(HttpMethod mthd, const std::string &pth, ELogger &audit)
      : m_path(pth),
        m_method(mthd),
        m_audit(audit)
   {
   }

   virtual Void process(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter &response) = 0;

   Void handler(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response)
   {
      std::stringstream ss;
      RAPIDJSON_NAMESPACE::Document doc;
      
      doc.Parse( request.body().c_str() );
      if ( doc.HasParseError() )
      {
         response.send( Pistache::Http::Code::Bad_Request, "{\"result\": \"ERROR\"}" );
         return;
      }

      if ( !doc.HasMember("username") || !doc["username"].IsString() )
      {
         response.send( Pistache::Http::Code::Bad_Request, "{\"result\": \"ERROR\"}" );
         return;
      }

      ss << ETime::Now().Format("%Y-%m-%dT%H:%M:%S.%0", False)
         << ","
         << doc["username"].GetString()
         << ","
         << request.method()
         << ","
         << request.resource()
         << ","
         << request.body();

      m_audit.info( ss.str().c_str() );

      process( request, response );
   }

   const EString &path() { return m_path; }
   HttpMethod httpMethod() { return m_method; }

protected:
   Pistache::Rest::Route::Handler getHandler()
   {
      return Pistache::Rest::Routes::bind( &ECliHandler::handler, this );
   }

private:
   ECliHandler();

   ELogger &m_audit;
   EString m_path;
   HttpMethod m_method;
};

class ECliEndpoint
{
public:
   ECliEndpoint(uint16_t port, ELogger &auditLogger, size_t thrds=1)
      : m_endpoint( std::make_shared<Pistache::Http::Endpoint>(
         Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(port))) )
   {
      init(thrds);
   }

   ECliEndpoint(Pistache::Address &addr, ELogger &auditLogger, size_t thrds=1)
      : m_endpoint( std::make_shared<Pistache::Http::Endpoint>(addr) )
   {
      init(thrds);
   }

   Void start()
   {
      m_endpoint->setHandler(m_router.handler());
      m_endpoint->serveThreaded();
   }

   Void shutdown()
   {
      m_endpoint->shutdown();
   }

   Void registerHandler(ECliHandler &hndlr)
   {
      switch (hndlr.httpMethod())
      {
         case ECliHandler::HttpMethod::httpGet:     { Pistache::Rest::Routes::Get(m_router, hndlr.path(), hndlr.getHandler());    break; }
         case ECliHandler::HttpMethod::httpPost:    { Pistache::Rest::Routes::Post(m_router, hndlr.path(), hndlr.getHandler());   break; }
         case ECliHandler::HttpMethod::httpPut:     { Pistache::Rest::Routes::Put(m_router, hndlr.path(), hndlr.getHandler());    break; }
         case ECliHandler::HttpMethod::httpDelete:  { Pistache::Rest::Routes::Delete(m_router, hndlr.path(), hndlr.getHandler()); break; }
         case ECliHandler::HttpMethod::httpPatch:   { Pistache::Rest::Routes::Patch(m_router, hndlr.path(), hndlr.getHandler());  break; }
         case ECliHandler::HttpMethod::httpOptions: { Pistache::Rest::Routes::Patch(m_router, hndlr.path(), hndlr.getHandler());  break; }
      }
   }

private:
   Void init(size_t thrds)
   {
      auto opts = Pistache::Http::Endpoint::options()
         .threads(thrds)
         .flags(Pistache::Tcp::Options::ReuseAddr);
      m_endpoint->init(opts);
   }

   std::shared_ptr<Pistache::Http::Endpoint> m_endpoint;
   Pistache::Rest::Router m_router;
};

#endif // #ifndef __ECLI_H