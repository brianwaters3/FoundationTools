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

#ifndef __EPCDNS_H
#define __EPCDNS_H

#include <stdio.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <list>
#include <vector>

#include "estring.h"
#include "dnscache.h"

/*
   MCC digit 1 - low order nibble octet 1
   MCC digit 2 - high order nibble octet 1
   MCC digit 3 - low order nibble octet 2

   MCC = [MCC digit 1][MCC digit 2][MCC digit 3]


   MNC digit 1 - low order nibble octet 3
   MNC digit 2 - high order nibble octet 3
   MNC digit 3 - high order nibble octet 2, nibble set to 1111 if MNC len is 2

   MNC = [MNC digit 1][MNC digit 2][MNC digit 3]
*/

#define LOW_NIBBLE(a) (((unsigned char)a) & 0x0f)
#define HIGH_NIBBLE(a) ((((unsigned char)a) & 0xf0) >> 4)

#define PARSE_MNC(a,b) { \
   if ( HIGH_NIBBLE(b[1]) == 0x0f ) { \
      a[0] = '0'; \
      a[1] = LOW_NIBBLE(b[2]) + '0'; \
      a[2] = HIGH_NIBBLE(b[2]) + '0'; \
      a[3] = '\0'; \
   } else { \
      a[0] = LOW_NIBBLE(b[2]) + '0'; \
      a[1] = HIGH_NIBBLE(b[2]) + '0'; \
      a[2] = HIGH_NIBBLE(b[1]) + '0'; \
      a[3] = '\0'; \
   } \
}

#define PARSE_MCC(a,b) { \
   a[0] = LOW_NIBBLE(b[0]) + '0';  \
   a[1] = HIGH_NIBBLE(b[0]) + '0'; \
   a[2] = LOW_NIBBLE(b[1]) + '0'; \
   a[3] = '\0'; \
}
   
#define PARSE_PLMNID(a) \
   char mnc[4], mcc[4]; \
   PARSE_MNC(mnc,a); \
   PARSE_MCC(mcc,a);

#define APPEND_MNC(a) append("mnc").append(a).append(".")
#define APPEND_MCC(a) append("mcc").append(a).append(".")
#define APPEND_3GPPNETWORK append("3gppnetwork.org")

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace EPCDNS
{
   /*
    * Service and Protocol service names for 3GPP
    * TS 23.003 V15.5.0, Table 19.4.3.1
    */
   enum AppServiceEnum
   {
      x_3gpp_unknown,
      x_3gpp_pgw,
      x_3gpp_sgw,
      x_3gpp_ggsn,
      x_3gpp_sgsn,
      x_3gpp_mme,
      x_3gpp_msc,
      x_3gpp_upf,
      x_3gpp_amf
   };
   
   enum AppProtocolEnum
   {
      x_unknown,
      x_gn,
      x_gp,
      x_n2,
      x_nq,
      x_nqprime,
      x_s1_mme,
      x_s1_u,
      x_s10,
      x_s11,
      x_s12,
      x_s16,
      x_s2a_gtp,
      x_s2a_mipv4,
      x_s2a_pmip,
      x_s2b_gtp,
      x_s2b_pmip,
      x_s2c_dsmip,
      x_s3,
      x_s4,
      x_s5_gtp,
      x_s5_pmip,
      x_s6a,
      x_s8_gtp,
      x_s8_pmip,
      x_sv,
      x_sxa,
      x_sxb,
      x_sxc
   };
   
   enum PGWAppProtocolEnum
   {
      pgw_x_gn,
      pgw_x_gp,
      pgw_x_s2a_gtp,
      pgw_x_s2a_mipv4,
      pgw_x_s2a_pmip,
      pgw_x_s2b_gtp,
      pgw_x_s2b_pmip,
      pgw_x_s2c_dsmip,
      pgw_x_s5_gtp,
      pgw_x_s5_pmip,
      pgw_x_s8_gtp,
      pgw_x_s8_pmip
   };
   
   enum SGWAppProtocolEnum
   {
      sgw_x_s1_u,
      sgw_x_s11,
      sgw_x_s12,
      sgw_x_s2a_pmip,
      sgw_x_s2b_pmip,
      sgw_x_s4,
      sgw_x_s5_gtp,
      sgw_x_s5_pmip,
      sgw_x_s8_gtp,
      sgw_x_s8_pmip
   };
   
   enum GGSNAppProtocolEnum
   {
      ggsn_x_gn,
      ggsn_x_gp
   };
   
   enum SGSNAppProtocolEnum
   {
      sgsn_x_gn,
      sgsn_x_gp,
      sgsn_x_nqprime,
      sgsn_x_s16,
      sgsn_x_s3,
      sgsn_x_s4,
      sgsn_x_sv
   };
   
   enum MMEAppProtocolEnum
   {
      mme_x_gn,
      mme_x_gp,
      mme_x_nq,
      mme_x_s10,
      mme_x_s11,
      mme_x_s1_mme,
      mme_x_s3,
      mme_x_s6a,
      mme_x_sv
   };
   
   enum MSCAppProtocolEnum
   {
      msc_x_sv
   };
   
   enum UPFAppProtocolEnum
   {
      upf_x_sxa,
      upf_x_sxb,
      upf_x_sxc
   };

   enum AMFAppProtocolEnum
   {
      amf_x_n2
   };

   enum DiameterApplicationEnum
   {
      dia_app_unknown,
      /*
      +------------------+----------------------------+
      | Tag              | Diameter Application       |
      +------------------+----------------------------+
      | aaa+ap1          | NASREQ [RFC3588]           |
      | aaa+ap2          | Mobile IPv4 [RFC4004]      |
      | aaa+ap3          | Base Accounting [RFC3588]  |
      | aaa+ap4          | Credit Control [RFC4006]   |
      | aaa+ap5          | EAP [RFC4072]              |
      | aaa+ap6          | SIP [RFC4740]              |
      | aaa+ap7          | Mobile IPv6 IKE [RFC5778]  |
      | aaa+ap8          | Mobile IPv6 Auth [RFC5778] |
      | aaa+ap9          | QoS [RFC5866]              |
      | aaa+ap4294967295 | Relay [RFC3588]            |
      +------------------+----------------------------+
      */
      dia_app_nasreq,
      dia_app_mobile_ipv4,
      dia_app_base_accounting,
      dia_app_credit_control,
      dia_app_eap,
      dia_app_sip6,
      dia_app_mobile_ipv6_ike,
      dia_app_mobile_ipv6_auth,
      dia_app_qos,
      dia_app_relay,
      /*
      +----------------+----------------------+
      | Tag            | Diameter Application |
      +----------------+----------------------+
      | aaa+ap16777250 | 3GPP STa [TS29.273]  |
      | aaa+ap16777251 | 3GPP S6a [TS29.272]  |
      | aaa+ap16777264 | 3GPP SWm [TS29.273]  |
      | aaa+ap16777267 | 3GPP S9 [TS29.215]   |
      +----------------+----------------------+
      */
      dia_app_3gpp_sta,
      dia_app_3gpp_s6a,
      dia_app_3gpp_swm,
      dia_app_3gpp_s9,
      /*
      +----------------+--------------------------------------------------+
      | Tag            | Diameter Application                             |
      +----------------+--------------------------------------------------+
      | aaa+ap16777281 | WiMAX Network Access Authentication and          |
      |                | Authorization Diameter Application (WNAAADA)     |
      |                | [WiMAX-BASE]                                     |
      | aaa+ap16777282 | WiMAX Network Accounting Diameter Application    |
      |                | (WNADA) [WiMAX-BASE]                             |
      | aaa+ap16777283 | WiMAX MIP4 Diameter Application (WM4DA)          |
      |                | [WiMAX-BASE]                                     |
      | aaa+ap16777284 | WiMAX MIP6 Diameter Application (WM6DA)          |
      |                | [WiMAX-BASE]                                     |
      | aaa+ap16777285 | WiMAX DHCP Diameter Application (WDDA)           |
      |                | [WiMAX-BASE]                                     |
      | aaa+ap16777286 | WiMAX Location Authentication Authorization      |
      |                | Diameter Application (WLAADA) [WiMAX-LBS]        |
      | aaa+ap16777287 | WiMAX Policy and Charging Control R3 Policies    |
      |                | Diameter Application (WiMAX PCC-R3-P)            |
      |                | [WiMAX-PCC]                                      |
      | aaa+ap16777288 | WiMAX Policy and Charging Control R3 Offline     |
      |                | Charging Diameter Application (WiMAX PCC-R3-OFC) |
      |                | [WiMAX-PCC]                                      |
      | aaa+ap16777289 | WiMAX Policy and Charging Control R3 Offline     |
      |                | Charging Prime Diameter Application (WiMAX       |
      |                | PCC-R3-OFC-PRIME) [WiMAX-PCC]                    |
      | aaa+ap16777290 | WiMAX Policy and Charging Control R3 Online      |
      |                | Charging Diameter Application (WiMAX PCC-R3-OC)  |
      |                | [WiMAX-PCC]                                      |
      +----------------+--------------------------------------------------+
      */
      dia_app_wimax_wnaaada,
      dia_app_wimax_wnada,
      dia_app_wimax_wm4da,
      dia_app_wimax_wm6da,
      dia_app_wimax_wdda,
      dia_app_wimax_wlaada,
      dia_app_wimax_pcc_r3_p,
      dia_app_wimax_pcc_r3_ofc,
      dia_app_wimax_pcc_r3_ofc_prime,
      dia_app_wimax_pcc_r3_oc
   };

   enum DiameterProtocolEnum
   {
      dia_protocol_unknown,
      dia_protocol_tcp,
      dia_protocol_sctp,
      dia_protocol_tls_tcp
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class Utility
   {
   public:
      static EString home_network( const char *mnc, const char *mcc );
      static EString home_network( const unsigned char *plmnid );
      static EString home_network_gprs( const char *mnc, const char *mcc );
      static EString home_network_gprs( const unsigned char *plmnid );
      static EString tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc );
      static EString tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid );
      static EString mme_fqdn( const char *mmec, const char *mmegi, const char *mnc, const char *mcc );
      static EString mme_fqdn( const char *mmec, const char *mmegi, const unsigned char *plmnid );
      static EString mme_pool_fqdn( const char *mmegi, const char *mnc, const char *mcc );
      static EString mme_pool_fqdn( const char *mmegi, const unsigned char *plmnid );
      static EString rai_fqdn( const char *rac, const char *lac, const char *mnc, const char *mcc );
      static EString rai_fqdn( const char *rac, const char *lac, const unsigned char *plmnid );
      static EString rnc_fqdn( const char *rnc, const char *mnc, const char *mcc );
      static EString rnc_fqdn( const char *rnc, const unsigned char *plmnid );
      static EString sgsn_fqdn( const char *nri, const char *rac, const char *lac, const char *mnc, const char *mcc );
      static EString sgsn_fqdn( const char *nri, const char *rac, const char *lac, const unsigned char *plmnid );
      static EString epc_nodes_domain_fqdn( const char *mnc, const char *mcc );
      static EString epc_nodes_domain_fqdn( const unsigned char *plmnid );
      static EString epc_node_fqdn( const char *node, const char *mnc, const char *mcc );
      static EString epc_node_fqdn( const char *node, const unsigned char *plmnid );
      static EString nonemergency_epdg_oi_fqdn( const char *mnc, const char *mcc );
      static EString nonemergency_epdg_oi_fqdn( const unsigned char *plmnid );
      static EString nonemergency_epdg_tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc );
      static EString nonemergency_epdg_tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid );
      static EString nonemergency_epdg_lac_fqdn( const char *lac, const char *mnc, const char *mcc );
      static EString nonemergency_epdg_lac_fqdn( const char *lac, const unsigned char *plmnid );
      static EString nonemergency_epdg_visitedcountry_fqdn( const char *mcc );
      static EString nonemergency_epdg_visitedcountry_fqdn( const unsigned char *plmnid );
      static EString emergency_epdg_oi_fqdn( const char *mnc, const char *mcc );
      static EString emergency_epdg_oi_fqdn( const unsigned char *plmnid );
      static EString emergency_epdg_tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc );
      static EString emergency_epdg_tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid );
      static EString emergency_epdg_lac_fqdn( const char *lac, const char *mnc, const char *mcc );
      static EString emergency_epdg_lac_fqdn( const char *lac, const unsigned char *plmnid );
      static EString emergency_epdg_visitedcountry_fqdn( const char *mcc );
      static EString emergency_epdg_visitedcountry_fqdn( const unsigned char *plmnid );
      static EString global_enodeb_id_fqdn( const char *enb, const char *mnc, const char *mcc );
      static EString global_enodeb_id_fqdn( const char *enb, const unsigned char *plmnid );
      static EString local_homenetwork_fqdn( const char *lhn, const char *mcc );
      static EString local_homenetwork_fqdn( const char *lhn, const unsigned char *plmnid );
      static EString epc( const char *mnc, const char *mcc );
      static EString epc( const unsigned char *plmnid );
      static EString apn_fqdn( const char *apnoi, const char *mnc, const char *mcc );
      static EString apn_fqdn( const char *apnoi, const unsigned char *plmnid );
      static EString apn( const char *apnoi, const char *mnc, const char *mcc );
      static EString apn( const char *apnoi, const unsigned char *plmnid );
   
      static AppServiceEnum getAppService( const std::string &s );
      static AppProtocolEnum getAppProtocol( const std::string &p );

      static const char *getAppService( AppServiceEnum s );
      static const char *getAppProtocol( AppProtocolEnum proto );
   
      static AppProtocolEnum getAppProtocol( PGWAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( SGWAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( GGSNAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( SGSNAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( MMEAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( MSCAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( UPFAppProtocolEnum proto );
      static AppProtocolEnum getAppProtocol( AMFAppProtocolEnum proto );
   
      static EString diameter_fqdn( const char *mnc, const char *mcc );
      static EString diameter_fqdn( const unsigned char *plmnid );

      static uint32_t getDiameterApplication( DiameterApplicationEnum app );
      static const char *getDiameterProtocol( DiameterProtocolEnum protocol );

      static EString getDiameterService( DiameterApplicationEnum app, DiameterProtocolEnum protocol );

   private:
      Utility() {}
   };
   
   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////
   
   class CanonicalNodeName : public std::list<std::string>
   {
   public:
      CanonicalNodeName();
      CanonicalNodeName( const std::string &n );

      void setName( const std::string &n );

      bool getTopon() { return m_topon; }
      const EString getName() const { return m_name; }

      int topologicalCompare( const CanonicalNodeName &right );

      void dump( const char *prefix )
      {
         std::cout << prefix << "Name - " << m_name << std::endl;
         std::cout << prefix << "topon - " << (m_topon?"true":"false") << std::endl;

         std::cout << prefix << "Labels" << std::endl;
         for ( CanonicalNodeName::const_iterator it = begin(); it != end(); ++it )
            std::cout << prefix << "  " << *it << std::endl;
      }

   private:
      bool m_topon;
      EString m_name;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   typedef int UsageType;

   class UsageTypeList : public std::list<UsageType>
   {
   public:
      UsageTypeList() {}
      void dump( const char *prefix )
      {
         for (UsageTypeList::const_iterator it = begin();
              it != end();
              ++it)
         {
            std::cout << prefix << *it << std::endl;
         }
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   typedef std::string NetworkCapability;

   class NetworkCapabilityList: public std::list<NetworkCapability>
   {
   public:
      void dump( const char *prefix )
      {
         for (NetworkCapabilityList::const_iterator it = begin();
              it != end();
              ++it)
         {
            std::cout << prefix << *it << std::endl;
         }
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class StringVector : public std::vector<std::string>
   {
   public:
      void dump( const char *prefix )
      {
         for (StringVector::const_iterator it = begin();
              it != end();
              ++it)
         {
            std::cout << prefix << *it << std::endl;
         }
      }

      void shuffle()
      {
         std::random_shuffle ( begin(), end() );
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class AppProtocol
   {
   public:
      AppProtocol() { m_protocol = x_unknown; }
      ~AppProtocol() {}

      const EString &getRawProtocol() { return m_rawProtocol; }
      AppProtocolEnum getProtocol() { return m_protocol; }
      UsageTypeList &getUsageTypes() { return m_usageTypes; }
      NetworkCapabilityList &getNetworkCapabilities() { return m_networkCapabilities; }

      AppProtocolEnum setProtocol( AppProtocolEnum ape ) { return m_protocol = ape; }
      UsageType addUsageType( UsageType ut ) { m_usageTypes.push_back( ut ); return ut; }
      NetworkCapability &addNetworkCapability( NetworkCapability &nc ) { m_networkCapabilities.push_back(nc); return m_networkCapabilities.back(); }

      void parse( const std::string &rp );

      bool findUsageType( UsageType ut )
      {
         return std::find( m_usageTypes.begin(), m_usageTypes.end(), ut ) != m_usageTypes.end();
      }

      bool findNetworkCapability( const NetworkCapability &nc )
      {
         return std::find( m_networkCapabilities.begin(), m_networkCapabilities.end(), nc ) != m_networkCapabilities.end();
      }

      void dump( const char *prefix = "" )
      {
         std::cout << prefix << "Protocol [" << Utility::getAppProtocol( m_protocol );
         if ( !m_rawProtocol.empty() )
            std::cout << "/" << m_rawProtocol;
         std::cout << "] - [";
         bool first = true;
         for ( UsageTypeList::const_iterator it = m_usageTypes.begin(); it != m_usageTypes.end(); ++it )
         {
            std::cout << (first?"":",") << *it;
            first = false;
         }
         std::cout << "] - [";
         first = true;
         for ( NetworkCapabilityList::const_iterator it = m_networkCapabilities.begin(); it != m_networkCapabilities.end(); ++it )
         {
            std::cout << (first?"":",") << *it;
            first = false;
         }
         std::cout << "]" << std::endl;
      }

   private:
      EString m_rawProtocol;
      AppProtocolEnum m_protocol;
      UsageTypeList m_usageTypes;
      NetworkCapabilityList m_networkCapabilities;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class AppProtocolList : public std::list<AppProtocol*>
   {
   public:
      AppProtocolList() {}
      ~AppProtocolList()
      {
         while ( !empty() )
         {
            AppProtocol *ap = *begin();
            erase( begin() );
            delete ap;
         }
      }

      void dump( const char *prefix )
      {
         for (AppProtocolList::const_iterator it = begin();
              it != end();
              ++it)
         {
            (*it)->dump( prefix );
         }
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class AppService
   {
   public:
      AppService() { m_service = x_3gpp_unknown; }
      AppService( const std::string &ds ) { parse( ds ); }
      ~AppService()
      {
         while ( !m_protocols.empty() )
         {
            AppProtocol *ap = *m_protocols.begin();
            m_protocols.pop_front();
            delete ap;
         }
      }

      AppServiceEnum getService() { return m_service; }
      AppProtocolList &getProtocols() { return m_protocols; }
      EString &getRawService() { return m_rawService; }

      void parse( const std::string &ds );

      AppProtocol *findProtocol( AppProtocolEnum protocol )
      {
         for ( AppProtocolList::const_iterator it = m_protocols.begin(); it != m_protocols.end(); ++it )
         {
            if ( (*it)->getProtocol() == protocol )
               return *it;
         }

         return NULL;
      }

      void dump()
      {
         std::cout << m_rawService << std::endl;
         std::cout << "   Service  [" << Utility::getAppService( m_service ) << "]" << std::endl;
         for ( AppProtocolList::const_iterator it = m_protocols.begin(); it != m_protocols.end(); ++it )
         {
            (*it)->dump( "   " );
         }
      }

   private:

      AppServiceEnum m_service;
      AppProtocolList m_protocols;
      EString m_rawService;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class NodeSelectorResult
   {
   public:
      NodeSelectorResult()
      {
         m_order = 0;
         m_preference = 0;
         m_port = 0;
      }

      const EString &getHostname() { return m_hostname; }
      uint16_t getOrder() { return m_order; }
      uint16_t getPreference() { return m_preference; }
      uint16_t getPort() { return m_port; }
      AppProtocolList &getSupportedProtocols() { return m_supported_protocols; }
      StringVector &getIPv4Hosts() { return m_ipv4_hosts; }
      StringVector &getIPv6Hosts() { return m_ipv6_hosts; }

      uint16_t setOrder( uint16_t order ) { return m_order = order; }
      uint16_t setPreference( uint16_t preference ) { return m_preference = preference; }
      uint16_t setPort( uint16_t port ) { return m_port = port; }
      const EString &setHostname( const std::string &hostname ) { m_hostname = hostname; return m_hostname; }
      void addSupportedProtocol( AppProtocol *ap ) { m_supported_protocols.push_back( ap ); }
      void addIPv4Host( const std::string &host ) { m_ipv4_hosts.push_back( host ); }
      void addIPv6Host( const std::string &host ) { m_ipv6_hosts.push_back( host ); }

      void dump( const char *prefix )
      {
         std::cout << prefix << "HOSTNAME: " << m_hostname << std::endl;
         std::cout << prefix << "  order - " << m_order << std::endl;
         std::cout << prefix << "  preference - " << m_preference << std::endl;
         std::cout << prefix << "  port - " << m_port << std::endl;

         EString pfx( prefix );
         std::cout << prefix << "  supported protocols" << std::endl;
         pfx.append( "    " );
         m_supported_protocols.dump( pfx.c_str() );

         std::cout << prefix << "  IPv4 HOSTS" << std::endl;
         m_ipv4_hosts.dump( pfx.c_str() );

         std::cout << prefix << "  IPv6 HOSTS" << std::endl;
         m_ipv6_hosts.dump( pfx.c_str() );

         //std::cout << prefix << "  Canonical Name" << std::endl;
         //m_canonicalname.dump( pfx.c_str() );
      }

   private:
      EString m_hostname;
      uint16_t m_order;
      uint16_t m_preference;
      uint16_t m_port;
      AppProtocolList m_supported_protocols;
      StringVector m_ipv4_hosts;
      StringVector m_ipv6_hosts;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class NodeSelectorResultList : public std::list<NodeSelectorResult*>
   {
   public:
      NodeSelectorResultList() {}
      ~NodeSelectorResultList()
      {
         while ( !empty() )
         {
            NodeSelectorResult* nsr = *begin();
            erase( begin() );
            delete nsr;
         }
      }

      void dump( const char *prefix )
      {
         for (NodeSelectorResultList::const_iterator it = begin();
              it != end();
              ++it)
         {
            (*it)->dump( prefix );
         }
      }

      static bool sort_compare( NodeSelectorResult*& first, NodeSelectorResult*& second );
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class NodeSelector
   {
   public:
      DNS::namedserverid_t getNamedServerID() { return m_nsid; }
      DNS::namedserverid_t setNamedServerID(DNS::namedserverid_t nsid) { return m_nsid = nsid; }
   
      AppServiceEnum getDesiredService() { return m_desiredService; }
      AppProtocolList &getDesiredProtocols() { return m_desiredProtocols; }
      const EString &getDomainName() { return m_domain; }
      NodeSelectorResultList &getResults() { return m_results; }
   
      UsageType addDesiredUsageType( UsageType ut ) { m_desiredUsageTypes.push_back( ut ); return ut; }
      NetworkCapability &addDesiredNetworkCapability( const char *nc ) { EString s(nc); m_desiredNetworkCapabilities.push_back( s ); return m_desiredNetworkCapabilities.back(); }
      NetworkCapability &addDesiredNetworkCapability( NetworkCapability &nc ) { m_desiredNetworkCapabilities.push_back( nc ); return nc; }

      NodeSelectorResultList &process();

      void dump()
      {
         std::cout << "NodeSelector REQUEST" << std::endl;
         std::cout << "  domain - " << m_domain << std::endl;
         std::cout << "  desired service - " << Utility::getAppService( m_desiredService ) << std::endl;
         std::cout << "  desired protocols" << std::endl;
         m_desiredProtocols.dump( "    " );
         std::cout << "  desired usage types" << std::endl;
         m_desiredUsageTypes.dump( "    " );
         std::cout << "  desired network capabilities" << std::endl;
         m_desiredNetworkCapabilities.dump( "    " );
         std::cout << "  results" << std::endl;
         m_results.dump( "    " );
      }
   
   protected:
      NodeSelector();
      ~NodeSelector();
   
      AppServiceEnum setAppService( AppServiceEnum s ) { return m_desiredService = s; }
      const EString &setDomainName( const std::string &d ) { m_domain = d; return m_domain; }
      void addDesiredProtocol( AppProtocol *ap ) { m_desiredProtocols.push_back( ap ); }
   
   private:
      AppServiceEnum parseService( const std::string &service, std::list<AppProtocolEnum> &protocols ) const;
      static bool naptr_compare( DNS::RRecordNAPTR*& first, DNS::RRecordNAPTR*& second );
   
      DNS::namedserverid_t m_nsid;
      EString m_domain;
      AppServiceEnum m_desiredService;
      AppProtocolList m_desiredProtocols;
      UsageTypeList m_desiredUsageTypes;
      NetworkCapabilityList m_desiredNetworkCapabilities;

      NodeSelectorResultList m_results;
      DNS::QueryPtr m_query;
   };
   
   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////
   
   class ColocatedCandidate
   {
   public:
      enum PairType
      {
         ptUnknown = 0,
         ptColocated = 1,
         ptTopologicalDistance = 2,
         ptDNSPriority = 3
      };

      ColocatedCandidate( NodeSelectorResult &candidate1, NodeSelectorResult &candidate2 );

      NodeSelectorResult &getCandidate1() { return m_candidate1; }
      NodeSelectorResult &getCandidate2() { return m_candidate2; }
      CanonicalNodeName &getCanonicalNodeName1() { return m_cnn1; }
      CanonicalNodeName &getCanonicalNodeName2() { return m_cnn2; }
      PairType getPairType() { return m_pairtype; }
      int getTopologicalMatches() { return m_topologicalMatches; }

      void dump( const char *prefix )
      {
         EString pfx( prefix );
         pfx.append( "  " );

         std::cout << prefix << "canonical name 1 - " << m_cnn1.getName() << std::endl;
         m_candidate1.dump( pfx.c_str() );
         std::cout << prefix << "canonical name 2 - " << m_cnn2.getName() << std::endl;
         m_candidate2.dump( pfx .c_str());
         std::cout << prefix << "pair type - " << (
            m_pairtype == ptColocated ? "colocated" :
            m_pairtype == ptTopologicalDistance ? "topological distance" :
            m_pairtype == ptDNSPriority ? "DNS priority" : "unknown") << std::endl;
         std::cout << prefix << "topological matches - " << m_topologicalMatches << std::endl;
      }

   private:
      ColocatedCandidate();

      NodeSelectorResult &m_candidate1;
      NodeSelectorResult &m_candidate2;
      CanonicalNodeName m_cnn1;
      CanonicalNodeName m_cnn2;
      PairType m_pairtype;
      int m_topologicalMatches;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class ColocatedCandidateList : public std::list<ColocatedCandidate*>
   {
   public:
      ColocatedCandidateList( NodeSelectorResultList &nodelist1, NodeSelectorResultList &nodelist2 );
      ~ColocatedCandidateList();

      void dump( const char *prefix = "" )
      {
         EString pfx1( prefix );
         pfx1.append( "  " );
         EString pfx2( pfx1 );
         pfx2.append("  ");

         int candidate = 1;
         std::cout << prefix << "candidate list" << std::endl;
         for ( ColocatedCandidateList::const_iterator it = begin(); it != end(); ++it )
         {
            std::cout << pfx1 << "candidate " << candidate++ << std::endl;
            (*it)->dump( pfx2.c_str() );
         }
      }

   private:
      ColocatedCandidateList();

      static bool sort_compare( ColocatedCandidate*& first, ColocatedCandidate*& second );

      NodeSelectorResultList &m_nodelist1;
      NodeSelectorResultList &m_nodelist2;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class MMENodeSelector : public NodeSelector
   {
   public:
      MMENodeSelector( const char *mmec, const char *mmegi, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_fqdn( mmec, mmegi, mnc, mcc ) );
      }
   
      MMENodeSelector( const char *mmec, const char *mmegi, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_fqdn( mmec, mmegi, plmnid ) );
      }
   
      MMENodeSelector( const char *mmegi, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_pool_fqdn( mmegi, mnc, mcc ) );
      }
   
      MMENodeSelector( const char *mmegi, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_pool_fqdn( mmegi, plmnid ) );
      }
   
      AppProtocol *addDesiredProtocol( MMEAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }
   
   private:
      MMENodeSelector() {}
   };
   
   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class PGWNodeSelector : public NodeSelector
   {
   public:
      PGWNodeSelector( const char *apnoi, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_pgw );
         setDomainName( Utility::apn_fqdn( apnoi, mnc, mcc ) );
      }
   
      PGWNodeSelector( const char *apnoi, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_pgw );
         setDomainName( Utility::apn_fqdn( apnoi, plmnid ) );
      }
   
      AppProtocol *addDesiredProtocol( PGWAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }
   
   private:
      PGWNodeSelector() {}
   };
   
   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class PGWUPFNodeSelector : public NodeSelector
   {
   public:
      PGWUPFNodeSelector( const char *apnoi, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::apn_fqdn( apnoi, mnc, mcc ) );
      }

      PGWUPFNodeSelector( const char *apnoi, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::apn_fqdn( apnoi, plmnid ) );
      }

      AppProtocol *addDesiredProtocol( UPFAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }

   private:
      PGWUPFNodeSelector() {}
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class SGWNodeSelector : public NodeSelector
   {
   public:
      SGWNodeSelector( const unsigned char lb, const unsigned char hb, const char *mnc, const char *mcc )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, mnc, mcc ) );
      }
   
      SGWNodeSelector( unsigned char lb, unsigned char hb, const unsigned char *plmnid )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, plmnid ) );
      }
   
      SGWNodeSelector( const char *lb, const char *hb, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lb, hb, mnc, mcc ) );
      }
   
      SGWNodeSelector( const char *lb, const char *hb, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lb, hb, plmnid ) );
      }

      SGWNodeSelector( const char *node, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::epc_node_fqdn( node, mnc, mcc ) );
      }
   
      SGWNodeSelector( const char *node, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::epc_node_fqdn( node, plmnid ) );
      }
   
      AppProtocol *addDesiredProtocol( SGWAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }
   
   private:
      SGWNodeSelector() {}
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class SGWUPFNodeSelector : public NodeSelector
   {
   public:
      SGWUPFNodeSelector( const unsigned char lb, const unsigned char hb, const char *mnc, const char *mcc )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, mnc, mcc ) );
      }

      SGWUPFNodeSelector( unsigned char lb, unsigned char hb, const unsigned char *plmnid )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, plmnid ) );
      }

      SGWUPFNodeSelector( const char *lb, const char *hb, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lb, hb, mnc, mcc ) );
      }

      SGWUPFNodeSelector( const char *lb, const char *hb, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lb, hb, plmnid ) );
      }

      SGWUPFNodeSelector( const char *node, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::epc_node_fqdn( node, mnc, mcc ) );
      }

      AppProtocol *addDesiredProtocol( UPFAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }

   private:
      SGWUPFNodeSelector() {}
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class ENodeBUPFNodeSelector : public NodeSelector
   {
   public:
      ENodeBUPFNodeSelector( const char *enb, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::global_enodeb_id_fqdn( enb, mnc, mcc ) );
      }

      ENodeBUPFNodeSelector( const char *enb, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::global_enodeb_id_fqdn( enb, plmnid ) );
      }

      AppProtocol *addDesiredProtocol( UPFAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }

   private:
      ENodeBUPFNodeSelector() {}
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   enum DiameterNaptrTypeEnum
   {
      dnt_unknown,
      dnt_hostname,
      dnt_service
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterNaptr
   {
   public:
      DiameterNaptr( DiameterNaptrTypeEnum drt )
         : m_type( drt )
      {
      }

      virtual ~DiameterNaptr()
      {
      }

      DiameterNaptrTypeEnum getType() { return m_type; }

      uint16_t getOrder() { return m_order; }
      uint16_t setOrder( uint16_t order ) { return m_order = order; }

      uint16_t getPreference() { return m_order; }
      uint16_t setPreference( uint16_t pref ) { return m_preference = pref; }

      const EString &getService() { return m_service; }
      const EString &setService( const char *service ) { return m_service = service; }
      const EString &setService( const std::string &service ) { return setService( service.c_str() ); }

      const EString &getReplacement() { return m_replacement; }
      const EString &setReplacement( const char *replacement ) { return m_replacement = replacement; }
      const EString &setReplacement( const std::string &replacement ) { return setReplacement( replacement.c_str() ); }

      void dump( const char *prefix )
      {
         std::cout << prefix << "Flags: " << (
            m_type == dnt_hostname ? "hostname" :
            m_type == dnt_service ? "service" : "unknown") << std::endl;
         std::cout << prefix << "Order: " << m_order << std::endl;
         std::cout << prefix << "Preference: " << m_preference << std::endl;
         std::cout << prefix << "Service: " << m_service << std::endl;
         std::cout << prefix << "Replacement: " << m_replacement << std::endl;
      }

   private:
      DiameterNaptr();

      DiameterNaptrTypeEnum m_type;
      uint16_t m_order;
      uint16_t m_preference;
      EString m_service;
      EString m_replacement;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterHost
   {
   public:
      DiameterHost() {}
      DiameterHost( const char *name ) : m_name( name ) {}
      DiameterHost( const std::string &name ) : m_name( name ) {}

      const EString &getName() const { return m_name; }
      const EString &setName( const char *name ) { return m_name = name; }
      const EString &setName( const std::string &name ) { return setName( name.c_str() ); }

      StringVector &getIPv4Addresses() { return m_ipv4_addrs; }
      StringVector &getIPv6Addresses() { return m_ipv6_addrs; }

      void addIPv4Address( const std::string &addr ) { m_ipv4_addrs.push_back( addr ); }
      void addIPv6Address( const std::string &addr ) { m_ipv6_addrs.push_back( addr ); }

      void dump( const char *prefix )
      {
         EString pfx( prefix );
         pfx.append( "  " );

         std::cout << prefix << "Name: " << m_name << std::endl;
         std::cout << prefix << "IPv4 addresses" << std::endl;
         m_ipv4_addrs.dump( pfx.c_str() );
         std::cout << prefix << "IPv6 addresses" << std::endl;
         m_ipv6_addrs.dump( pfx.c_str() );
      }

   private:
      EString m_name;
      StringVector m_ipv4_addrs;
      StringVector m_ipv6_addrs;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterSrv
   {
   public:
      DiameterSrv() {}
      ~DiameterSrv() {}

      uint16_t getPriority() { return m_priority; }
      uint16_t setPriority( uint16_t priority ) { return m_priority = priority; }

      uint16_t getWeight() { return m_weight; }
      uint16_t setWeight( uint16_t weight ) { return m_weight = weight; }

      uint16_t getPort() { return m_port; }
      uint16_t setPort( uint16_t port ) { return m_port = port; }

      DiameterHost &getHost() { return m_host; }

      void dump( const char *prefix )
      {
         EString pfx( prefix );
         
         std::cout << prefix << "Priority: " << m_priority << std::endl;
         std::cout << prefix << "Weight: " << m_weight << std::endl;
         std::cout << prefix << "Port: " << m_port << std::endl;
       
         std::cout << prefix << "Diameter Host" << std::endl;
         m_host.dump( pfx.c_str() );
      }

   private:
      uint16_t m_priority;
      uint16_t m_weight;
      uint16_t m_port;
      DiameterHost m_host;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterSrvVector : public std::vector<DiameterSrv*>
   {
   public:
      ~DiameterSrvVector()
      {
         while ( !empty() )
         {
            DiameterSrv *ds = *begin();
            erase( begin() );
            delete ds;
         }
      }

      void dump( const char *prefix )
      {
         EString pfx( prefix );
         pfx.append( "  " );

         int cnt = 0;
         for (DiameterSrvVector::const_iterator it = begin();
              it != end();
              ++it)
         {
            std::cout << prefix << "Diameter SRV " << ++cnt << std::endl;
            (*it)->dump( pfx.c_str() );
         }
      }

      void sort_vector();

   private:
      static bool sort_compare( DiameterSrv* first, DiameterSrv* second );
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterNaptrA : public DiameterNaptr
   {
   public:
      DiameterNaptrA() : DiameterNaptr( dnt_hostname ) {}

      DiameterHost &getHost() { return m_host; }

      void dump( const char *prefix )
      {
         EString pfx( prefix );
         pfx.append( "  " );
         DiameterNaptr::dump( prefix );
         std::cout << prefix << "Host Information" << std::endl;
         m_host.dump( pfx.c_str() );
      }

   private:
      DiameterHost m_host;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterNaptrS : public DiameterNaptr
   {
   public:
      DiameterNaptrS() : DiameterNaptr( dnt_service ) {}

      DiameterSrvVector &getSrvs() { return m_srv; }

      void dump( const char *prefix )
      {
         EString pfx( prefix );
         pfx.append( "  " );
         DiameterNaptr::dump( prefix );
         std::cout << prefix << "Service Information" << std::endl;
         m_srv.dump( pfx.c_str() );
      }

   private:
      DiameterSrvVector m_srv;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterNaptrList : public std::list<DiameterNaptr*>
   {
   public:
      ~DiameterNaptrList()
      {
         while ( !empty() )
         {
            DiameterNaptr *n = *begin();
            erase( begin() );
            delete n;
         }
      }

      void dump( const char *prefix = "" )
      {
         for (DiameterNaptrList::const_iterator it = begin();
              it != end();
              ++it)
         {
            if ( (*it)->getType() == dnt_hostname )
               ((DiameterNaptrA*)*it)->dump( prefix );
            if ( (*it)->getType() == dnt_service )
               ((DiameterNaptrS*)*it)->dump( prefix );
         }
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   class DiameterSelector
   {
   public:
      DiameterSelector();

      const EString &getRealm() { return m_realm; }
      const EString &setRealm( const char *realm ) { return m_realm = realm; }
      const EString &setRealm( const char *mnc, const char *mcc ) { return m_realm = Utility::diameter_fqdn( mnc, mcc ); }
      const EString &setRealm( const unsigned char *plmnid ) { return m_realm = Utility::diameter_fqdn( plmnid ); }

      DiameterApplicationEnum getApplicataion() { return m_application; }
      DiameterApplicationEnum setApplication( DiameterApplicationEnum app ) { return m_application = app; }

      DiameterProtocolEnum getProtocol() { return m_protocol; }
      DiameterProtocolEnum setProtocol( DiameterProtocolEnum proto ) { return m_protocol = proto; }

      DiameterNaptrList &process();

   private:
      EString m_realm;
      DiameterApplicationEnum m_application;
      DiameterProtocolEnum m_protocol;

      DNS::QueryPtr m_query;
      DiameterNaptrList m_results;
   };

} // namespace EPCDNS

#endif // #ifndef __EPCDNS_H
