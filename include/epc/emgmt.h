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

#ifndef __EMGMT_H
#define __EMGMT_H

/// @file
/// @brief Classes used for implementing a REST based command line interface.

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

/// @brief Custom HTTP header class for the X-User-Name header.
class EManagementUserNameHeader : public Pistache::Http::Header::Header
{
public:
   NAME("X-User-Name")

   /// @brief Class constructor.
   EManagementUserNameHeader()
   {
   }

   /// @brief Parses the user name from the buffer.
   /// @param data A string object containing the header data to parse.
   void parse(const std::string &data)
   {
      m_username = data;
   }

   /// @brief Serializes the user name to the output stream.
   /// @param os The output stream to write the data to.
   void write(std::ostream &os) const
   {
      os << m_username;
   }

   /// @brief Returns the value of the user name.
   EString &getUserName() { return m_username; }

private:
   EString m_username;
};

/// @brief Pure virtual base class for an administrative management interface handler.
class EManagementHandler
{
   friend class EManagementEndpoint;

public:
   /// @brief Represents the type of the handler.
   enum class HttpMethod
   {
      /// HTTP GET
      httpGet,
      /// HTTP POST
      httpPost,
      /// HTTP PUT
      httpPut,
      /// HTTP DELETE
      httpDelete,
      /// @cond DOXYGEN_EXCLUDE
      httpPatch,
      httpOptions
      /// @endcond
   };

   /// @brief Class constructor.
   /// @param mthd HTTP method associated with this handler.
   /// @param pth the HTTP route for this handler.
   /// @param audit a reference to the ELogger object that will log all management operations.
   EManagementHandler(HttpMethod mthd, cpStr pth, ELogger &audit);
   /// @brief Class constructor.
   /// @param mthd HTTP method associated with this handler.
   /// @param pth the HTTP route for this handler.
   /// @param audit a reference to the ELogger object that will log all management operations.
   EManagementHandler(HttpMethod mthd, const std::string &pth, ELogger &audit);

   /// @brief Pure virtual method that will be called by handler() to perform the processing.
   /// @param request HTTP request object.
   /// @param response HTTP response object.
   virtual Void process(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter &response) = 0;

   /// @brief HTTP handler that will be called by the Pistache framework.
   /// @param request HTTP request object.
   /// @param response HTTP response object.
   Void handler(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response);

   /// @brief Returns the route path for this HTTP handler.
   const EString &path() { return m_path; }
   /// @brief Returns the HTTP method for this HTTP handler.
   HttpMethod httpMethod() { return m_method; }

protected:
   Pistache::Rest::Route::Handler getHandler();

private:
   EManagementHandler();

   ELogger &m_audit;
   EString m_path;
   HttpMethod m_method;
};

/// @brief Implemts the HTTP server endpoint.
class EManagementEndpoint
{
public:
   /// @brief Class constructor.
   /// @param port the IP port to listen for requests on (all IP addresses).
   /// @param thrds the number of threads that will process requests.
   EManagementEndpoint(uint16_t port, size_t thrds=1);
   /// @brief Class constructor.
   /// @param addr the IP Address to listen for requests on (all IP addresses).
   /// @param thrds the number of threads that will process requests.
   EManagementEndpoint(Pistache::Address &addr, size_t thrds=1);

   /// @brief Starts the endpoint.
   Void start();
   /// @brief Stops and shuts down the endpoint.
   Void shutdown();

   /// @brief Registers a REST handler.
   Void registerHandler(EManagementHandler &hndlr);

private:
   Void init(size_t thrds);

   std::shared_ptr<Pistache::Http::Endpoint> m_endpoint;
   Pistache::Rest::Router m_router;

   static Bool m_username_header_registered;
};

#endif // #ifndef __EMGMT_H