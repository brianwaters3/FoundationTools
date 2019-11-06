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

#ifndef __DNSRECORD_H
#define __DNSRECORD_H

/// @file
/// @brief Defines the classes related to DNS records.

#include <list>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <memory.h>
#include <time.h>

#include "estring.h"

namespace DNS
{
   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents the question for the name server.
   class Question
   {
   public:
      /// @brief Class constructor.
      /// @param qname the query name of the requested resource.
      /// @param qtype the query type for the requested resource.
      /// @param qclass specifies the class of the query.
      Question( const std::string& qname, ns_type qtype, ns_class qclass )
         : m_qname( qname ),
           m_qtype( qtype ),
           m_qclass( qclass )
      {
      }

      /// @brief Retrieves the query name of the requested resource.
      /// @return the query name of the requested resource.
      EString &getQName() { return m_qname; }
      /// @brief Retrieves the query type for the requested resource.
      /// @return the query type for the requested resource.
      ns_type getQType() { return m_qtype; }
      /// @brief Retrieves the class of the query.
      /// @return the class of the query.
      ns_class getQClass() { return m_qclass; }

      /// @brief Prints the contents the stdout.
      Void dump()
      {
         std::cout << "Question:"
            << " qtype=" << m_qtype
            << " qclass=" << m_qclass
            << " qname=" << m_qname
            << std::endl;
      }

   private:
      Question() {}

      EString m_qname;
      ns_type m_qtype;
      ns_class m_qclass;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE
   class QuestionList : public std::list<Question*>
   {
   public:
      QuestionList() {}
      ~QuestionList()
      {
         while ( !empty() )
         {
            Question *q = *begin();
            erase( begin() );
            delete q;
         }
      }
   };
   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents a DNS Resource Record.
   class ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param rtype specifies the meaning of the data in the RDATA field.
      /// @param rclass the class of the data in the RDATA field.
      /// @param ttl specifies the time interval (in seconds) that the resource
      ///   record may be cached before it should be discarded.
      ResourceRecord( const std::string &name,
                      ns_type rtype,
                      ns_class rclass,
                      int32_t ttl )
         : m_name( name ),
           m_type( rtype ),
           m_class( rclass ),
           m_ttl( ttl ),
           m_expires( time(NULL) + ttl )
      {
      }
      /// @brief Class destructor.
      virtual ~ResourceRecord() {}

      /// @brief Retrieves the domain name to which this resource record pertains.
      /// @return the domain name to which this resource record pertains.
      const EString &getName() { return m_name; }
      /// @brief Retrieves the resource type.
      /// @return the resource type.
      ns_type getType() { return m_type; }
      /// @brief Retrieves the class of the data in the RDATA field.
      /// @return the class of the data in the RDATA field.
      ns_class getClass() { return m_class; }
      /// @brief Retrieves the time interval (in seconds) that the resource
      ///   record may be cached before it should be discarded.
      /// @return the time interval (in seconds) that the resource
      ///   record may be cached before it should be discarded.
      uint32_t getTTL() { return m_ttl; }
      /// @brief Retrieves the expiration time of this resource record.
      /// @return the expiration time of this resource record.
      time_t getExpires() { return m_expires; }

      /// @brief Determines if this resource record has expired.
      /// @return True indicates that this resource record is expired, otherwise False.
      Bool isExpired() { return m_expires <= time(NULL); }

      /// @brief Prints the contents of this resource record.
      virtual Void dump()
      {
         std::cout << "ResourceRecord:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      EString m_name;
      ns_type m_type;
      ns_class m_class;
      int32_t m_ttl;
      time_t m_expires;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE
   class ResourceRecordList : public std::list<ResourceRecord*>
   {
   public:
      ResourceRecordList() {}
      ~ResourceRecordList()
      {
         while ( !empty() )
         {
            ResourceRecord *rr = *begin();
            erase( begin() );
            delete rr;
         }
      }
   };
   /// @endcond

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents an A resource record.
   class RRecordA : public ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param ttl the time to live value.
      /// @param address the IPv4 address associated with this server.
      RRecordA( const std::string &name,
                int32_t ttl,
                const struct in_addr &address )
         : ResourceRecord(name, ns_t_a, ns_c_in, ttl)
      {
         memcpy( &m_address, &address, sizeof(m_address) );
      }

      /// @brief Retrieves the IP address for this A record.
      /// @return the IP address for this A record.
      const struct in_addr &getAddress() { return m_address; }

      /// @brief Convert the IPv4 address to a string.
      /// @return the string representation of the IPv4 address.
      EString getAddressString()
      {
         char address[ INET_ADDRSTRLEN ];
         inet_ntop( AF_INET, &m_address, address, sizeof(address) );
         return EString( address );
      }

      /// @brief Prints the contents fo this A record.
      virtual Void dump()
      {
         std::cout << "RRecordA:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " address=" << getAddressString()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      struct in_addr m_address;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents an NS resource record.
   class RRecordNS : public ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param ttl the time to live value.
      /// @param ns the named server for this NS record.
      RRecordNS( const std::string &name,
                    int32_t ttl,
                    const std::string &ns )
         : ResourceRecord( name, ns_t_cname, ns_c_in, ttl ),
           m_namedserver( ns )
      {
      }

      const EString &getNamedServer() { return m_namedserver; }

      /// @brief Prints the contents fo this NS record.
      virtual Void dump()
      {
         std::cout << "RRecordNS:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " ns=" << getNamedServer()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      EString m_namedserver;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents an AAAA resource record.
   class RRecordAAAA : public ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param ttl the time to live value.
      /// @param address the IPv6 address associated with this server.
      RRecordAAAA( const std::string &name,
                   int32_t ttl,
                   const struct in6_addr &address )
         : ResourceRecord( name, ns_t_aaaa, ns_c_in, ttl )
      {
         memcpy( &m_address, &address, sizeof(m_address) );
      }

      const struct in6_addr &getAddress() { return m_address; }

      EString getAddressString()
      {
         char address[ INET6_ADDRSTRLEN ];
         inet_ntop( AF_INET6, &m_address, address, sizeof(address) );
         return EString( address );
      }

      /// @brief Prints the contents fo this AAAA record.
      virtual Void dump()
      {
         std::cout << "RRecordAAAA:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " address=" << getAddressString()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      struct in6_addr m_address;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents an CNAME resource record.
   class RRecordCNAME : public ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param ttl the time to live value.
      /// @param alias the alias.
      RRecordCNAME( const std::string &name,
                    int32_t ttl,
                    const std::string &alias )
         : ResourceRecord( name, ns_t_cname, ns_c_in, ttl ),
           m_alias( alias )
      {
      }

      const EString &getAlias() { return m_alias; }

      /// @brief Prints the contents fo this CNAME record.
      virtual Void dump()
      {
         std::cout << "RRecordCNAME:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " alias=" << getAlias()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      EString m_alias;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents an SRV resource record.
   class RRecordSRV : public ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param ttl the time to live value.
      /// @param priority the priority of this target host.
      /// @param weight the relative weight for entries with the same priority.
      /// @param port the port on this target host of this service.
      /// @param target the domain name of the target host.
      RRecordSRV( const std::string &name,
                  int32_t ttl,
                  uint16_t priority,
                  uint16_t weight,
                  uint16_t port,
                  const std::string &target)
         : ResourceRecord( name, ns_t_srv, ns_c_in, ttl ),
           m_priority( priority ),
           m_weight( weight ),
           m_port( port ),
           m_target( target )
      {
      }

      /// @brief Retrieves the priority of this target host.
      /// @return the priority of this target host.
      uint16_t getPriority() { return m_priority; }
      /// @brief Retrieves 
      /// @return 
      uint16_t getWeight() { return m_weight; }
      /// @brief Retrieves the relative weight for entries with the same priority.
      /// @return the relative weight for entries with the same priority.
      uint16_t getPort() { return m_port; }
      /// @brief The domain name of the target host.
      /// @return the domain name of the target host.
      const EString &getTarget() { return m_target; }

      /// @brief Prints the contents fo this SRV record.
      virtual Void dump()
      {
         std::cout << "RRecordSRV:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " priority=" << getPriority()
            << " weight=" << getWeight()
            << " port=" << getPort()
            << " target=" << getTarget()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      uint16_t m_priority;
      uint16_t m_weight;
      uint16_t m_port;
      EString m_target;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Represents an NAPTR resource record.
   class RRecordNAPTR : public ResourceRecord
   {
   public:
      /// @brief Class constructor.
      /// @param name the domain name to which this resource record pertains.
      /// @param ttl the time to live value.
      /// @param order the order in which the NAPTR records MUST be processed in
      ///   order to accurately represent the ordered list of Rules.
      /// @param preference the order in which NAPTR records with equal Order
      ///   values SHOULD be processed, low numbers being processed before high numbers.
      /// @param flags controls aspects of the rewriting and interpretation of
      ///   the fields in the record.
      /// @param service the Service Parameters applicable to this this
      ///   delegation path.
      /// @param regexp a substitution expression that is applied to the original
      ///   string held by the client in order to construct the next domain name
      ///   to lookup.
      /// @param replacement this field is used when the regular expression is a
      ///   simple replacement operation.
      RRecordNAPTR( const std::string &name,
                    int32_t ttl,
                    uint16_t order,
                    uint16_t preference,
                    const std::string &flags,
                    const std::string &service,
                    const std::string &regexp,
                    const std::string &replacement )
         : ResourceRecord( name, ns_t_naptr, ns_c_in, ttl ),
           m_order( order ),
           m_preference( preference ),
           m_flags( flags ),
           m_service( service ),
           m_regexp( regexp ),
           m_replacement( replacement )
      {
      }

      /// @brief Retrieves the order in which the NAPTR records MUST be processed in
      ///   order to accurately represent the ordered list of Rules.
      /// @return the order in which the NAPTR records MUST be processed in
      ///   order to accurately represent the ordered list of Rules.
      const uint16_t getOrder() const { return m_order; }
      /// @brief Retrieves he order in which NAPTR records with equal Order
      ///   values SHOULD be processed, low numbers being processed before high numbers.
      /// @return he order in which NAPTR records with equal Order
      ///   values SHOULD be processed, low numbers being processed before high numbers.
      const uint16_t getPreference() const { return m_preference; }
      /// @brief Retrieves the flags associated with this NAPTR record.
      /// @return the flags associated with this NAPTR record.
      const EString &getFlags() { return m_flags; }
      /// @brief Retrieves the Service Parameters applicable to this this
      ///   delegation path.
      /// @return the Service Parameters applicable to this this delegation path.
      const EString &getService() { return m_service; }
      /// @brief Retrieves the regular expression associated with this NAPTR record.
      /// @return the regular expression associated with this NAPTR record.
      const EString &getRegexp() { return m_regexp; }
      /// @brief Retrieves the replacement string associated with this NAPTR record.
      /// @return the replacement string associated with this NAPTR record.
      const EString &getReplacement() { return m_replacement; }

      /// @brief Prints the contents fo this NAPTR record.
      virtual Void dump()
      {
         std::cout << "RRecordNAPTR:"
            << " type=" << getType()
            << " class=" << getClass()
            << " ttl=" << getTTL()
            << " expires=" << getExpires()
            << " order=" << getOrder()
            << " preference=" << getPreference()
            << " flags=" << getFlags()
            << " service=" << getService()
            << " regexp=" << getRegexp()
            << " replacement=" << getReplacement()
            << " name=" << getName()
            << std::endl;
      }
   
   private:
      uint16_t m_order;
      uint16_t m_preference;
      EString m_flags;
      EString m_service;
      EString m_regexp;
      EString m_replacement;
   };
}

#endif // #ifdef __DNSRECORD_H
