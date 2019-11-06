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

#include "estats.h"

EStatistics::DiameterHook EStatistics::m_hook_error;
EStatistics::DiameterHook EStatistics::m_hook_success;
ERWLock EStatistics::m_lock;
EStatistics::InterfaceMap EStatistics::m_interfaces;

Void EStatistics::init(ELogger &logger)
{
   UInt mask_error = HOOK_MASK( HOOK_MESSAGE_PARSING_ERROR, HOOK_MESSAGE_ROUTING_ERROR /*, HOOK_MESSAGE_DROPPED*/  );
   UInt mask_success = HOOK_MASK( HOOK_MESSAGE_RECEIVED, HOOK_MESSAGE_SENDING );

   m_hook_error.registerHook( mask_error );
   m_hook_success.registerHook( mask_success );

   m_hook_error.setLogger( logger );
   m_hook_success.setLogger( logger );
}

Void EStatistics::uninit()
{
   m_hook_error.unregisterHook();
   m_hook_success.unregisterHook();
}

Void EStatistics::reset()
{
   for (auto &ifc : getInterfaces())
   {
      ifc.second.reset();
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Bool EStatistics::DiameterHook::getResult(struct msg *m)
{
   int ret;
   struct avp *a = NULL;

   if ((ret=fd_msg_browse_internal(m, MSG_BRW_FIRST_CHILD, (msg_or_avp**)&a, NULL)) == 0)
   {
      struct avp_hdr *ah;

      while (ret == 0 && a)
      {
         fd_msg_avp_hdr( a, &ah );

         if (ah->avp_vendor == 0 && ah->avp_code == 268 /* Result-Code */)
         {
            struct fd_pei ei;
            if (fd_msg_parse_dict(a, fd_g_config->cnf_dict, &ei) == 0)
               return ah->avp_value->i32 == 2001; /* DIAMETER_SUCCESS */
            return False;
         }

         ret = fd_msg_browse_internal(a, MSG_BRW_NEXT, (msg_or_avp**)&a, NULL);
      }
   }

   return false;
}

/// @cond DOXYGEN_EXCLUDE

Void EStatistics::DiameterHook::process(enum fd_hook_type type, struct msg * msg,
   struct peer_hdr * peer, Void * other, struct fd_hook_permsgdata *pmd)
{
   struct msg_hdr* hdr = NULL;

   if ( !msg || fd_msg_hdr(msg,&hdr) )
      return;

   Bool isError = HOOK_MASK(HOOK_MESSAGE_RECEIVED, HOOK_MESSAGE_SENDING) & type == 0;
   Bool isRequest = (hdr->msg_flags & CMD_FLAG_REQUEST) == CMD_FLAG_REQUEST;
   EStatistics::InterfaceId intfcid = hdr->msg_appl;
   EStatistics::MessageId msgid = isRequest ? hdr->msg_code : hdr->msg_code | DIAMETER_ANSWER_BIT;

   try
   {
      EStatistics::Interface &intfc( EStatistics::getInterface(intfcid) );
      
      if (isRequest)
      {
         switch (type)
         {
            case HOOK_MESSAGE_RECEIVED:      { intfc.incRequestReceivedOk( peer->info.pi_diamid, msgid );      break; }
            case HOOK_MESSAGE_SENDING:       { intfc.incRequestSentOk( peer->info.pi_diamid, msgid );          break; }
            case HOOK_MESSAGE_PARSING_ERROR: { intfc.incRequestSentErrors( peer->info.pi_diamid, msgid );      break; }
            case HOOK_MESSAGE_ROUTING_ERROR: { intfc.incRequestReceivedErrors( peer->info.pi_diamid, msgid );  break; }
         }
      }
      else
      {
         if (!isError)
         {
            Bool success = getResult(msg);
            if (success)
            {
               switch (type)
               {
                  case HOOK_MESSAGE_RECEIVED:   { intfc.incResponseReceivedOkAccepted( peer->info.pi_diamid, msgid | DIAMETER_ANSWER_BIT ); break; }
                  case HOOK_MESSAGE_SENDING:    { intfc.incResponseSentOkAccepted( peer->info.pi_diamid, msgid | DIAMETER_ANSWER_BIT ); break; }
               }
            }
            else
            {
               switch (type)
               {
                  case HOOK_MESSAGE_RECEIVED:   { intfc.incResponseReceivedOkRejected( peer->info.pi_diamid, msgid | DIAMETER_ANSWER_BIT ); break; }
                  case HOOK_MESSAGE_SENDING:    { intfc.incResponseSentOkRejected( peer->info.pi_diamid, msgid | DIAMETER_ANSWER_BIT ); break; }
               }
            }
         }
         else
         {
            switch (type)
            {
               case HOOK_MESSAGE_PARSING_ERROR: { intfc.incResponseSentErrors( peer->info.pi_diamid, msgid | DIAMETER_ANSWER_BIT );      break; }
               case HOOK_MESSAGE_ROUTING_ERROR: { intfc.incResponseReceivedErrors( peer->info.pi_diamid, msgid | DIAMETER_ANSWER_BIT );  break; }
            }
         }
         
      }      
   }
   catch(EError &e)
   {
      // std::cerr << e.what() << '\n';
      // return;
   }
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EStatistics::MessageStats::MessageStats(EStatistics::MessageId id, cpStr name)
   : m_id( id ),
     m_name( name )
{
   reset();
}

EStatistics::MessageStats::MessageStats(EStatistics::MessageId id, const EString &name)
   : m_id( id ),
     m_name( name )
{
   reset();
}

EStatistics::MessageStats::MessageStats(const MessageStats &m)
   : m_id( m.m_id ),
     m_name( m.m_name ),
     m_rqst_sent_err( m.m_rqst_sent_err.load() ),
     m_rqst_rcvd_err( m.m_rqst_rcvd_err.load() ),
     m_rqst_sent_ok( m.m_rqst_sent_ok.load() ),
     m_rqst_rcvd_ok( m.m_rqst_rcvd_ok.load() ),

     m_resp_sent_err( m.m_resp_sent_err.load() ),
     m_resp_rcvd_err( m.m_resp_rcvd_err.load() ),
     m_resp_sent_accept( m.m_resp_sent_accept.load() ),
     m_resp_sent_reject( m.m_resp_sent_reject.load() ),
     m_resp_rcvd_accept( m.m_resp_rcvd_accept.load() ),
     m_resp_rcvd_reject( m.m_resp_rcvd_reject.load() )
{
}

Void EStatistics::MessageStats::reset()
{
   m_rqst_sent_err = 0;
   m_rqst_rcvd_err = 0;
   m_rqst_sent_ok = 0;
   m_rqst_rcvd_ok = 0;

   m_resp_sent_err = 0;
   m_resp_rcvd_err = 0;
   m_resp_sent_accept = 0;
   m_resp_sent_reject = 0;
   m_resp_rcvd_accept = 0;
   m_resp_rcvd_reject = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EStatistics::Peer::Peer(cpStr name, const EStatistics::MessageStatsMap &tmplt)
   : m_name( name ),
     m_msgstats( tmplt )
{         
}

EStatistics::Peer::Peer(const EString &name, const EStatistics::MessageStatsMap &tmplt)
   : m_name( name ),
     m_msgstats( tmplt )
{         
}

EStatistics::Peer::Peer(const Peer &p)
   : m_name( p.m_name ),
     m_msgstats( p.m_msgstats )
{
}

EStatistics::MessageStats &EStatistics::Peer::getMessageStats(UInt msgid)
{
   ERDLock l(m_lock);
   auto srch = m_msgstats.find(msgid);
   if (srch == m_msgstats.end())
   {
      EString s;
      s.format("Unknown message ID [%u]", msgid);
      throw EError(EError::Warning, s);
   }
   return srch->second;
}

Void EStatistics::Peer::reset()
{
   EWRLock l(m_lock);
   for (auto &msgstats : getMessageStats())
      msgstats.second.reset();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

EStatistics::Interface::Interface(EStatistics::InterfaceId id, EStatistics::ProtocolType protocol, cpStr name)
   : m_id( id ),
     m_protocol( protocol ),
     m_name( name )
{
}

EStatistics::Interface::Interface(EStatistics::InterfaceId id, EStatistics::ProtocolType protocol, const EString &name)
   : m_id( id ),
     m_protocol( protocol ),
     m_name( name )
{
}

EStatistics::Interface::Interface(const EStatistics::Interface &i)
   : m_id( i.m_id ),
     m_protocol( i.m_protocol ),
     m_name( i.m_name ),
     m_peers( i.m_peers )
{
}

EStatistics::Peer &EStatistics::Interface::getPeer(const EString &peer, Bool addFlag)
{
   ERDLock l(m_lock);
   auto srch = m_peers.find(peer);
   if (srch == m_peers.end())
   {
      if (addFlag)
      {
         return _addPeer(peer);
      }
      else
      {
         EString s;
         s.format("Unknown peer [%s]", peer.c_str());
         throw EError(EError::Warning, s);
      }
   }
   return srch->second;
}

EStatistics::Peer &EStatistics::Interface::addPeer(const EString &peer)
{
   EWRLock l(m_lock);
   return _addPeer(peer);
}

Void EStatistics::Interface::removePeer(const EString &peer)
{
   EWRLock l(m_lock);
   auto srch = m_peers.find(peer);
   if (srch != m_peers.end())
      m_peers.erase( srch );
}

Void EStatistics::Interface::reset()
{
   EWRLock l(m_lock);
   for (auto &peer : getPeers())
      peer.second.reset();
}

EStatistics::MessageStats &EStatistics::Interface::addMessageStatsTemplate(EStatistics::MessageId msgid, const EString &name)
{
   auto p = m_msgstats_template.emplace(msgid, MessageStats(msgid, name));
   return p.first->second;
}

EStatistics::Peer &EStatistics::Interface::_addPeer(const EString &peer)
{
   Peer p( peer, getMessageStatsTemplate() );
   auto addedpeer = m_peers.emplace(peer, p);
   return addedpeer.first->second;
}
