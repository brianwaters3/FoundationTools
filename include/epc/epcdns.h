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

/// @file
/// @brief Classes and macros to support DNS functions defined in TS 29.303.

#include <stdio.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <list>
#include <vector>

#include "estring.h"
#include "dnscache.h"

/// @cond DOXYGEN_EXCLUDE
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
/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief The namespace for the TS 29.303 related DNS functions.
namespace EPCDNS
{
   /*
    * Service and Protocol service names for 3GPP
    * TS 23.003 V15.5.0, Table 19.4.3.1
    */
   /// @brief Service names defined in 23.003 v15.5.0, table 19.4.3.1
   enum AppServiceEnum
   {
      /// Unknown
      x_3gpp_unknown,
      /// Any
      x_3gpp_any,
      /// PGW
      x_3gpp_pgw,
      /// SGW
      x_3gpp_sgw,
      /// GGSN
      x_3gpp_ggsn,
      /// SGSN
      x_3gpp_sgsn,
      /// MME
      x_3gpp_mme,
      /// MSC
      x_3gpp_msc,
      /// UPF
      x_3gpp_upf,
      /// AMF
      x_3gpp_amf
   };
   
   /// @brief 3GPP protocols defined in 23.003 v15.5.0, table 19.4.3.1.
   enum AppProtocolEnum
   {
      /// Unknown
      x_unknown,
      /// x-gn
      x_gn,
      /// x-gp
      x_gp,
      /// x-n2
      x_n2,
      /// x-nq
      x_nq,
      /// x-nqprime
      x_nqprime,
      /// x-s1-mme
      x_s1_mme,
      /// x-s1-u
      x_s1_u,
      /// x-s10
      x_s10,
      /// x-s11
      x_s11,
      /// x-s12
      x_s12,
      /// x-16
      x_s16,
      /// x-s2a-gtp
      x_s2a_gtp,
      /// x-s2a-mipv4
      x_s2a_mipv4,
      /// x_s2a_pmip
      x_s2a_pmip,
      /// x-s2b-gtp
      x_s2b_gtp,
      /// x-s2b-pmip
      x_s2b_pmip,
      /// x-s2c-dsmip
      x_s2c_dsmip,
      /// x-s3
      x_s3,
      /// x-s4
      x_s4,
      /// x-s5-gtp
      x_s5_gtp,
      /// x-s5-pmip
      x_s5_pmip,
      /// x-s6a
      x_s6a,
      /// x-s8-gtp
      x_s8_gtp,
      /// x-s8-pmip
      x_s8_pmip,
      /// x-sv
      x_sv,
      /// x-sxa
      x_sxa,
      /// x-sxb
      x_sxb,
      /// x-sxc
      x_sxc
   };
   
   /// @brief the PGW related protocols.
   enum PGWAppProtocolEnum
   {
      /// x-gn
      pgw_x_gn,
      /// x-gp
      pgw_x_gp,
      /// x-s2a-gtp
      pgw_x_s2a_gtp,
      /// x-s2a-mipv4
      pgw_x_s2a_mipv4,
      /// x-s2a-pmip
      pgw_x_s2a_pmip,
      /// x-s2b-gtp
      pgw_x_s2b_gtp,
      /// x-s2b-pmip
      pgw_x_s2b_pmip,
      /// x-s2c-dsmip
      pgw_x_s2c_dsmip,
      /// x-s5-gtp
      pgw_x_s5_gtp,
      /// x-s5-pmip
      pgw_x_s5_pmip,
      /// x-s8-gtp
      pgw_x_s8_gtp,
      /// x-s8-pmip
      pgw_x_s8_pmip
   };
   
   /// @brief the SGW related protocols.
   enum SGWAppProtocolEnum
   {
      /// x-s1_u
      sgw_x_s1_u,
      /// x-s11
      sgw_x_s11,
      /// x-s12
      sgw_x_s12,
      /// x-s2a-pmip
      sgw_x_s2a_pmip,
      /// x-s2b-pmip
      sgw_x_s2b_pmip,
      /// x-s4
      sgw_x_s4,
      /// x-s5-gtp
      sgw_x_s5_gtp,
      /// x-s5-pmip
      sgw_x_s5_pmip,
      /// x-s8-gtp
      sgw_x_s8_gtp,
      /// x-s8-pmip
      sgw_x_s8_pmip
   };
   
   /// @brief the GGSN related protocols.
   enum GGSNAppProtocolEnum
   {
      /// x-gn
      ggsn_x_gn,
      /// x-gp
      ggsn_x_gp
   };
   
   /// @brief the SGSN related protocols.
   enum SGSNAppProtocolEnum
   {
      /// x-gn
      sgsn_x_gn,
      /// x-gp
      sgsn_x_gp,
      /// x-nqprime
      sgsn_x_nqprime,
      /// x-s16
      sgsn_x_s16,
      /// x-s3
      sgsn_x_s3,
      /// x-s4
      sgsn_x_s4,
      /// x-sv
      sgsn_x_sv
   };
   
   /// @brief the SGSN related protocols.
   enum MMEAppProtocolEnum
   {
      /// x-gn
      mme_x_gn,
      /// x-gp
      mme_x_gp,
      /// x-nq
      mme_x_nq,
      /// x-s10
      mme_x_s10,
      /// x-s11
      mme_x_s11,
      /// x-s1-mme
      mme_x_s1_mme,
      /// x-s3
      mme_x_s3,
      /// x-s6a
      mme_x_s6a,
      /// x-sv
      mme_x_sv
   };
   
   /// @brief the MSC related protocols.
   enum MSCAppProtocolEnum
   {
      /// x-sv
      msc_x_sv
   };
   
   /// @brief the UPF related protocols.
   enum UPFAppProtocolEnum
   {
      /// x-sxa
      upf_x_sxa,
      /// x-sxb
      upf_x_sxb,
      /// x-sxc
      upf_x_sxc
   };

   /// @brief the AMF related protocols.
   enum AMFAppProtocolEnum
   {
      /// x-n2
      amf_x_n2
   };

   /// @brief the Diameter application ID's.
   enum DiameterApplicationEnum
   {
      /// unknown
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
      /// NASREQ
      dia_app_nasreq,
      /// Mobile IPv4
      dia_app_mobile_ipv4,
      /// Base Accounting
      dia_app_base_accounting,
      /// Credit Control
      dia_app_credit_control,
      /// EAP
      dia_app_eap,
      /// SIP
      dia_app_sip6,
      /// Mobile IPv6 IKE
      dia_app_mobile_ipv6_ike,
      /// Mobile IPv6 Auth
      dia_app_mobile_ipv6_auth,
      /// QoS
      dia_app_qos,
      /// Relay
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
      /// 3GPP STa
      dia_app_3gpp_sta,
      /// 3GPP S6a
      dia_app_3gpp_s6a,
      /// 3GPP SWm
      dia_app_3gpp_swm,
      /// 3GPP S9
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
      /// iMAX Network Access Authentication and Authorization Diameter Application (WNAAADA)
      dia_app_wimax_wnaaada,
      /// WiMAX Network Accounting Diameter Application (WNADA)
      dia_app_wimax_wnada,
      /// WiMAX MIP4 Diameter Application (WM4DA)
      dia_app_wimax_wm4da,
      /// WiMAX MIP6 Diameter Application (WM6DA)
      dia_app_wimax_wm6da,
      /// WiMAX DHCP Diameter Application (WDDA)
      dia_app_wimax_wdda,
      /// WiMAX Location Authentication Authorization Diameter Application (WLAADA)
      dia_app_wimax_wlaada,
      /// WiMAX Policy and Charging Control R3 Policies Diameter Application (WiMAX PCC-R3-P)
      dia_app_wimax_pcc_r3_p,
      /// WiMAX Policy and Charging Control R3 Offline Charging Diameter Application (WiMAX PCC-R3-OFC)
      dia_app_wimax_pcc_r3_ofc,
      /// WiMAX Policy and Charging Control R3 Offline Charging Prime Diameter Application (WiMAX PCC-R3-OFC-PRIME)
      dia_app_wimax_pcc_r3_ofc_prime,
      /// WiMAX Policy and Charging Control R3 Online Charging Diameter Application (WiMAX PCC-R3-OC)
      dia_app_wimax_pcc_r3_oc
   };

   /// @brief The Diameter transport protocols.
   enum DiameterProtocolEnum
   {
      /// Unknown
      dia_protocol_unknown,
      /// TCP
      dia_protocol_tcp,
      /// SCTP
      dia_protocol_sctp,
      /// TCP with TLS
      dia_protocol_tls_tcp
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Provides several utilities for manipulating names, services and protocols.
   class Utility
   {
   public:
      /// @brief Constructs the home network domain name.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the home network domain name.
      static EString home_network( const char *mnc, const char *mcc );
      /// @brief Constructs the home network domain name.
      /// @param plmnid the public land mobile network ID.
      /// @return the home network domain name.
      static EString home_network( const unsigned char *plmnid );
      /// @brief Constructs the APN operator identifier.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the APN operator identifier.
      static EString home_network_gprs( const char *mnc, const char *mcc );
      /// @brief Constructs the APN operator identifier.
      /// @param plmnid the public land mobile network ID.
      /// @return the APN operator identifier.
      static EString home_network_gprs( const unsigned char *plmnid );
      /// @brief Constructs the tracking area identity FQDN.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the tracking area identity FQDN.
      static EString tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc );
      /// @brief Constructs the tracking area identity FQDN.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      /// @return the tracking area identity FQDN.
      static EString tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid );
      /// @brief Constructs the mobile management entity (MME) FQDN.
      /// @param mmec MME code.
      /// @param mmegi MME Group ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the mobile management entity (MME) FQDN.
      static EString mme_fqdn( const char *mmec, const char *mmegi, const char *mnc, const char *mcc );
      /// @brief Constructs the mobile management entity (MME) FQDN.
      /// @param mmec MME code.
      /// @param mmegi MME Group ID.
      /// @param plmnid the public land mobile network ID.
      /// @return the mobile management entity (MME) FQDN.
      static EString mme_fqdn( const char *mmec, const char *mmegi, const unsigned char *plmnid );
      /// @brief Constructs the MME pool FQDN.
      /// @param mmegi MME Group ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the MME pool FQDN.
      static EString mme_pool_fqdn( const char *mmegi, const char *mnc, const char *mcc );
      /// @brief Constructs the MME pool FQDN.
      /// @param mmegi MME Group ID.
      /// @param plmnid the public land mobile network ID.
      /// @return the MME pool FQDN.
      static EString mme_pool_fqdn( const char *mmegi, const unsigned char *plmnid );
      /// @brief Constructs the routing area identity (RAI) FQDN.
      /// @param rac routing area code.
      /// @param lac location area code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the routing area identity (RAI) FQDN.
      static EString rai_fqdn( const char *rac, const char *lac, const char *mnc, const char *mcc );
      /// @brief Constructs the routing area identity (RAI) FQDN.
      /// @param rac routing area code.
      /// @param lac location area code.
      /// @param plmnid the public land mobile network ID.
      /// @return the routing area identity (RAI) FQDN.
      static EString rai_fqdn( const char *rac, const char *lac, const unsigned char *plmnid );
      /// @brief Constructs the radio network controller (RNC) FQDN.
      /// @param rnc radio network controller ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the radio network controller (RNC) FQDN.
      static EString rnc_fqdn( const char *rnc, const char *mnc, const char *mcc );
      /// @brief Constructs the radio network controller (RNC) FQDN.
      /// @param rnc radio network controller ID.
      /// @param plmnid the public land mobile network ID.
      /// @return the radio network controller (RNC) FQDN.
      static EString rnc_fqdn( const char *rnc, const unsigned char *plmnid );
      /// @brief Constructs the serving GPRS support node (SGSN) FQDN.
      /// @param nri network resource identifier.
      /// @param rac routing area code.
      /// @param lac location area code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the serving GPRS support node (SGSN) FQDN.
      static EString sgsn_fqdn( const char *nri, const char *rac, const char *lac, const char *mnc, const char *mcc );
      /// @brief Constructs the serving GPRS support node (SGSN) FQDN.
      /// @param nri network resource identifier.
      /// @param rac routing area code.
      /// @param lac location area code.
      /// @param plmnid the public land mobile network ID.
      /// @return the serving GPRS support node (SGSN) FQDN.
      static EString sgsn_fqdn( const char *nri, const char *rac, const char *lac, const unsigned char *plmnid );
      /// @brief Constructs the EPC nodes subdomain (DNS zone).
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the EPC nodes subdomain (DNS zone).
      static EString epc_nodes_domain_fqdn( const char *mnc, const char *mcc );
      /// @brief Constructs the EPC nodes subdomain (DNS zone).
      /// @param plmnid the public land mobile network ID.
      /// @return the EPC nodes subdomain (DNS zone).
      static EString epc_nodes_domain_fqdn( const unsigned char *plmnid );
      /// @brief Constructs the EPC node FQDN.
      /// @param node the node name.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the EPC node FQDN.
      static EString epc_node_fqdn( const char *node, const char *mnc, const char *mcc );
      /// @brief Constructs the EPC node FQDN.
      /// @param node the node name.
      /// @param plmnid the public land mobile network ID.
      /// @return the EPC node FQDN.
      static EString epc_node_fqdn( const char *node, const unsigned char *plmnid );
      /// @brief Constructs the operator identifier (OI) based ePDG FQDN.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the operator identifier (OI) based ePDG FQDN.
      static EString nonemergency_epdg_oi_fqdn( const char *mnc, const char *mcc );
      /// @brief Constructs the operator identifier (OI) based ePDG FQDN.
      /// @param plmnid the public land mobile network ID.
      /// @return the operator identifier (OI) based ePDG FQDN.
      static EString nonemergency_epdg_oi_fqdn( const unsigned char *plmnid );
      /// @brief Constructs the tracking area identity based ePDG FQDN.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return he tracking area identity based ePDG FQDN.
      static EString nonemergency_epdg_tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc );
      /// @brief Constructs the tracking area identity based ePDG FQDN.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      /// @return he tracking area identity based ePDG FQDN.
      static EString nonemergency_epdg_tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid );
      /// @brief Constructs the location area code based ePDG FQDN.
      /// @param lac location area code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the location area code based ePDG FQDN.
      static EString nonemergency_epdg_lac_fqdn( const char *lac, const char *mnc, const char *mcc );
      /// @brief Constructs the location area code based ePDG FQDN.
      /// @param lac location area code.
      /// @param plmnid the public land mobile network ID.
      /// @return the location area code based ePDG FQDN.
      static EString nonemergency_epdg_lac_fqdn( const char *lac, const unsigned char *plmnid );
      /// @brief Constructs the visited country FQDN.
      /// @param mcc the mobile country code.
      /// @return the visited country FQDN.
      static EString nonemergency_epdg_visitedcountry_fqdn( const char *mcc );
      /// @brief Constructs the visited country FQDN.
      /// @param plmnid the public land mobile network ID.
      /// @return the visited country FQDN.
      static EString nonemergency_epdg_visitedcountry_fqdn( const unsigned char *plmnid );
      /// @brief Constructs the operator identifier (OI) based emergency ePDG FQDN.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the operator identifier (OI) based emergency ePDG FQDN.
      static EString emergency_epdg_oi_fqdn( const char *mnc, const char *mcc );
      /// @brief Constructs the operator identifier (OI) based emergency ePDG FQDN.
      /// @param plmnid the public land mobile network ID.
      /// @return the operator identifier (OI) based emergency ePDG FQDN.
      static EString emergency_epdg_oi_fqdn( const unsigned char *plmnid );
      /// @brief Constructs the tracking area identity based emergency ePDG FQDN.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the tracking area identity based emergency ePDG FQDN.
      static EString emergency_epdg_tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc );
      /// @brief Constructs the tracking area identity based emergency ePDG FQDN.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      /// @return the tracking area identity based emergency ePDG FQDN.
      static EString emergency_epdg_tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid );
      /// @brief Constructs the location area code based emergency ePDG FQDN.
      /// @param lac location area code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the location area code based emergency ePDG FQDN.
      static EString emergency_epdg_lac_fqdn( const char *lac, const char *mnc, const char *mcc );
      /// @brief Constructs the location area code based emergency ePDG FQDN.
      /// @param lac location area code.
      /// @param plmnid the public land mobile network ID.
      /// @return the location area code based emergency ePDG FQDN.
      static EString emergency_epdg_lac_fqdn( const char *lac, const unsigned char *plmnid );
      /// @brief Constructs the visited country emergency FQDN.
      /// @param mcc the mobile country code.
      /// @return the visited country emergency FQDN.
      static EString emergency_epdg_visitedcountry_fqdn( const char *mcc );
      /// @brief Constructs the visited country emergency FQDN.
      /// @param plmnid the public land mobile network ID.
      /// @return the visited country emergency FQDN.
      static EString emergency_epdg_visitedcountry_fqdn( const unsigned char *plmnid );
      /// @brief Constructs the global eNodeB ID.
      /// @param enb the eNodeB-ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return he global eNodeB ID.
      static EString global_enodeb_id_fqdn( const char *enb, const char *mnc, const char *mcc );
      /// @brief Constructs the global eNodeB ID.
      /// @param enb the eNodeB-ID.
      /// @param plmnid the public land mobile network ID.
      /// @return he global eNodeB ID.
      static EString global_enodeb_id_fqdn( const char *enb, const unsigned char *plmnid );
      /// @brief Constructs the local home network identifier.
      /// @param lhn local home network.
      /// @param mcc the mobile country code.
      /// @return the local home network identifier.
      static EString local_homenetwork_fqdn( const char *lhn, const char *mcc );
      /// @brief Constructs the local home network identifier.
      /// @param lhn local home network.
      /// @param plmnid the public land mobile network ID.
      /// @return the local home network identifier.
      static EString local_homenetwork_fqdn( const char *lhn, const unsigned char *plmnid );
      /// @brief Constructs the home network realm/domain name.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the home network realm/domain name.
      static EString epc( const char *mnc, const char *mcc );
      /// @brief Constructs the home network realm/domain name.
      /// @param plmnid the public land mobile network ID.
      /// @return the home network realm/domain name.
      static EString epc( const unsigned char *plmnid );
      /// @brief Constructs the APN FQDN.
      /// @param apn APN.
      /// @param mnc the mobile network code.
      /// @param mcc the  mcc mobile country code.
      /// @return the APN FQDN.
      static EString apn_fqdn( const char *apn, const char *mnc, const char *mcc );
      /// @brief Constructs the APN FQDN.
      /// @param apn the APN.
      /// @param plmnid the public land mobile network ID.
      /// @return the APN FQDN.
      static EString apn_fqdn( const char *apn, const unsigned char *plmnid );
      /// @brief Constructs the APN.
      /// @param apn the APN.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the APN.
      static EString apn( const char *apn, const char *mnc, const char *mcc );
      /// @brief Constructs the APN.
      /// @param apn the APN.
      /// @param plmnid the public land mobile network ID.
      /// @return the APN.
      static EString apn( const char *apn, const unsigned char *plmnid );
   
      /// @brief Retrieves the AppServiceEnum value for a given string.
      /// @param s the string representation of the application service.
      /// @return the corresponding AppServiceEnum value.
      static AppServiceEnum getAppService( const std::string &s );
      /// @brief Retrieves the AppProtocolEnum value for a given string.
      /// @param p the string representation of the protocol.
      /// @return the corresponding AppProtocolEnum value.
      static AppProtocolEnum getAppProtocol( const std::string &p );

      /// @brief Retrieves the string representation of the specified AppServiceEnum value.
      /// @param s the AppServiceEnum value.
      /// @return the string representation of the specified AppServiceEnum value.
      static const char *getAppService( AppServiceEnum s );
      /// @brief Retrieves the string representation of the specified AppProtocolEnum value.
      /// @param proto the AppProtocolEnum value.
      /// @return the string representation of the specified AppProtocolEnum value.
      static const char *getAppProtocol( AppProtocolEnum proto );
   
      /// @brief Converts a PGWAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( PGWAppProtocolEnum proto );
      /// @brief Converts a SGWAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( SGWAppProtocolEnum proto );
      /// @brief Converts a GGSNAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( GGSNAppProtocolEnum proto );
      /// @brief Converts a SGSNAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( SGSNAppProtocolEnum proto );
      /// @brief Converts a MMEAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( MMEAppProtocolEnum proto );
      /// @brief Converts a MSCAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( MSCAppProtocolEnum proto );
      /// @brief Converts a UPFAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( UPFAppProtocolEnum proto );
      /// @brief Converts a AMFAppProtocolEnum to an AppProtocolEnum value.
      /// @param proto the protocol to convert.
      /// @return the resulting AppProtocolEnum.
      static AppProtocolEnum getAppProtocol( AMFAppProtocolEnum proto );
   
      /// @brief Constructs the Diameter FQDN.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      /// @return the Diameter FQDN.
      static EString diameter_fqdn( const char *mnc, const char *mcc );
      /// @brief Constructs the Diameter FQDN.
      /// @param plmnid the public land mobile network ID.
      /// @return the Diameter FQDN.
      static EString diameter_fqdn( const unsigned char *plmnid );

      /// @brief Converts a DiameterApplicationEnum to the corresponding Diameter application ID.
      /// @param app the DiameterApplicationEnum to convert.
      /// @return the corresponding Diameter application ID.
      static uint32_t getDiameterApplication( DiameterApplicationEnum app );
      /// @brief Converts a DiameterProtocolEnum to the corresponding Diameter protocol string.
      /// @param protocol the DiameterProtocolEnum to convert.
      /// @return the corresponding Diameter protocol string.
      static const char *getDiameterProtocol( DiameterProtocolEnum protocol );

      /// @brief Constructs the Diameter service string.
      /// @param app the DiameterApplicationEnum value.
      /// @param protocol the DiameterProtocolEnum value.
      /// @return the Diameter service string.
      static EString getDiameterService( DiameterApplicationEnum app, DiameterProtocolEnum protocol );

   private:
      Utility() {}
   };
   
   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////
   
   /// @brief Canonical node name.
   class CanonicalNodeName : public std::list<std::string>
   {
   public:
      /// @brief Default constructor.
      CanonicalNodeName();
      /// @brief Class constructor.
      /// @param n the canonical node name.
      CanonicalNodeName( const std::string &n );

      /// @brief Assigns the canonical node name.
      /// @param n the canonical node name.
      Void setName( const std::string &n );

      /// @brief Determines if the "topon" feature is enabled for this object.
      /// @return True indicates "topon" is active, otherwise False.
      Bool getTopon() { return m_topon; }
      /// @brief Retrieves the canonical node name.
      /// @return the canonical node name.
      const EString getName() const { return m_name; }

      /// @brief Performs a topological comparison with another CanonicalNodeName object.
      /// @param right the other canonical node name object.
      /// @return the comparison score.
      int topologicalCompare( const CanonicalNodeName &right );

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
      {
         std::cout << prefix << "Name - " << m_name << std::endl;
         std::cout << prefix << "topon - " << (m_topon?"true":"false") << std::endl;

         std::cout << prefix << "Labels" << std::endl;
         for ( CanonicalNodeName::const_iterator it = begin(); it != end(); ++it )
            std::cout << prefix << "  " << *it << std::endl;
      }

   private:
      Bool m_topon;
      EString m_name;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @cond DOXYGEN_EXCLUDE
   typedef int UsageType;
   /// @endcond

   /// @brief Represents a list of usage types.
   class UsageTypeList : public std::list<UsageType>
   {
   public:
      /// @brief Default constructor.
      UsageTypeList() {}

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @cond DOXYGEN_EXCLUDE
   typedef std::string NetworkCapability;
   /// @endcond

   /// @brief Represents a list of network capabilities.
   class NetworkCapabilityList: public std::list<NetworkCapability>
   {
   public:

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief Represents a vector (array) of strings.
   class StringVector : public std::vector<std::string>
   {
   public:
      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
      {
         for (StringVector::const_iterator it = begin();
              it != end();
              ++it)
         {
            std::cout << prefix << *it << std::endl;
         }
      }

      /// @brief Randomizes the list of strings.
      Void shuffle()
      {
         std::random_shuffle ( begin(), end() );
      }
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Application protocol object.
   class AppProtocol
   {
   public:
      /// @brief Default constructor.
      AppProtocol() { m_protocol = x_unknown; }
      /// @brief Class destructor.
      ~AppProtocol() {}

      /// @brief Retrieves the raw protocol string.
      /// @return the raw protocol string.
      const EString &getRawProtocol() { return m_rawProtocol; }
      /// @brief Retrieves the protocol enumeration.
      /// @return the protocol enumeration.
      AppProtocolEnum getProtocol() { return m_protocol; }
      /// @brief Retrieves the list of usage types.
      /// @return the list of usage types.
      UsageTypeList &getUsageTypes() { return m_usageTypes; }
      /// @brief Retrieves the list of network capabilities.
      /// @return the list of network capabilities.
      NetworkCapabilityList &getNetworkCapabilities() { return m_networkCapabilities; }

      /// @brief Assigns the protocol enumeration.
      /// @param ape the protocol enumeration.
      /// @return the protocol enumeration.
      AppProtocolEnum setProtocol( AppProtocolEnum ape ) { return m_protocol = ape; }
      /// @brief Adds a usage type object to the usage type list.
      /// @param ut the usage type to add.
      /// @return the added usage type.
      UsageType addUsageType( UsageType ut ) { m_usageTypes.push_back( ut ); return ut; }
      /// @brief Adds a network capability to the network capability list.
      /// @param nc the network capability to add.
      /// @return the network capability added.
      NetworkCapability &addNetworkCapability( NetworkCapability &nc ) { m_networkCapabilities.push_back(nc); return m_networkCapabilities.back(); }

      /// @brief Parses any usage types and network capabilities.
      /// @param rp the string to parse.
      Void parse( const std::string &rp );

      /// @brief Determines if a usage type exists in the usage type list.
      /// @param ut the usage type to search for.
      /// @return True if the usage type exists, otherwise False.
      Bool findUsageType( UsageType ut )
      {
         return std::find( m_usageTypes.begin(), m_usageTypes.end(), ut ) != m_usageTypes.end();
      }

      /// @brief Determines if a network capability exists in the network capabilities list.
      /// @param nc the network capability to search for.
      /// @return True if the network capability exists, otherwise False.
      Bool findNetworkCapability( const NetworkCapability &nc )
      {
         return std::find( m_networkCapabilities.begin(), m_networkCapabilities.end(), nc ) != m_networkCapabilities.end();
      }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix = "" )
      {
         std::cout << prefix << "Protocol [" << Utility::getAppProtocol( m_protocol );
         if ( !m_rawProtocol.empty() )
            std::cout << "/" << m_rawProtocol;
         std::cout << "] - [";
         Bool first = true;
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

   /// @brief Manages a list of application protocols.
   class AppProtocolList : public std::list<AppProtocol*>
   {
   public:
      /// @brief Default constructor.
      AppProtocolList() {}
      /// @brief Class destructor.
      ~AppProtocolList()
      {
         while ( !empty() )
         {
            AppProtocol *ap = *begin();
            erase( begin() );
            delete ap;
         }
      }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief The application service object.
   class AppService
   {
   public:
      /// @brief Default constructor.
      AppService() { m_service = x_3gpp_unknown; }
      /// @brief Class constructor.
      /// @param ds the application string to parse.
      AppService( const std::string &ds ) { parse( ds ); }
      /// @brief Class destructor.
      ~AppService()
      {
         while ( !m_protocols.empty() )
         {
            AppProtocol *ap = *m_protocols.begin();
            m_protocols.pop_front();
            delete ap;
         }
      }

      /// @brief Retrieves the application service type.
      /// @return the application service type.
      AppServiceEnum getService() { return m_service; }
      /// @brief Retrieves the list of application protocols.
      /// @return the list of application protocols.
      AppProtocolList &getProtocols() { return m_protocols; }
      /// @brief Retrieves the raw application service string.
      /// @return the raw application service string.
      EString &getRawService() { return m_rawService; }

      /// @brief Parses an application service string.
      /// @param ds the application service string to parse.
      Void parse( const std::string &ds );

      /// @brief Searches for a specific application protocol object.
      /// @param protocol the application protocol to search for.
      /// @return pointer to the application protocol object if found, otherwise NULL.
      AppProtocol *findProtocol( AppProtocolEnum protocol )
      {
         for ( AppProtocolList::const_iterator it = m_protocols.begin(); it != m_protocols.end(); ++it )
         {
            if ( (*it)->getProtocol() == protocol )
               return *it;
         }

         return NULL;
      }

      /// @brief Prints the contents of this object.
      Void dump()
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

   /// @brief Contains the node selector results.
   class NodeSelectorResult
   {
   public:
      /// @brief Default constructor.
      NodeSelectorResult()
      {
         m_order = 0;
         m_preference = 0;
         m_port = 0;
      }

      /// @brief Retrieves the host name.
      /// @return the host name.
      const EString &getHostname() { return m_hostname; }
      /// @brief Retrieves the order value.
      /// @return the order value.
      uint16_t getOrder() { return m_order; }
      /// @brief Retrieves the preference value.
      /// @return the preference value.
      uint16_t getPreference() { return m_preference; }
      /// @brief Retrieves the IP port value.
      /// @return the IP port value.
      uint16_t getPort() { return m_port; }
      /// @brief Retrieves the list supported protocols.
      /// @return the list supported protocols.
      AppProtocolList &getSupportedProtocols() { return m_supported_protocols; }
      /// @brief Retrieves the list of IPv4 hosts.
      /// @return the list of IPv4 hosts.
      StringVector &getIPv4Hosts() { return m_ipv4_hosts; }
      /// @brief Retrieves the list of IPv6 hosts.
      /// @return the list of IPv6 hosts.
      StringVector &getIPv6Hosts() { return m_ipv6_hosts; }

      /// @brief Assigns the order value.
      /// @param order the order value.
      /// @return the order value.
      uint16_t setOrder( uint16_t order ) { return m_order = order; }
      /// @brief Assigns the preference value.
      /// @param preference the preference value.
      /// @return the preference value.
      uint16_t setPreference( uint16_t preference ) { return m_preference = preference; }
      /// @brief Assigns the IP port.
      /// @param port the IP port.
      /// @return the IP port.
      uint16_t setPort( uint16_t port ) { return m_port = port; }
      /// @brief Assigns the host name.
      /// @param hostname the host name.
      /// @return the host name.
      const EString &setHostname( const std::string &hostname ) { m_hostname = hostname; return m_hostname; }
      /// @brief Adds the application object to the list.
      /// @param ap the application object to be added to the list.
      Void addSupportedProtocol( AppProtocol *ap ) { m_supported_protocols.push_back( ap ); }
      /// @brief Adds an IPv4 host to the list of IPv4 hosts.
      /// @param host the IPv4 host to add.
      Void addIPv4Host( const std::string &host ) { m_ipv4_hosts.push_back( host ); }
      /// @brief Adds an IPv6 host to the list of IPv6 hosts.
      /// @param host the IPv6 host to add.
      Void addIPv6Host( const std::string &host ) { m_ipv6_hosts.push_back( host ); }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief A list of node selector result objects.
   class NodeSelectorResultList : public std::list<NodeSelectorResult*>
   {
   public:
      /// @brief Default constructor.
      NodeSelectorResultList() {}
      /// @brief Class destructor.
      ~NodeSelectorResultList()
      {
         while ( !empty() )
         {
            NodeSelectorResult* nsr = *begin();
            erase( begin() );
            delete nsr;
         }
      }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
      {
         for (NodeSelectorResultList::const_iterator it = begin();
              it != end();
              ++it)
         {
            (*it)->dump( prefix );
         }
      }

      /// @brief A comparison function used to sort the members of the list.
      /// @param first the first NodeSelectorResult object.
      /// @param second the second NodeSelectorResult object.
      /// @return True if the first value is less than the second value, otherwise False.
      static Bool sort_compare( NodeSelectorResult*& first, NodeSelectorResult*& second );
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief contains the information to select a node.
   class NodeSelector
   {
   public:
      /// @brief Retrieves the named server ID.
      /// @return the named server ID.
      DNS::namedserverid_t getNamedServerID() { return m_nsid; }
      /// @brief Assigns the named server ID.
      /// @param nsid the named server ID.
      /// @return the named server ID.
      DNS::namedserverid_t setNamedServerID(DNS::namedserverid_t nsid) { return m_nsid = nsid; }
   
      /// @brief Retrieves the desired application service type.
      /// @return the desired application service type.
      AppServiceEnum getDesiredService() { return m_desiredService; }
      /// @brief Retrieves the list of application protocols.
      /// @return the list of application protocols.
      AppProtocolList &getDesiredProtocols() { return m_desiredProtocols; }
      /// @brief Retrieves the domain name.
      /// @return the domain name.
      const EString &getDomainName() { return m_domain; }
      /// @brief Retrieves the node selector results list.
      /// @return the node selector results list.
      NodeSelectorResultList &getResults() { return m_results; }
   
      /// @brief Adds a desired usage type to the list of desired usage types.
      /// @param ut the usage type to add.
      /// @return the usage type added.
      UsageType addDesiredUsageType( UsageType ut ) { m_desiredUsageTypes.push_back( ut ); return ut; }
      /// @brief Adds a desired network capability to the list of desired network capabilities.
      /// @param nc the desired network capability to add.
      /// @return the desired network capability to added.
      NetworkCapability &addDesiredNetworkCapability( const char *nc ) { EString s(nc); m_desiredNetworkCapabilities.push_back( s ); return m_desiredNetworkCapabilities.back(); }
      /// @brief Adds a desired network capability to the list of desired network capabilities.
      /// @param nc the desired network capability to add.
      /// @return the desired network capability to added.
      NetworkCapability &addDesiredNetworkCapability( NetworkCapability &nc ) { m_desiredNetworkCapabilities.push_back( nc ); return nc; }

      /// @brief Performs the selection process.
      /// @return the node selector results.
      NodeSelectorResultList &process();

      /// @brief Prints the contents of this object.
      Void dump()
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
      /// @brief Default constructor.
      NodeSelector();
      /// @brief Class destructor.
      ~NodeSelector();
   
      /// @brief Assigns the application service type.
      /// @param s the application service type.
      /// @return the application service type.
      AppServiceEnum setAppService( AppServiceEnum s ) { return m_desiredService = s; }
      /// @brief Assigns the domain name.
      /// @param d the domain name.
      /// @return the domain name.
      const EString &setDomainName( const std::string &d ) { m_domain = d; return m_domain; }
      /// @brief Adds a desired application protcol to the desired protocol list.
      /// @param ap the desired application protocol.
      Void addDesiredProtocol( AppProtocol *ap ) { m_desiredProtocols.push_back( ap ); }
   
   private:
      AppServiceEnum parseService( const std::string &service, std::list<AppProtocolEnum> &protocols ) const;
      static Bool naptr_compare( DNS::RRecordNAPTR*& first, DNS::RRecordNAPTR*& second );
   
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
   
   /// @brief Represents a colocated candidate.
   class ColocatedCandidate
   {
   public:
      /// @brief Pair type.
      enum PairType
      {
         /// unknown
         ptUnknown = 0,
         /// colocated
         ptColocated = 1,
         /// topological distance
         ptTopologicalDistance = 2,
         /// DNS priority
         ptDNSPriority = 3
      };

      /// @brief Class constructor.
      /// @param candidate1 the first candidate.
      /// @param candidate2 the second candidate.
      ColocatedCandidate( NodeSelectorResult &candidate1, NodeSelectorResult &candidate2 );

      /// @brief Retrieves the first candidate node selector result object.
      /// @return the first candidate node selector result object.
      NodeSelectorResult &getCandidate1() { return m_candidate1; }
      /// @brief Retrieves the second candidate node selector result object.
      /// @return the second candidate node selector result object.
      NodeSelectorResult &getCandidate2() { return m_candidate2; }
      /// @brief Retrieves the first canonical node name.
      /// @return the first canonical node name.
      CanonicalNodeName &getCanonicalNodeName1() { return m_cnn1; }
      /// @brief Retrieves the second canonical node name.
      /// @return the second canonical node name.
      CanonicalNodeName &getCanonicalNodeName2() { return m_cnn2; }
      /// @brief Retrieves the pair type.
      /// @return the pair type.
      PairType getPairType() { return m_pairtype; }
      /// @brief Retrieves the number of topological matches.
      /// @return the number of topological matches.
      int getTopologicalMatches() { return m_topologicalMatches; }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief A list of colocated candidates.
   class ColocatedCandidateList : public std::list<ColocatedCandidate*>
   {
   public:
      /// @brief Class constructor.
      /// @param nodelist1 the first list of node selection results.
      /// @param nodelist2 the second list of node selection results.
      ColocatedCandidateList( NodeSelectorResultList &nodelist1, NodeSelectorResultList &nodelist2 );
      /// @brief Class destructor.
      ~ColocatedCandidateList();

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix = "" )
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

      static Bool sort_compare( ColocatedCandidate*& first, ColocatedCandidate*& second );

      NodeSelectorResultList &m_nodelist1;
      NodeSelectorResultList &m_nodelist2;
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief A MME node selector.
   class MMENodeSelector : public NodeSelector
   {
   public:
      
      /// @brief Class constructor.
      /// @param mmec MME code.
      /// @param mmegi MME Group ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      MMENodeSelector( const char *mmec, const char *mmegi, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_fqdn( mmec, mmegi, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param mmec MME code.
      /// @param mmegi MME Group ID.
      /// @param plmnid the public land mobile network ID.
      MMENodeSelector( const char *mmec, const char *mmegi, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_fqdn( mmec, mmegi, plmnid ) );
      }
   
      /// @brief Class constructor.
      /// @param mmegi MME Group ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      MMENodeSelector( const char *mmegi, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_pool_fqdn( mmegi, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param mmegi MME Group ID.
      /// @param plmnid the public land mobile network ID.
      MMENodeSelector( const char *mmegi, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_mme );
         setDomainName( Utility::mme_pool_fqdn( mmegi, plmnid ) );
      }

      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
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

   /// @brief A PGW node selector.
   class PGWNodeSelector : public NodeSelector
   {
   public:
      /// @brief Class constructor.
      /// @param apn APN.
      /// @param mnc the mobile network code.
      /// @param mcc the  mcc mobile country code.
      PGWNodeSelector( const char *apn, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_pgw );
         setDomainName( Utility::apn_fqdn( apn, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param apn the APN.
      /// @param plmnid the public land mobile network ID.
      PGWNodeSelector( const char *apn, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_pgw );
         setDomainName( Utility::apn_fqdn( apn, plmnid ) );
      }
   
      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
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

   /// @brief A PGW user plane node selector.
   class PGWUPFNodeSelector : public NodeSelector
   {
   public:
      /// @brief Class constructor.
      /// @param apn APN.
      /// @param mnc the mobile network code.
      /// @param mcc the  mcc mobile country code.
      PGWUPFNodeSelector( const char *apn, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::apn_fqdn( apn, mnc, mcc ) );
      }

      /// @brief Class constructor.
      /// @param apn the APN.
      /// @param plmnid the public land mobile network ID.
      PGWUPFNodeSelector( const char *apn, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::apn_fqdn( apn, plmnid ) );
      }

      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
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

   /// @brief A SGW node selector.
   class SGWNodeSelector : public NodeSelector
   {
   public:
      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      SGWNodeSelector( const unsigned char lb, const unsigned char hb, const char *mnc, const char *mcc )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      SGWNodeSelector( unsigned char lb, unsigned char hb, const unsigned char *plmnid )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, plmnid ) );
      }
   
      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      SGWNodeSelector( const char *lb, const char *hb, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lb, hb, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      SGWNodeSelector( const char *lb, const char *hb, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::tai_fqdn( lb, hb, plmnid ) );
      }

      /// @brief Class constructor.
      /// @param node the node name.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      SGWNodeSelector( const char *node, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::epc_node_fqdn( node, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param node the node name.
      /// @param plmnid the public land mobile network ID.
      SGWNodeSelector( const char *node, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_sgw );
         setDomainName( Utility::epc_node_fqdn( node, plmnid ) );
      }
   
      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
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

   /// @brief A SGW user plane node selector.
   class SGWUPFNodeSelector : public NodeSelector
   {
   public:
      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      SGWUPFNodeSelector( const unsigned char lb, const unsigned char hb, const char *mnc, const char *mcc )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, mnc, mcc ) );
      }

      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      SGWUPFNodeSelector( unsigned char lb, unsigned char hb, const unsigned char *plmnid )
      {
         char lbbuf[3];
         char hbbuf[3];

         sprintf( lbbuf, "%02x", lb );
         sprintf( hbbuf, "%02x", hb );

         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lbbuf, hbbuf, plmnid ) );
      }

      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      SGWUPFNodeSelector( const char *lb, const char *hb, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lb, hb, mnc, mcc ) );
      }

      /// @brief Class constructor.
      /// @param lb low byte of the type allocation code.
      /// @param hb high byte of the type allocation code.
      /// @param plmnid the public land mobile network ID.
      SGWUPFNodeSelector( const char *lb, const char *hb, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::tai_fqdn( lb, hb, plmnid ) );
      }

      /// @brief Class constructor.
      /// @param node the node name.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      SGWUPFNodeSelector( const char *node, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::epc_node_fqdn( node, mnc, mcc ) );
      }

      /// @brief Class constructor.
      /// @param node the node name.
      /// @param plmnid the public land mobile network ID.
      SGWUPFNodeSelector( const char *node, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::epc_node_fqdn( node, plmnid ) );
      }

      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
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

   /// @brief An eNodeB user plane node selector.
   class ENodeBUPFNodeSelector : public NodeSelector
   {
   public:
      /// @brief Class constructor.
      /// @param enb the eNodeB-ID.
      /// @param mnc the mobile network code.
      /// @param mcc the mobile country code.
      ENodeBUPFNodeSelector( const char *enb, const char *mnc, const char *mcc )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::global_enodeb_id_fqdn( enb, mnc, mcc ) );
      }

      /// @brief Class constructor.
      /// @param enb the eNodeB-ID.
      /// @param plmnid the public land mobile network ID.
      ENodeBUPFNodeSelector( const char *enb, const unsigned char *plmnid )
      {
         setAppService( x_3gpp_upf );
         setDomainName( Utility::global_enodeb_id_fqdn( enb, plmnid ) );
      }

      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
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

    /// @brief A EPC node selector.
   class EpcNodeSelector : public NodeSelector
   {
   public:
      /// @brief Class constructor.
      /// @param node the node name.
      /// @param mnc the mobile network code.
      /// @param mcc the  mcc mobile country code.
      /// @param appsvc the desired application service.  Defaults to x_3gpp_any.
      EpcNodeSelector( const char *node, const char *mnc, const char *mcc, AppServiceEnum appsvc = x_3gpp_any )
      {
         NodeSelector::setAppService( appsvc );
         setDomainName( Utility::epc_node_fqdn( node, mnc, mcc ) );
      }
   
      /// @brief Class constructor.
      /// @param node the node name.
      /// @param plmnid the public land mobile network ID.
      /// @param appsvc the desired application service.  Defaults to x_3gpp_any.
      EpcNodeSelector( const char *node, const unsigned char *plmnid, AppServiceEnum appsvc = x_3gpp_any )
      {
         NodeSelector::setAppService( appsvc );
         setDomainName( Utility::epc_node_fqdn( node, plmnid ) );
      }
   
      /// @brief Class constructor.
      /// @param fqdn the node name.
      /// @param appsvc the desired application service.  Defaults to x_3gpp_any.
      EpcNodeSelector( const char *fqdn, AppServiceEnum appsvc = x_3gpp_any )
      {
         NodeSelector::setAppService( appsvc );
         setDomainName( fqdn );
      }
   
      /// @brief Adds the desired application protocol to the selector.
      /// @param p the applciation protocol to add.
      /// @return the added application protocol object.   
      AppProtocol *addDesiredProtocol( PGWAppProtocolEnum p )
      {
         AppProtocol *ap = new AppProtocol();
         ap->setProtocol( Utility::getAppProtocol( p ) );
         NodeSelector::addDesiredProtocol( ap );
         return ap;
      }

      /// @brief Assigns the required application service type.
      /// @param s the desired application service.
      /// @return a reference to this object.
      EpcNodeSelector &setAppService( AppServiceEnum s )
      {
         NodeSelector::setAppService( s );
         return *this;
      }
   
   private:
      EpcNodeSelector() {}
   };
   
   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

  /// @brief Diameter name pointer (NAPTR) type
   enum DiameterNaptrTypeEnum
   {
      /// unknown
      dnt_unknown,
      /// host name
      dnt_hostname,
      /// service
      dnt_service
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief Diameter name pointer (NAPTR) object.
   class DiameterNaptr
   {
   public:
      /// @brief Class constructor.
      /// @param drt the NAPTR type.
      DiameterNaptr( DiameterNaptrTypeEnum drt )
         : m_type( drt )
      {
      }

      /// @brief Class destructor.
      virtual ~DiameterNaptr()
      {
      }

      /// @brief Retrieves the NAPTR type.
      /// @return the NAPTR type.
      DiameterNaptrTypeEnum getType() { return m_type; }

      /// @brief Retrieves the order value.
      /// @return the order value.
      uint16_t getOrder() { return m_order; }
      /// @brief Assigns the order value.
      /// @param order the order value.
      /// @return the order value.
      uint16_t setOrder( uint16_t order ) { return m_order = order; }

      /// @brief Retrieves the preference value.
      /// @return the preference value.
      uint16_t getPreference() { return m_order; }
      /// @brief Assigns the preference value.
      /// @param pref the preference value.
      /// @return the preference value.
      uint16_t setPreference( uint16_t pref ) { return m_preference = pref; }

      /// @brief Retrieves the service value.
      /// @return the service value.
      const EString &getService() { return m_service; }
      /// @brief Assigns the service value.
      /// @param service the service value.
      /// @return the service value.
      const EString &setService( const char *service ) { return m_service = service; }
      /// @brief Assigns the service value.
      /// @param service the service value.
      /// @return the service value.
      const EString &setService( const std::string &service ) { return setService( service.c_str() ); }

      /// @brief Retrieves the replacement value.
      /// @return the replacement value.
      const EString &getReplacement() { return m_replacement; }
      /// @brief Assigns the replacement value.
      /// @param replacement the replacement value.
      /// @return the replacement value.
      const EString &setReplacement( const char *replacement ) { return m_replacement = replacement; }
      /// @brief Assigns the replacement value.
      /// @param replacement the replacement value.
      /// @return the replacement value.
      const EString &setReplacement( const std::string &replacement ) { return setReplacement( replacement.c_str() ); }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief Represents a diameter host.
   class DiameterHost
   {
   public:
      /// @brief Default constructor.
      DiameterHost() {}
      /// @brief Class constructor.
      /// @param name the Diameter host name.
      DiameterHost( const char *name ) : m_name( name ) {}
      /// @brief Class constructor.
      /// @param name the Diameter host name.
      DiameterHost( const std::string &name ) : m_name( name ) {}

      /// @brief Retrieves the Diameter host name.
      /// @return the Diameter host name.
      const EString &getName() const { return m_name; }
      /// @brief Assigns the Diameter host name.
      /// @param name the Diameter host name.
      /// @return the Diameter host name.
      const EString &setName( const char *name ) { return m_name = name; }
      /// @brief Assigns the Diameter host name.
      /// @param name the Diameter host name.
      /// @return the Diameter host name.
      const EString &setName( const std::string &name ) { return setName( name.c_str() ); }

      /// @brief Retrieves the list of IPv4 addresses for this Diameter host.
      /// @return the list of IPv4 addresses for this diameter host.
      StringVector &getIPv4Addresses() { return m_ipv4_addrs; }
      /// @brief Retrieves the list of IPv6 addresses for this Diameter host.
      /// @return the list of IPv6 addresses for this Diameter host.
      StringVector &getIPv6Addresses() { return m_ipv6_addrs; }

      /// @brief Adds an IPv4 address for this Diameter host.
      /// @param addr an IPv4 address for this Diameter host.
      Void addIPv4Address( const std::string &addr ) { m_ipv4_addrs.push_back( addr ); }
      /// @brief Adds an IPv6 address for this Diameter host.
      /// @param addr an IPv6 address for this Diameter host.
      Void addIPv6Address( const std::string &addr ) { m_ipv6_addrs.push_back( addr ); }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief Represents a Diameter server.
   class DiameterSrv
   {
   public:
      /// @brief Default constructor.
      DiameterSrv() {}
      /// @brief Class destructor.
      ~DiameterSrv() {}

      /// @brief Retrieves the priority value.
      /// @return the priority value.
      uint16_t getPriority() { return m_priority; }
      /// @brief Assigns the priority value.
      /// @param priority the priority value.
      /// @return the priority value.
      uint16_t setPriority( uint16_t priority ) { return m_priority = priority; }

      /// @brief Retrieves the weight value.
      /// @return the weight value.
      uint16_t getWeight() { return m_weight; }
      /// @brief Assigns the weight value.
      /// @param weight the weight value.
      /// @return the weight value.
      uint16_t setWeight( uint16_t weight ) { return m_weight = weight; }

      /// @brief Retrieves the IP port.
      /// @return the IP port.
      uint16_t getPort() { return m_port; }
      /// @brief Assigns the IP port.
      /// @param port the IP port.
      /// @return the IP port.
      uint16_t setPort( uint16_t port ) { return m_port = port; }

      /// @brief Retrieves the diameter host.
      /// @return the diameter host.
      DiameterHost &getHost() { return m_host; }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief Represents a vector (array) of Diameter servers.
   class DiameterSrvVector : public std::vector<DiameterSrv*>
   {
   public:
      /// @brief Class destructor.
      ~DiameterSrvVector()
      {
         while ( !empty() )
         {
            DiameterSrv *ds = *begin();
            erase( begin() );
            delete ds;
         }
      }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

      /// @brief Sorts the vector (array) of Diameter servers.
      Void sort_vector();

   private:
      static Bool sort_compare( DiameterSrv* first, DiameterSrv* second );
   };

   /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////

   /// @brief A Diameter NAPTR A record object.
   class DiameterNaptrA : public DiameterNaptr
   {
   public:
      /// @brief Default constructor.
      DiameterNaptrA() : DiameterNaptr( dnt_hostname ) {}

      /// @brief Retrieves the Diameter host.
      /// @return the Diameter host.
      DiameterHost &getHost() { return m_host; }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief A Diameter NAPTR S record object.
   class DiameterNaptrS : public DiameterNaptr
   {
   public:
      /// @brief Default constructor.
      DiameterNaptrS() : DiameterNaptr( dnt_service ) {}

      /// @brief Retrieves the Diameter server.
      /// @return the Diameter server.
      DiameterSrvVector &getSrvs() { return m_srv; }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix )
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

   /// @brief A list of Diameter NAPTR objects.
   class DiameterNaptrList : public std::list<DiameterNaptr*>
   {
   public:
      /// @brief Class destructor.
      ~DiameterNaptrList()
      {
         while ( !empty() )
         {
            DiameterNaptr *n = *begin();
            erase( begin() );
            delete n;
         }
      }

      /// @brief Prints the contents of this object.
      /// @param prefix a value to prepend to each line.
      Void dump( const char *prefix = "" )
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

   /// @brief Represents a Diameter selector.
   class DiameterSelector
   {
   public:
      /// @brief Default constructor.
      DiameterSelector();

      /// @brief Retrieves the realm.
      /// @return the realm.
      const EString &getRealm() { return m_realm; }
      /// @brief Assigns the realm.
      /// @param realm the realm.
      /// @return the realm.
      const EString &setRealm( const char *realm ) { return m_realm = realm; }
      /// @brief Assigns the realm.
      /// @param mnc mobile network code.
      /// @param mcc mobile country code.
      /// @return the realm.
      const EString &setRealm( const char *mnc, const char *mcc ) { return m_realm = Utility::diameter_fqdn( mnc, mcc ); }
      /// @brief Assigns the realm.
      /// @param plmnid the public land mobile netowrk ID.
      /// @return the realm.
      const EString &setRealm( const unsigned char *plmnid ) { return m_realm = Utility::diameter_fqdn( plmnid ); }

      /// @brief Retrieves the application type.
      /// @return the application type.
      DiameterApplicationEnum getApplicataion() { return m_application; }
      /// @brief Assigns the applciation type.
      /// @param app the application type.
      /// @return the application type.
      DiameterApplicationEnum setApplication( DiameterApplicationEnum app ) { return m_application = app; }

      /// @brief Retrieves the protocol type.
      /// @return the protocol type.
      DiameterProtocolEnum getProtocol() { return m_protocol; }
      /// @brief Assigns the protocol type.
      /// @param proto the protocol type.
      /// @return the protocol type.
      DiameterProtocolEnum setProtocol( DiameterProtocolEnum proto ) { return m_protocol = proto; }

      /// @brief Performs the lookup of the Diameter hosts.
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
