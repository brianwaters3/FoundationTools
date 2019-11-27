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

#include <stdlib.h>
#include <iostream>

#include "epcdns.h"

using namespace EPCDNS;

EString Utility::home_network( const char *mnc, const char *mcc )
{
   EString s;

   // '^mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::home_network( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return home_network( mnc, mcc );
}

EString Utility::home_network_gprs( const char *mnc, const char *mcc )
{
   EString s;

   // '^mnc(\d{3})\.mcc(\d{3})\.gprs'
   s.APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "gprs" );

   return s;
}

EString Utility::home_network_gprs( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return home_network_gprs( mnc, mcc );
}

EString Utility::tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc )
{
   EString s;

   // '^tac-lb([0-9a-fA-F]{2})\.tac-hb([0-9a-fA-F]{2})\.tac\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "tac-lb" )
    .append( lb )
    .append( ".tac-hb" )
    .append( hb )
    .append( ".tac.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return tai_fqdn( lb, hb, mnc, mcc );
}

EString Utility::mme_fqdn( const char *mmec, const char *mmegi, const char *mnc, const char *mcc )
{
   EString s;

   // '^mmec([0-9a-fA-F]{2})\.mmegi([0-9a-fA-F]{4})\.mme.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "mmec" )
    .append( mmec )
    .append( ".mmegi" )
    .append( mmegi )
    .append( ".mme.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::mme_fqdn( const char *mmec, const char *mmegi, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return mme_fqdn( mmec, mmegi, mnc, mcc );
}

EString Utility::mme_pool_fqdn( const char *mmegi, const char *mnc, const char *mcc )
{
   EString s;

   // '^mmegi([0-9a-fA-F]{4})\.mme\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "mmegi" )
    .append( mmegi )
    .append( ".mme.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::mme_pool_fqdn( const char *mmegi, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return mme_pool_fqdn( mmegi, mnc, mcc );
}

EString Utility::rai_fqdn( const char *rac, const char *lac, const char *mnc, const char *mcc )
{
   EString s;

   // '^rac([0-9a-fA-F]{4})\.lac([0-9a-fA-F]{4})\.rac\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "rac" )
    .append( rac )
    .append( ".lac" )
    .append( lac )
    .append( ".rac.epc." )
       .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::rai_fqdn( const char *rac, const char *lac, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return rai_fqdn( rac, lac, mnc, mcc );
}

EString Utility::rnc_fqdn( const char *rnc, const char *mnc, const char *mcc )
{
   EString s;

   // '^rnc([0-9a-fA-F]{4})\.rnc\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "rnc" )
    .append( rnc )
    .append( ".rnc.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::rnc_fqdn( const char *rnc, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return rnc_fqdn( rnc, mnc, mcc );
}

EString Utility::sgsn_fqdn( const char *nri, const char *rac, const char *lac, const char *mnc, const char *mcc )
{
   EString s;

   // '^nri([0-9a-fA-F]{4})\.rac([0-9a-fA-F]{4})\.lac([0-9a-fA-F]{4})\.rac\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "nri" )
    .append( nri )
    .append( ".rac" )
    .append( rac )
    .append( ".lac" )
    .append( lac )
    .append( ".rac.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::sgsn_fqdn( const char *nri, const char *rac, const char *lac, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return sgsn_fqdn( nri, rac, lac, mnc, mcc );
}

EString Utility::epc_nodes_domain_fqdn( const char *mnc, const char *mcc )
{
   EString s;

   // '^node\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "node.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::epc_nodes_domain_fqdn( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return epc_nodes_domain_fqdn( mnc, mcc );
}

EString Utility::epc_node_fqdn( const char *node, const char *mnc, const char *mcc )
{
   EString s;

   // '^(.+)\.node\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org$'
   s.append( node )
    .append( ".node.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::epc_node_fqdn( const char *node, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return epc_node_fqdn( node, mnc, mcc );
}

EString Utility::nonemergency_epdg_oi_fqdn( const char *mnc, const char *mcc )
{
   EString s;

   // '^epdg\.epc\.mnc(\d{3})\.mcc(\d{3})\.pub\.3gppnetworks\.org'
   s.append( "epdg.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "pub." )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::nonemergency_epdg_oi_fqdn( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return nonemergency_epdg_oi_fqdn( mnc, mcc );
}

EString Utility::nonemergency_epdg_tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc )
{
   EString s;

   // '^tac-lb([0-9a-fA-F]{2})\.tac-hb([0-9a-fA-F]{2})\.tac\.epdg\.epc\.mnc(\d{3})\.mcc(\d{3})\.pub\.3gppnetworks\.org'
   s.append( "tac-lb" )
    .append( lb )
    .append( ".tac-hb" )
    .append( hb )
    .append( ".tac.epdg.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::nonemergency_epdg_tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return nonemergency_epdg_tai_fqdn( lb, hb, mnc, mcc );
}

EString Utility::nonemergency_epdg_lac_fqdn( const char *lac, const char *mnc, const char *mcc )
{
   EString s;

   // '^lac([0-9a-fA-F]{4})\.epdg\.epc\.mnc(\d{3})\.mcc(\d{3})\.pub\.3gppnetworks\.org'
   s.append( "lac" )
    .append( lac )
    .append( ".epdg.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "pub." )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::nonemergency_epdg_lac_fqdn( const char *lac, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return nonemergency_epdg_lac_fqdn( lac, mnc, mcc );
}

EString Utility::nonemergency_epdg_visitedcountry_fqdn( const char *mcc )
{
   EString s;

   // '^epdg\.epc\.mcc(\d{3})\.visited-country\.pub\.3gppnetworks\.org'
   s.append( "epdg.epc." )
    .APPEND_MCC( mcc )
    .append( "visited-country.pub." )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::nonemergency_epdg_visitedcountry_fqdn( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return nonemergency_epdg_visitedcountry_fqdn( mcc );
}

EString Utility::emergency_epdg_oi_fqdn( const char *mnc, const char *mcc )
{
   EString s;

   // '^sos\.epdg\.epc\.mnc(\d{3})\.mcc(\d{3})\.pub\.3gppnetworks\.org'
   s.append( "sos.epdg.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "pub." )
    .APPEND_3GPPNETWORK;

   return s;
}

EString Utility::emergency_epdg_oi_fqdn( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return emergency_epdg_oi_fqdn( mnc, mcc );
}

EString Utility::emergency_epdg_tai_fqdn( const char *lb, const char *hb, const char *mnc, const char *mcc )
{
   EString s;

   // '^tac-lb([0-9a-fA-F]{2})\.tac-hb([0-9a-fA-F]{2})\.tac\.sos\.epdg\.epc\.mnc(\d{3})\.mcc(\d{3})\.pub\.3gppnetworks\.org',
   s.append( "tac-lb" )
    .append( lb )
    .append( ".tac-hb" )
    .append( hb )
    .append( ".tac.sos.epdg.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "pub." )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::emergency_epdg_tai_fqdn( const char *lb, const char *hb, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return emergency_epdg_tai_fqdn( lb, hb, mnc, mcc );
}

EString Utility::emergency_epdg_lac_fqdn( const char *lac, const char *mnc, const char *mcc )
{
   EString s;

   // '^lac([0-9a-fA-F]{4})\.sos\.epdg\.epc\.mnc(\d{3})\.mcc(\d{3})\.pub\.3gppnetworks\.org'
   s.append( "lac" )
    .append( lac )
    .append( ".sos.epdg.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "pub." )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::emergency_epdg_lac_fqdn( const char *lac, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return emergency_epdg_lac_fqdn( lac, mnc, mcc );
}

EString Utility::emergency_epdg_visitedcountry_fqdn( const char *mcc )
{
   EString s;

   // '^sos\.epdg\.epc\.mcc(\d{3})\.visited-country\.pub\.3gppnetworks\.org'
   s.append( "sos.epdg.epc." )
    .APPEND_MCC( mcc )
    .append( "visited-country.pub." )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::emergency_epdg_visitedcountry_fqdn( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return emergency_epdg_visitedcountry_fqdn( mcc );
}

EString Utility::global_enodeb_id_fqdn( const char *enb, const char *mnc, const char *mcc )
{
   EString s;

   // '^enb([0-9a-fA-F]{4})\.enb\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "enb" )
    .append( enb )
    .append( ".enb.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::global_enodeb_id_fqdn( const char *enb, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return global_enodeb_id_fqdn( enb, mnc, mcc );
}

EString Utility::local_homenetwork_fqdn( const char *lhn, const char *mcc )
{
   EString s;

   // '^lhn(.+)\.lhn\.epc\.mcc(\d{3})\.visited-country\.pub\.3gppnetworks\.org$'
   s.append( "lhn" )
    .append( lhn )
    .append( ".lhn.epc." )
    .APPEND_MCC( mcc )
    .append( "visited-country.pub." )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::local_homenetwork_fqdn( const char *lhn, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return local_homenetwork_fqdn( lhn, mcc );
}

EString Utility::epc( const char *mnc, const char *mcc )
{
   EString s;

   // '^epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org'
   s.append( "epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::epc( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return epc( mnc, mcc );
}

EString Utility::apn_fqdn( const char *apn, const char *mnc, const char *mcc )
{
   EString s;

   // '(.+)\.apn\.epc\.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org$'
   s.append( apn )
    .append( ".apn.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::apn_fqdn( const char *apn, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return apn_fqdn( apn, mnc, mcc );
}

EString Utility::apn( const char *_apn, const char *mnc, const char *mcc )
{
   EString s;

   // '(.+)\.apn\.mnc(\d{3})\.mcc(\d{3})\.gprs$'} 
   s.append( _apn )
    .append( ".apn." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .append( "gprs" ); 

   return s;
}

EString Utility::apn( const char *_apn, const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return apn( _apn, mnc, mcc );
}

AppServiceEnum Utility::getAppService( const std::string &s )
{
   return
      s == "x-3gpp-pgw"  ? x_3gpp_pgw  :
      s == "x-3gpp-sgw"  ? x_3gpp_sgw  :
      s == "x-3gpp-ggsn" ? x_3gpp_ggsn :
      s == "x-3gpp-sgsn" ? x_3gpp_sgsn :
      s == "x-3gpp-mme"  ? x_3gpp_mme  :
      s == "x-3gpp-msc"  ? x_3gpp_msc  :
      s == "x-3gpp-upf"  ? x_3gpp_upf  :
      s == "x-3gpp-amf"  ? x_3gpp_amf  : x_3gpp_unknown;
}

AppProtocolEnum Utility::getAppProtocol( const std::string &p )
{
   return
      p == "x-gn"        ? x_gn        :
      p == "x-gp"        ? x_gp        :
      p == "x-n2"        ? x_n2        :
      p == "x-nq"        ? x_nq        :
      p == "x-nqprime"   ? x_nqprime   :
      p == "x-s1-mme"    ? x_s1_mme    :
      p == "x-s1-u"      ? x_s1_u      :
      p == "x-s10"       ? x_s10       :
      p == "x-s11"       ? x_s11       :
      p == "x-s12"       ? x_s12       :
      p == "x-s16"       ? x_s16       :
      p == "x-s2a-gtp"   ? x_s2a_gtp   :
      p == "x-s2a-mipv4" ? x_s2a_mipv4 :
      p == "x-s2a-pmip"  ? x_s2a_pmip  :
      p == "x-s2b-gtp"   ? x_s2b_gtp   :
      p == "x-s2b-pmip"  ? x_s2b_pmip  :
      p == "x-s2c-dsmip" ? x_s2c_dsmip :
      p == "x-s3"        ? x_s3        :
      p == "x-s4"        ? x_s4        :
      p == "x-s5-gtp"    ? x_s5_gtp    :
      p == "x-s5-pmip"   ? x_s5_pmip   :
      p == "x-s6a"       ? x_s6a       :
      p == "x-s8-gtp"    ? x_s8_gtp    :
      p == "x-s8-pmip"   ? x_s8_pmip   :
      p == "x-sv"        ? x_sv        :
      p == "x-sxa"       ? x_sxa       :
      p == "x-sxb"       ? x_sxb       :
      p == "x-sxc"       ? x_sxc       : x_unknown;
}

const char *Utility::getAppService( AppServiceEnum s )
{
   return
      s == x_3gpp_pgw  ? "x-3gpp-pgw"  :
      s == x_3gpp_sgw  ? "x-3gpp-sgw"  :
      s == x_3gpp_ggsn ? "x-3gpp-ggsn" :
      s == x_3gpp_sgsn ? "x-3gpp-sgsn" :
      s == x_3gpp_mme  ? "x-3gpp-mme"  :
      s == x_3gpp_msc  ? "x-3gpp-msc"  :
      s == x_3gpp_upf  ? "x-3gpp-upf"  :
      s == x_3gpp_amf  ? "x-3gpp-amf"  : "x-3gpp-unknown";
}

const char *Utility::getAppProtocol( AppProtocolEnum p )
{
   return
      p == x_gn        ? "x-gn"        :
      p == x_gp        ? "x-gp"        :
      p == x_n2        ? "x-n2"        :
      p == x_nq        ? "x-nq"        :
      p == x_nqprime   ? "x-nqprime"   :
      p == x_s1_mme    ? "x-s1-mme"    :
      p == x_s1_u      ? "x-s1-u"      :
      p == x_s10       ? "x-s10"       :
      p == x_s11       ? "x-s11"       :
      p == x_s12       ? "x-s12"       :
      p == x_s16       ? "x-s16"       :
      p == x_s2a_gtp   ? "x-s2a-gtp"   :
      p == x_s2a_mipv4 ? "x-s2a-mipv4" :
      p == x_s2a_pmip  ? "x-s2a-pmip"  :
      p == x_s2b_gtp   ? "x-s2b-gtp"   :
      p == x_s2b_pmip  ? "x-s2b-pmip"  :
      p == x_s2c_dsmip ? "x-s2c-dsmip" :
      p == x_s3        ? "x-s3"        :
      p == x_s4        ? "x-s4"        :
      p == x_s5_gtp    ? "x-s5-gtp"    :
      p == x_s5_pmip   ? "x-s5-pmip"   :
      p == x_s6a       ? "x-s6a"       :
      p == x_s8_gtp    ? "x-s8-gtp"    :
      p == x_s8_pmip   ? "x-s8-pmip"   :
      p == x_sv        ? "x-sv"        :
      p == x_sxa       ? "x-sxa"       :
      p == x_sxb       ? "x-sxb"       :
      p == x_sxc       ? "x-sxc"       : "x-unknown";
}

AppProtocolEnum Utility::getAppProtocol( PGWAppProtocolEnum p )
{
   return
      p == pgw_x_gn        ? x_gn        :
      p == pgw_x_gp        ? x_gp        :
      p == pgw_x_s2a_gtp   ? x_s2a_gtp   :
      p == pgw_x_s2a_mipv4 ? x_s2a_mipv4 :
      p == pgw_x_s2a_pmip  ? x_s2a_pmip  :
      p == pgw_x_s2b_gtp   ? x_s2b_gtp   :
      p == pgw_x_s2b_pmip  ? x_s2b_pmip  :
      p == pgw_x_s2c_dsmip ? x_s2c_dsmip :
      p == pgw_x_s5_gtp    ? x_s5_gtp    :
      p == pgw_x_s5_pmip   ? x_s5_pmip   :
      p == pgw_x_s8_gtp    ? x_s8_gtp    :
      p == pgw_x_s8_pmip   ? x_s8_pmip   : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( SGWAppProtocolEnum p )
{
   return
      p == sgw_x_s1_u     ? x_s1_u     :
      p == sgw_x_s11      ? x_s11      :
      p == sgw_x_s12      ? x_s12      :
      p == sgw_x_s2a_pmip ? x_s2a_pmip :
      p == sgw_x_s2b_pmip ? x_s2b_pmip :
      p == sgw_x_s4       ? x_s4       :
      p == sgw_x_s5_gtp   ? x_s5_gtp   :
      p == sgw_x_s5_pmip  ? x_s5_pmip  :
      p == sgw_x_s8_gtp   ? x_s8_gtp   :
      p == sgw_x_s8_pmip  ? x_s8_pmip  : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( GGSNAppProtocolEnum p )
{
   return
      p == ggsn_x_gn ? x_gn :
      p == ggsn_x_gp ? x_gp : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( SGSNAppProtocolEnum p )
{
   return
      p == sgsn_x_gn      ? x_gn      :
      p == sgsn_x_gp      ? x_gp      :
      p == sgsn_x_nqprime ? x_nqprime :
      p == sgsn_x_s16     ? x_s16     :
      p == sgsn_x_s3      ? x_s3      :
      p == sgsn_x_s4      ? x_s4      :
      p == sgsn_x_sv      ? x_sv      : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( MMEAppProtocolEnum p )
{
   return
      p == mme_x_gn     ? x_gn     :
      p == mme_x_gp     ? x_gp     :
      p == mme_x_nq     ? x_nq     :
      p == mme_x_s10    ? x_s10    :
      p == mme_x_s11    ? x_s11    :
      p == mme_x_s1_mme ? x_s1_mme :
      p == mme_x_s3     ? x_s3     :
      p == mme_x_s6a    ? x_s6a    :
      p == mme_x_sv     ? x_sv     : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( MSCAppProtocolEnum p )
{
   return
      p == msc_x_sv ? x_sv : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( UPFAppProtocolEnum p )
{
   return
      p == upf_x_sxa ? x_sxa :
      p == upf_x_sxb ? x_sxb :
      p == upf_x_sxc ? x_sxc : x_unknown;
}

AppProtocolEnum Utility::getAppProtocol( AMFAppProtocolEnum p )
{
   return
      p == amf_x_n2 ? x_n2 : x_unknown;
}

EString Utility::diameter_fqdn( const char *mnc, const char *mcc )
{
   EString s;

   // 'diameter\.epc.mnc(\d{3})\.mcc(\d{3})\.3gppnetworks\.org$'
   s.append( "diameter.epc." )
    .APPEND_MNC( mnc )
    .APPEND_MCC( mcc )
    .APPEND_3GPPNETWORK; 

   return s;
}

EString Utility::diameter_fqdn( const unsigned char *plmnid )
{
   PARSE_PLMNID( plmnid );
   return diameter_fqdn( mnc, mcc );
}

EString Utility::getDiameterService( DiameterApplicationEnum app, DiameterProtocolEnum protocol )
{
   std::ostringstream buffer; 
   buffer << "aaa+ap" << getDiameterApplication( app ) << ":" << getDiameterProtocol( protocol );
   return buffer.str();
}

uint32_t Utility::getDiameterApplication( DiameterApplicationEnum app )
{
   return
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
      app == dia_app_nasreq           ? 1 :
      app == dia_app_mobile_ipv4      ? 2 :
      app == dia_app_base_accounting  ? 3 :
      app == dia_app_credit_control   ? 4 :
      app == dia_app_eap              ? 5 :
      app == dia_app_sip6             ? 6 :
      app == dia_app_mobile_ipv6_ike  ? 7 :
      app == dia_app_mobile_ipv6_auth ? 8 :
      app == dia_app_qos              ? 9 :
      app == dia_app_relay            ? 4294967295 :
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
      app == dia_app_3gpp_sta ? 16777250 :
      app == dia_app_3gpp_s6a ? 16777251 :
      app == dia_app_3gpp_swm ? 16777264 :
      app == dia_app_3gpp_s9  ? 16777267 :
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
      app == dia_app_wimax_wnaaada          ? 16777281 :
      app == dia_app_wimax_wnada            ? 16777282 :
      app == dia_app_wimax_wm4da            ? 16777283 :
      app == dia_app_wimax_wm6da            ? 16777284 :
      app == dia_app_wimax_wdda             ? 16777285 :
      app == dia_app_wimax_wlaada           ? 16777286 :
      app == dia_app_wimax_pcc_r3_p         ? 16777287 :
      app == dia_app_wimax_pcc_r3_ofc       ? 16777288 :
      app == dia_app_wimax_pcc_r3_ofc_prime ? 16777289 :
      app == dia_app_wimax_pcc_r3_oc        ? 16777290 : 0;
}

const char *Utility::getDiameterProtocol( DiameterProtocolEnum protocol )
{
   return
      protocol == dia_protocol_tcp ?     "diameter.tcp"     :
      protocol == dia_protocol_sctp ?    "diameter.sctp"    :
      protocol == dia_protocol_tls_tcp ? "diameter.tls.tcp" : "unknown";
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CanonicalNodeName::CanonicalNodeName()
   : m_topon( False )
{
}

CanonicalNodeName::CanonicalNodeName( const std::string &n )
   : m_topon( False )
{
   setName( n );
}

Void CanonicalNodeName::setName( const std::string &n )
{
   // reset the object
   clear();
   m_topon = False;
   m_name = "";
   
   // parse the name into labels
   std::string arg;
   std::istringstream ss( n );

   // get the first label
   std::getline( ss, arg, '.' );

   // check for "topon"
   m_topon = arg == "topon";

   // check for not "topoff"
   if ( !m_topon && arg != "topoff" )
   {
      // set the name to what was passed in and stop processing
      m_name = n;
      return;
   }

   // skip the interface
   std::getline( ss, arg, '.' );

   // save the canonical name
   if ( !std::getline( ss, arg ) )
      return;  // no need to continue if there is no more data
   m_name = arg;

   // only get the labels of topon is specified
   if ( m_topon )
   {
      // set ss to the conanical name to split
      ss.str( m_name );
      ss.clear();

      // add the labels to the list
      while ( std::getline( ss, arg, '.' ) )
         push_back( arg );

      // reverse the list for use in topological matching
      reverse();
   }
}

int CanonicalNodeName::topologicalCompare( const CanonicalNodeName &right )
{
   int matchingLabels = 0;
   CanonicalNodeName::const_iterator itl = begin();
   CanonicalNodeName::const_iterator itr = right.begin();
   
   while ( itl != end() && itr != right.end() )
   {
      if ( *itl != *itr )
         break;
      ++matchingLabels;
      ++itl;
      ++itr;
   }

   return matchingLabels;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

NodeSelectorResultList &NodeSelector::process()
{
   std::list<AppProtocolEnum> supportedProtocols;

   // perform dns query
   Bool cacheHit = False;
   m_query = DNS::Cache::getInstance(m_nsid).query( ns_t_naptr, m_domain, cacheHit );

   // evaluate each answer to see if it matches the service/protocol requirements
   for (std::list<DNS::ResourceRecord*>::const_iterator rrit = m_query->getAnswers().begin();
        rrit != m_query->getAnswers().end();
        ++rrit )
   {
      DNS::RRecordNAPTR* naptr = (DNS::RRecordNAPTR*)*rrit;

      // parse the service field
      AppService service;

      service.parse( naptr->getService() );

      // check for service match
      if ( m_desiredService == x_3gpp_any || service.getService() == m_desiredService )
      {
         NodeSelectorResult *nsr = new NodeSelectorResult();

         nsr->setHostname( naptr->getReplacement() );
         nsr->setOrder( naptr->getOrder() );
         nsr->setPreference( naptr->getPreference() );

         // identify all of the desired protocols supported by the service 
         for (AppProtocolList::const_iterator dpit = m_desiredProtocols.begin();
              dpit != m_desiredProtocols.end();
              ++dpit)
         {
            // is the protocol supported by the naptr service
            AppProtocol *sp = service.findProtocol( (*dpit)->getProtocol() );
            if ( sp )
            {
               AppProtocol *nsrap = new AppProtocol();

               nsrap->setProtocol( sp->getProtocol() );

               // the protocol has to support at least one of the desired usage types
               if ( !sp->getUsageTypes().empty() )
               {
                  // the naptr app protocol only supports specific usage types
                  // need to check to see if one of the desired usage types matches
                  for ( UsageTypeList::const_iterator utit = m_desiredUsageTypes.begin();
                        utit != m_desiredUsageTypes.end();
                        ++utit )
                  {
                     if ( sp->findUsageType( *utit ) )
                        nsrap->addUsageType( *utit );
                  }

                  if ( !nsrap->getUsageTypes().empty() )
                  {
                     // at least 1 usage type matched, so consider the protocol a match
                     //nsr->addSupportedProtocol( nsrap );
                  }
                  else
                  {
                     // the protocol was not a match
                     delete nsrap;
                     continue;
                  }
               }

               // the protocol has to support all of the requested network capabilities
               {
                  Bool addit = True;
                  for ( NetworkCapabilityList::iterator ncit = m_desiredNetworkCapabilities.begin();
                        ncit != m_desiredNetworkCapabilities.end();
                        ++ncit )
                  {
                     if ( sp->findNetworkCapability( *ncit ) )
                     {
                        nsrap->addNetworkCapability( *ncit );
                     }
                     else
                     {
                        // the protocol is not a match since this network capability is not supported
                        addit = False;
                        break;
                     }
                  }

                  if (!addit)
                  {
                     delete nsrap;
                     continue;
                  }
               }

               // the naptr app protocol supportes at least 1 usage type and all of the requested network capabiities
               nsr->addSupportedProtocol( nsrap );
            }
         }

         if ( !nsr->getSupportedProtocols().empty() )
         {
            // iterate through the dns query additonal records adding the ip addresses for the host to the result
            for ( DNS::ResourceRecordList::const_iterator it = m_query->getAdditional().begin();
                  it != m_query->getAdditional().end();
                  ++it )
            {
               if ( nsr->getHostname() == (*it)->getName() )
               {
                  switch ( (*it)->getType() )
                  {
                     case ns_t_a:
                     {
                        nsr->addIPv4Host( ((DNS::RRecordA*)*it)->getAddressString() );
                        break;
                     }
                     case ns_t_aaaa:
                     {
                        nsr->addIPv6Host( ((DNS::RRecordAAAA*)*it)->getAddressString() );
                        break;
                     }
                  }
               }
            }

            // shuffle the ip addresses
            nsr->getIPv4Hosts().shuffle();
            nsr->getIPv6Hosts().shuffle();

            // add the nsr pointer to the list since at least 1 protocol matched
            m_results.push_back( nsr );
         }
         else
         {
            // delete the nsr pointer since no protocols matched
            delete nsr;
         }
      }
   }

   // sort the naptr list
   m_results.sort( NodeSelectorResultList::sort_compare );
      
   return m_results;
}

NodeSelector::NodeSelector()
{
   m_nsid = DNS::NS_DEFAULT;
   m_query = NULL;
}

NodeSelector::~NodeSelector()
{
   while ( !m_desiredProtocols.empty() )
   {
      AppProtocol *ap = *m_desiredProtocols.begin();
      m_desiredProtocols.pop_front();
      delete ap;
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Bool NodeSelectorResultList::sort_compare( NodeSelectorResult*& first, NodeSelectorResult*& second )
{
   if ( first->getOrder() < second->getOrder() )
      return True;

   if ( first->getOrder() > second->getOrder() )
      return False;

   return first->getPreference() < second->getPreference();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void AppService::parse( const std::string &rs )
{
   m_rawService = rs;
   
   std::string arg;
   std::istringstream ss( m_rawService );

   if ( !std::getline(ss, arg, ':') )
      return;

   m_service = Utility::getAppService( arg );
   
   while ( getline( ss, arg, ':' ) )
   {
      AppProtocol *ap = new AppProtocol();
      ap->parse( arg );
      m_protocols.push_back( ap );
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Void AppProtocol::parse( const std::string &rp )
{
   m_rawProtocol = rp;

   std::string arg;
   std::istringstream ss( m_rawProtocol );

   if ( !std::getline( ss, arg, '+' ) )
      return;

   m_protocol = Utility::getAppProtocol( arg );

   do
   {
      std::istringstream ss2( arg );

      if ( std::getline( ss2, arg, '-' ) )
      {
         if ( arg == "ue" )
         {
            while ( std::getline( ss2, arg, '.' ) )
               m_usageTypes.push_back( atoi( arg.c_str() ) );
         }
         else if ( arg == "nc" )
         {
            while ( std::getline( ss2, arg, '.' ) )
               m_networkCapabilities.push_back( arg );
         }
      }
   } while ( std::getline( ss, arg, '+') );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ColocatedCandidate::ColocatedCandidate( NodeSelectorResult &candidate1, NodeSelectorResult &candidate2 )
   : m_candidate1( candidate1 ),
     m_candidate2( candidate2 )
{
   m_cnn1.setName( m_candidate1.getHostname() );
   m_cnn2.setName( m_candidate2.getHostname() );

   m_pairtype =
      m_cnn1.getName() == m_cnn2.getName() ? ptColocated :
      m_cnn1.getTopon() && m_cnn2.getTopon() ? ptTopologicalDistance : ptDNSPriority;

   m_topologicalMatches =
      m_pairtype == ptTopologicalDistance ?  m_cnn1.topologicalCompare( m_cnn2 ) : 0;
}

ColocatedCandidateList::ColocatedCandidateList( NodeSelectorResultList &nodelist1, NodeSelectorResultList &nodelist2 )
   : m_nodelist1( nodelist1 ),
     m_nodelist2( nodelist2 )
{
   // create all of the colocated candidate pairs (which also classifies them)
   for ( NodeSelectorResultList::const_iterator it1 = m_nodelist1.begin();
         it1 != m_nodelist1.end();
         ++it1 )
   {
      for( NodeSelectorResultList::const_iterator it2 = m_nodelist2.begin();
           it2 != m_nodelist2.end();
           it2++ )
      {
         ColocatedCandidate *cc = new ColocatedCandidate( *(*it1), *(*it2) );
         push_back( cc );
      }
   }

   sort( ColocatedCandidateList::sort_compare );
}

ColocatedCandidateList::~ColocatedCandidateList()
{
   while ( !empty() )
   {
      ColocatedCandidate *cc = *begin();
      erase( begin() );
      delete cc;
   }
}

Bool ColocatedCandidateList::sort_compare( ColocatedCandidate*& first, ColocatedCandidate*& second )
{
   // sort by pairtype ascending
   if ( (int)first->getPairType() < (int)second->getPairType() )
      return True;
   if ( (int)first->getPairType() > (int)second->getPairType() )
      return False;

   // sort by the candidate1 order
   if ( first->getCandidate1().getOrder() < second->getCandidate1().getOrder() )
      return True;
   if ( first->getCandidate1().getOrder() > second->getCandidate1().getOrder() )
      return False;

   // sort by the candidate1 preference
   return first->getCandidate1().getPreference() < second->getCandidate1().getPreference();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DiameterSelector::DiameterSelector()
   : m_application( dia_app_unknown ),
     m_protocol( dia_protocol_unknown )
{
}

DiameterNaptrList &DiameterSelector::process()
{
   // validate m_applciation
   if ( m_application == dia_app_unknown )
      return m_results;

   // validate m_protocol
   if ( m_protocol == dia_protocol_unknown )
      return m_results;

   // validate realm
   if ( m_realm.empty() )
      return m_results;

   // construct the service string
   EString service( Utility::getDiameterService( m_application, m_protocol ) );

   // perform dns query
   Bool cacheHit = False;
   m_query = DNS::Cache::getInstance().query( ns_t_naptr, m_realm, cacheHit );

   // evaluate each answer to see if it matches the service/protocol requirements
   for ( std::list<DNS::ResourceRecord*>::const_iterator rrit = m_query->getAnswers().begin();
         rrit != m_query->getAnswers().end();
         ++rrit )
   {
      DNS::RRecordNAPTR* naptr = (DNS::RRecordNAPTR*)*rrit;
      
      // does the naptr service field match the app/protocol that we are looking for
      if ( naptr->getService() == service )
      {
         DiameterNaptr *n = NULL;

         if ( naptr->getFlags() == "a" )
            n = new DiameterNaptrA();
         else if ( naptr->getFlags() == "s" )
            n = new DiameterNaptrS();

         // check for valid "flags" value
         if ( !n )
            continue;

         n->setOrder( naptr->getOrder() );
         n->setPreference( naptr->getPreference() );
         n->setService( naptr->getService() );
         n->setReplacement( naptr->getReplacement() );

         if ( n->getType() == dnt_hostname )
         {
            DiameterNaptrA *a = (DiameterNaptrA*)n;

            // set the host name
            a->getHost().setName( n->getReplacement() );

            // add all of the A/AAAA records for the host
            for ( DNS::ResourceRecordList::const_iterator rr = m_query->getAdditional().begin();
                  rr != m_query->getAdditional().end();
                  ++rr)
            {
               if ( (*rr)->getName() == a->getHost().getName() )
               {
                  switch ( (*rr)->getType() )
                  {
                     case ns_t_a:
                     {
                        a->getHost().addIPv4Address( ((DNS::RRecordA*)*rr)->getAddressString() );
                        break;
                     }
                     case ns_t_aaaa:
                     {
                        a->getHost().addIPv6Address( ((DNS::RRecordAAAA*)*rr)->getAddressString() );
                        break;
                     }
                  }
               }
            }

            // randomize the ip addresses
            a->getHost().getIPv4Addresses().shuffle();
            a->getHost().getIPv6Addresses().shuffle();
         }
         else if ( n->getType() == dnt_service )
         {
            DiameterNaptrS *s = (DiameterNaptrS*)n;

            // add all of the matching SRV records
            for ( DNS::ResourceRecordList::const_iterator rr = m_query->getAdditional().begin();
                  rr != m_query->getAdditional().end();
                  ++rr)
            {
               if ( (*rr)->getType() == ns_t_srv &&  (*rr)->getName() == s->getReplacement() )
               {
                  DNS::RRecordSRV *rrs = (DNS::RRecordSRV*)*rr;

                  DiameterSrv *ds = new DiameterSrv();

                  // set the SRV properties
                  ds->setPriority( rrs->getPriority() );
                  ds->setWeight( rrs->getWeight() );
                  ds->setPort( rrs->getPort() );

                  // loop through the additional records adding the hostname entries that match the SRV hostname
                  for ( DNS::ResourceRecordList::const_iterator rr2 = m_query->getAdditional().begin();
                        rr2 != m_query->getAdditional().end();
                        ++rr2 )
                  {
                     if ( (*rr2)->getName() == rrs->getTarget() )
                     {
                        ds->getHost().setName( (*rr2)->getName() );
                        switch ( (*rr2)->getType() )
                        {
                           case ns_t_a:
                           {
                              ds->getHost().addIPv4Address( ((DNS::RRecordA*)*rr2)->getAddressString() );
                              break;
                           }
                           case ns_t_aaaa:
                           {
                              ds->getHost().addIPv6Address( ((DNS::RRecordAAAA*)*rr2)->getAddressString() );
                              break;
                           }
                        }
                     }
                  }

                  // randomize the ip addresses
                  ds->getHost().getIPv4Addresses().shuffle();
                  ds->getHost().getIPv6Addresses().shuffle();

                  // add to the SRV collection
                  s->getSrvs().push_back( ds );
               }
            }

            // sort the srv records
            s->getSrvs().sort_vector();
         }
         else
         {
            delete n;
            continue;
         }

         // add the record to the results
         m_results.push_back( n );
      }
   }
   

   return m_results;
}

Void DiameterSrvVector::sort_vector()
{
   // sort the SRV records according to RFC 2782

   // sort the list ascending on priority and weight
   std::sort( this->begin(), this->end(), DiameterSrvVector::sort_compare );

   int i = 0;
   int priority = -1;
   int first = -1;
   int last = -1;

   srand( time(NULL) );              /* initialize random seed: */

   for ( int i = 0; i <= size(); i++ )
   {
      if ( ( i == size() || at(i)->getPriority() != priority ) && priority != -1)
      {
         if ( priority != -1 )
         {
            // the sorted vector contains indices in the specified range in sorted order
            std::vector<int> sorted;

            // execute selection algo for each item in range
            for ( int j = 0; j < last - first + 1; j++ )
            {
               int runningTotal = 0;
               std::vector<int> unsorted;
               std::vector<int> runningTotals;

               // generate the running totals
               for ( int k = first; k <= last; k++ )
               {
                  // skip if entry is already in sorted list
                  if ( std::find( sorted.begin(), sorted.end(), k ) == sorted.end() )
                  {
                     runningTotal += at(k)->getWeight();
                     unsorted.push_back( k );
                     runningTotals.push_back( runningTotal );
                  }
               }

               // generate value between 0 and runningTotal
               int val = rand() & runningTotal;

               // identify the entry and place it in the sorted list
               for ( int l = 0; l < runningTotals.size(); l++ )
               {
                  if ( runningTotals[l] >= val )
                  {
                     sorted.push_back( unsorted[l] );
                     break;
                  }
               }
            }

            // save the pointers in sorted order
            DiameterSrvVector newsorted;
            for ( int j = 0; j < sorted.size(); j++ )
               newsorted.push_back( at( sorted[j] ) );

            // update the original array with the sorted values
            for ( int j = first; j <= last; j++ )
               (*this)[ j ] = newsorted[ j - first ];
         }

         if ( i < size() )
         {
            // init for the next priority
            priority = at(i)->getPriority();
            first = i;
            last = i;
         }
      }
      else
      {
         last = i;
      }
   }
}

Bool DiameterSrvVector::sort_compare( DiameterSrv* first, DiameterSrv* second )
{
   return
      first->getPriority() < second->getPriority() ? True :
      first->getPriority() > second->getPriority() ? False : first->getWeight() < second->getWeight();
}
