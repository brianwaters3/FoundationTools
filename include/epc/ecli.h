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

#ifndef __ECLI_H
#define __ECLI_H

#include <iostream>
#include <pistache/endpoint.h>
#include <pistache/http_header.h>
#include <pistache/router.h>

#define RAPIDJSON_NAMESPACE epctoolsrapidjson

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "elogger.h"
#include "estring.h"
#include "etime.h"

class ECliUserNameHeader : public Pistache::Http::Header::Header
{
public:
   NAME("X-User-Name")

   ECliUserNameHeader()
   {
   }

   void parse(const std::string &data)
   {
      m_username = data;
   }

   void write(std::ostream &os) const
   {
      os << m_username;
   }

   EString &getUserName() { return m_username; }
   EString &setUserName(cpStr username) { return m_username = username; }
   EString &setUserName(std::string &username) { return m_username = username; }

private:
   EString m_username;
};

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

   ECliHandler(HttpMethod mthd, cpStr pth, ELogger &audit);
   ECliHandler(HttpMethod mthd, const std::string &pth, ELogger &audit);

   virtual Void process(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter &response) = 0;

   Void handler(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response);

   const EString &path() { return m_path; }
   HttpMethod httpMethod() { return m_method; }

protected:
   Pistache::Rest::Route::Handler getHandler();

private:
   ECliHandler();

   ELogger &m_audit;
   EString m_path;
   HttpMethod m_method;
};

class ECliEndpoint
{
public:
   ECliEndpoint(uint16_t port, size_t thrds=1);
   ECliEndpoint(Pistache::Address &addr, size_t thrds=1);

   Void start();
   Void shutdown();

   Void registerHandler(ECliHandler &hndlr);

private:
   Void init(size_t thrds);

   std::shared_ptr<Pistache::Http::Endpoint> m_endpoint;
   Pistache::Rest::Router m_router;

   static Bool m_username_header_registered;
};

#endif // #ifndef __ECLI_H