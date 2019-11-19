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

#include "emgmt.h"


EManagementHandler::EManagementHandler(HttpMethod mthd, cpStr pth, ELogger &audit)
   : m_path(pth),
      m_method(mthd),
      m_audit(audit)
{
}

EManagementHandler::EManagementHandler(HttpMethod mthd, const std::string &pth, ELogger &audit)
   : m_path(pth),
      m_method(mthd),
      m_audit(audit)
{
}

Void EManagementHandler::handler(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response)
{
   try
   {
      std::stringstream ss;
      auto headers = request.headers();
      auto username = headers.get<EManagementUserNameHeader>();

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

Pistache::Rest::Route::Handler EManagementHandler::getHandler()
{
   return Pistache::Rest::Routes::bind( &EManagementHandler::handler, this );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Bool EManagementEndpoint::m_username_header_registered = False;

EManagementEndpoint::EManagementEndpoint(uint16_t port, size_t thrds)
   : m_endpoint( std::make_shared<Pistache::Http::Endpoint>(
      Pistache::Address(Pistache::Ipv4::any(), Pistache::Port(port))) )
{
   init(thrds);
}

EManagementEndpoint::EManagementEndpoint(Pistache::Address &addr, size_t thrds)
   : m_endpoint( std::make_shared<Pistache::Http::Endpoint>(addr) )
{
   init(thrds);
}

Void EManagementEndpoint::start()
{
   if (!m_username_header_registered)
   {
      Pistache::Http::Header::Registry::registerHeader<EManagementUserNameHeader>();
      m_username_header_registered = True;
   }

   m_endpoint->setHandler(m_router.handler());
   m_endpoint->serveThreaded();
}

Void EManagementEndpoint::shutdown()
{
   m_endpoint->shutdown();
}

Void EManagementEndpoint::registerHandler(EManagementHandler &hndlr)
{
   switch (hndlr.httpMethod())
   {
      case EManagementHandler::HttpMethod::httpGet:     { Pistache::Rest::Routes::Get(m_router, hndlr.path(), hndlr.getHandler());    break; }
      case EManagementHandler::HttpMethod::httpPost:    { Pistache::Rest::Routes::Post(m_router, hndlr.path(), hndlr.getHandler());   break; }
      case EManagementHandler::HttpMethod::httpPut:     { Pistache::Rest::Routes::Put(m_router, hndlr.path(), hndlr.getHandler());    break; }
      case EManagementHandler::HttpMethod::httpDelete:  { Pistache::Rest::Routes::Delete(m_router, hndlr.path(), hndlr.getHandler()); break; }
      case EManagementHandler::HttpMethod::httpPatch:   { Pistache::Rest::Routes::Patch(m_router, hndlr.path(), hndlr.getHandler());  break; }
      case EManagementHandler::HttpMethod::httpOptions: { Pistache::Rest::Routes::Patch(m_router, hndlr.path(), hndlr.getHandler());  break; }
   }
}

Void EManagementEndpoint::init(size_t thrds)
{
   auto opts = Pistache::Http::Endpoint::options()
      .threads(thrds)
      .flags(Pistache::Tcp::Options::ReuseAddr);
   m_endpoint->init(opts);
}
