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

#include "ecli.h"


ECliHandler::ECliHandler(HttpMethod mthd, cpStr pth, ELogger &audit)
   : m_path(pth),
      m_method(mthd),
      m_audit(audit)
{
}

ECliHandler::ECliHandler(HttpMethod mthd, const std::string &pth, ELogger &audit)
   : m_path(pth),
      m_method(mthd),
      m_audit(audit)
{
}

Void ECliHandler::handler(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response)
{
   try
   {
      std::stringstream ss;
      auto headers = request.headers();
      auto username = headers.get<ECliUserNameHeader>();

      ss << ETime::Now().Format("%Y-%m-%dT%H:%M:%S.%0", False)
         << ","
         << username->getUserName()
         << ","
         << request.method()
         << ","
         << request.resource()
         << ","
         << request.body();

      m_audit.info( ss.str().c_str() );
      m_audit.flush();

      process( request, response );
   }
   catch(...)
   {
      response.send( Pistache::Http::Code::Bad_Request, "{\"result\": \"ERROR\"}" );
   }
}

Pistache::Rest::Route::Handler ECliHandler::getHandler()
{
   return Pistache::Rest::Routes::bind( &ECliHandler::handler, this );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Bool ECliEndpoint::m_username_header_registered = False;

ECliEndpoint::ECliEndpoint(uint16_t port, size_t thrds)
   : m_endpoint( std::make_shared<Pistache::Http::Endpoint>(
      Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(port))) )
{
   init(thrds);
}

ECliEndpoint::ECliEndpoint(Pistache::Address &addr, size_t thrds)
   : m_endpoint( std::make_shared<Pistache::Http::Endpoint>(addr) )
{
   init(thrds);
}

Void ECliEndpoint::start()
{
   if (!m_username_header_registered)
   {
      Pistache::Http::Header::Registry::registerHeader<ECliUserNameHeader>();
      m_username_header_registered = True;
   }

   m_endpoint->setHandler(m_router.handler());
   m_endpoint->serveThreaded();
}

Void ECliEndpoint::shutdown()
{
   m_endpoint->shutdown();
}

Void ECliEndpoint::registerHandler(ECliHandler &hndlr)
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

Void ECliEndpoint::init(size_t thrds)
{
   auto opts = Pistache::Http::Endpoint::options()
      .threads(thrds)
      .flags(Pistache::Tcp::Options::ReuseAddr);
   m_endpoint->init(opts);
}
