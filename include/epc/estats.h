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

#ifndef __ESTATS_H
#define __ESTATS_H

#include <atomic>
#include <unordered_map>

#include "ebase.h"
#include "eerror.h"
#include "efd.h"
#include "elogger.h"
#include "estring.h"
#include "esynch.h"

//
// According to IANA, the current diameter application ID range is from 0 to 16777361.
// Diameter application ID's start at 0 and appear to be "reserved" through 16777215 (0x00FFFFFF).
// 3GPP diameter applications ID's appear to have the low order bit of the high order word set to 1 (16777216 decimal or 0x1000000 hex).
// 
#define GTPV2C_BASE  (0xff000000)
#define GTPV1U_BASE  (0xfe000000)
#define PFCP_BASE    (0xfd000000)

const UInt INTERFACE_S11C    = (GTPV2C_BASE + 1);
const UInt INTERFACE_S5S8C   = (GTPV2C_BASE + 2);
const UInt INTERFACE_S4      = (GTPV2C_BASE + 3);
const UInt INTERFACE_S2b     = (GTPV2C_BASE + 4);

const UInt INTERFACE_S1U     = (GTPV1U_BASE + 1);
const UInt INTERFACE_S11U    = (GTPV1U_BASE + 2);
const UInt INTERFACE_S5S8U   = (GTPV1U_BASE + 3);
const UInt INTERFACE_SWu     = (GTPV1U_BASE + 4);

const UInt INTERFACE_Sxa     = (PFCP_BASE + 1);
const UInt INTERFACE_Sxb     = (PFCP_BASE + 2);
const UInt INTERFACE_Sxc     = (PFCP_BASE + 3);
const UInt INTERFACE_SxaSxb  = (PFCP_BASE + 4);

const UInt DIAMETER_ANSWER_BIT = 0x80000000;

class EStatistics
{
public:
   enum class ProtocolType
   {
      diameter,
      gtpv2c,
      gtpv1u,
      pfcp
   };

   typedef UInt InterfaceId;
   typedef UInt MessageId;

   class DiameterHook : public FDHook
   {
   public:
      DiameterHook()
         : m_logger( NULL )
      {
      }

      Void process(enum fd_hook_type type, struct msg * msg, struct peer_hdr * peer,
         void * other, struct fd_hook_permsgdata *pmd);
      
      Void setLogger(ELogger &logger) { m_logger = &logger; }

   private:
      Bool getResult(struct msg *m);

      ELogger *m_logger;
   };

   class MessageStats
   {
   public:
      MessageStats(MessageId id, cpStr name)
         : m_id( id ),
           m_name( name )
      {
         reset();
      }

      MessageStats(MessageId id, const EString &name)
         : m_id( id ),
           m_name( name )
      {
         reset();
      }

      MessageStats(const MessageStats &m)
         : m_id( m.m_id ),
           m_name( m_name ),
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

      Void reset()
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

      MessageId getId() { return m_id; }
      const EString &getName() { return m_name; }

      UInt getRequestSentErrors() { return m_rqst_sent_err; }
      UInt getRequestReceivedErrors() { return m_rqst_rcvd_err; }
      UInt getRequestSentOk() { return m_rqst_sent_ok; }
      UInt getRequestReceivedOk() { return m_rqst_rcvd_ok; }

      UInt getResponseSentErrors() { return m_resp_sent_err; }
      UInt getResponseReceivedErrors() { return m_resp_rcvd_err; }
      UInt getResponseSentOkAccepted() { return m_resp_sent_accept; }
      UInt getResponseSentOkRejected() { return m_resp_sent_reject; }
      UInt getResponseReceivedOkAccepted() { return m_resp_rcvd_accept; }
      UInt getResponseReceivedOkRejected() { return m_resp_rcvd_reject; }

      UInt incRequestSentErrors() { return ++m_rqst_sent_err; }
      UInt incRequestReceivedErrors() { return ++m_rqst_rcvd_err; }
      UInt incRequestSentOk() { return ++m_rqst_sent_ok; }
      UInt incRequestReceivedOk() { return ++m_rqst_rcvd_ok; }

      UInt incResponseSentErrors() { return ++m_resp_sent_err; }
      UInt incResponseReceivedErrors() { return ++m_resp_rcvd_err; }
      UInt incResponseSentOkAccepted() { return ++m_resp_sent_accept; }
      UInt incResponseSentOkRejected() { return ++m_resp_sent_reject; }
      UInt incResponseReceivedOkAccepted() { return ++m_resp_rcvd_accept; }
      UInt incResponseReceivedOkRejected() { return ++m_resp_rcvd_reject; }

   private:
      MessageStats();

      MessageId m_id;
      EString m_name;

      std::atomic<UInt> m_rqst_sent_err;
      std::atomic<UInt> m_rqst_rcvd_err;
      std::atomic<UInt> m_rqst_sent_ok;
      std::atomic<UInt> m_rqst_rcvd_ok;

      std::atomic<UInt> m_resp_sent_err;
      std::atomic<UInt> m_resp_rcvd_err;
      std::atomic<UInt> m_resp_sent_accept;
      std::atomic<UInt> m_resp_sent_reject;
      std::atomic<UInt> m_resp_rcvd_accept;
      std::atomic<UInt> m_resp_rcvd_reject;
   };

   typedef std::unordered_map<UInt,MessageStats> MessageStatsMap;

   class Peer
   {
   public:
      Peer(cpStr name, std::unordered_map<UInt,MessageStats> &tmplt)
         : m_name( name ),
           m_msgstats( tmplt )
      {         
      }

      Peer(const EString &name, std::unordered_map<UInt,MessageStats> &tmplt)
         : m_name( name ),
           m_msgstats( tmplt )
      {         
      }

      Peer(const Peer &p)
         : m_name( p.m_name ),
           m_msgstats( p.m_msgstats )
      {
      }

      std::unordered_map<UInt,MessageStats> getMessageStats() { return m_msgstats; }

      MessageStats &getMessageStats(UInt msgid)
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

      // MessageStats &addMessageStats(UInt msgid, const EString &name)
      // {
      //    EWRLock l(m_lock);
      //    auto p = m_msgstats.emplace(msgid, name);
      //    return p.first->second;
      // }

      // Void removeMessageStats(UInt msgid)
      // {
      //    EWRLock l(m_lock);
      //    auto srch = m_msgstats.find(msgid);
      //    if (srch != m_msgstats.end())
      //       m_msgstats.erase( srch );
      // }

      #define INCREMENT_MESSAGE_STAT(__id,__func)  \
      {                                            \
         auto srch = m_msgstats.find(__id);        \
         if (srch == m_msgstats.end() )            \
            return 0;                              \
         return srch->second.__func();             \
      }

      UInt incRequestSentErrors(UInt msgid)           { INCREMENT_MESSAGE_STAT(msgid, incRequestSentErrors); }
      UInt incRequestReceivedErrors(UInt msgid)       { INCREMENT_MESSAGE_STAT(msgid, incRequestReceivedErrors); }
      UInt incRequestSentOk(UInt msgid)               { INCREMENT_MESSAGE_STAT(msgid, incRequestSentOk); }
      UInt incRequestReceivedOk(UInt msgid)           { INCREMENT_MESSAGE_STAT(msgid, incRequestReceivedOk); }
      UInt incResponseSentErrors(UInt msgid)          { INCREMENT_MESSAGE_STAT(msgid, incResponseSentErrors); }
      UInt incResponseReceivedErrors(UInt msgid)      { INCREMENT_MESSAGE_STAT(msgid, incResponseReceivedErrors); }
      UInt incResponseSentOkAccepted(UInt msgid)      { INCREMENT_MESSAGE_STAT(msgid, incResponseSentOkAccepted); }
      UInt incResponseSentOkRejected(UInt msgid)      { INCREMENT_MESSAGE_STAT(msgid, incResponseSentOkRejected); }
      UInt incResponseReceivedOkAccepted(UInt msgid)  { INCREMENT_MESSAGE_STAT(msgid, incResponseReceivedOkAccepted); }
      UInt incResponseReceivedOkRejected(UInt msgid)  { INCREMENT_MESSAGE_STAT(msgid, incResponseReceivedOkRejected); }

      #undef INCREMENT_MESSAGE_STAT

   private:
      EString m_name;
      ERWLock m_lock;
      std::unordered_map<MessageId,MessageStats> m_msgstats;
   };

   typedef std::unordered_map<std::string,Peer> PeerMap;

   class Interface
   {
   public:
      Interface(InterfaceId id, ProtocolType protocol, cpStr name)
         : m_id( id ),
           m_protocol( protocol ),
           m_name( name )
         {
         }
      Interface(InterfaceId id, ProtocolType protocol, const EString &name)
         : m_id( id ),
           m_protocol( protocol ),
           m_name( name )
         {
         }
      
      Interface(const Interface &i)
         : m_id( i.m_id ),
           m_protocol( i.m_protocol ),
           m_name( i.m_name ),
           m_peers( i.m_peers )
      {
      }

      InterfaceId getId() { return m_id; }
      PeerMap &getPeers() { return m_peers; }

      Peer &getPeer(const EString &peer, Bool addFlag = True)
      {
         ERDLock l(m_lock);
         auto srch = m_peers.find(peer);
         if (srch == m_peers.end())
         {
            if (addFlag)
            {
               return addPeer(peer);
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

      Peer &addPeer(const EString &peer)
      {
         EWRLock l(m_lock);
         auto p = m_peers.emplace(peer, Peer(peer,m_msgstats_template));
         return p.first->second;
      }

      Void removePeer(const EString &peer)
      {
         EWRLock l(m_lock);
         auto srch = m_peers.find(peer);
         if (srch != m_peers.end())
            m_peers.erase( srch );
      }

      MessageStats &addMessageStatsTemplate(UInt msgid, const EString &name)
      {
         auto p = m_msgstats_template.emplace(msgid, MessageStats(msgid, name));
         return p.first->second;
      }

      MessageStatsMap &getMessageStatsTemplate() { return m_msgstats_template; }

      #define INCREMENT_MESSAGE_STAT(__peer,__id,__func) \
      {                                                  \
         Peer &__p( getPeer(__peer) );                     \
         return __p.__func(__id);                          \
      }

      UInt incRequestSentErrors(cpStr peer, UInt msgid)           { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestSentErrors); }
      UInt incRequestReceivedErrors(cpStr peer, UInt msgid)       { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestReceivedErrors); }
      UInt incRequestSentOk(cpStr peer, UInt msgid)               { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestSentOk); }
      UInt incRequestReceivedOk(cpStr peer, UInt msgid)           { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestReceivedOk); }
      UInt incResponseSentErrors(cpStr peer, UInt msgid)          { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseSentErrors); }
      UInt incResponseReceivedErrors(cpStr peer, UInt msgid)      { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseReceivedErrors); }
      UInt incResponseSentOkAccepted(cpStr peer, UInt msgid)      { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseSentOkAccepted); }
      UInt incResponseSentOkRejected(cpStr peer, UInt msgid)      { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseSentOkRejected); }
      UInt incResponseReceivedOkAccepted(cpStr peer, UInt msgid)  { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseReceivedOkAccepted); }
      UInt incResponseReceivedOkRejected(cpStr peer, UInt msgid)  { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseReceivedOkRejected); }

      UInt incRequestSentErrors(const std::string &peer, UInt msgid)          { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestSentErrors); }
      UInt incRequestReceivedErrors(const std::string &peer, UInt msgid)      { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestReceivedErrors); }
      UInt incRequestSentOk(const std::string &peer, UInt msgid)              { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestSentOk); }
      UInt incRequestReceivedOk(const std::string &peer, UInt msgid)          { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestReceivedOk); }
      UInt incResponseSentErrors(const std::string &peer, UInt msgid)         { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseSentErrors); }
      UInt incResponseReceivedErrors(const std::string &peer, UInt msgid)     { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseReceivedErrors); }
      UInt incResponseSentOkAccepted(const std::string &peer, UInt msgid)     { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseSentOkAccepted); }
      UInt incResponseSentOkRejected(const std::string &peer, Int msgid)      { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseSentOkRejected); }
      UInt incResponseReceivedOkAccepted(const std::string &peer, UInt msgid) { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseReceivedOkAccepted); }
      UInt incResponseReceivedOkRejected(const std::string &peer, UInt msgid) { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseReceivedOkRejected); }

      #undef INCREMENT_MESSAGE_STAT

   private:
      Interface();

      InterfaceId m_id;
      ProtocolType m_protocol;
      EString m_name;
      ERWLock m_lock;
      PeerMap m_peers;
      MessageStatsMap m_msgstats_template;
   };

   typedef std::unordered_map<InterfaceId,Interface> InterfaceMap;

   static Interface &getInterface(InterfaceId id)
   {
      ERDLock l(m_lock);
      auto srch = m_interfaces.find(id);
      if (srch == m_interfaces.end())
      {
         EString s;
         s.format("Unknown interface ID [%u]", id);
         throw EError(EError::Warning, s);
      }
      return srch->second;
   }

   static Interface &addInterface(InterfaceId id, ProtocolType protocol, const EString &intfc)
   {
      EWRLock l(m_lock);
      auto it = m_interfaces.emplace(id, Interface(id,protocol,intfc));
      return it.first->second;
   }

   static Void removeInterface(InterfaceId id)
   {
      EWRLock l(m_lock);
      auto srch = m_interfaces.find(id);
      if (srch != m_interfaces.end())
         m_interfaces.erase( srch );
   }

   static Void init(ELogger &logger);

private:
   static DiameterHook m_hook_error;
   static DiameterHook m_hook_success;

   static ERWLock m_lock;
   static InterfaceMap m_interfaces;
};

#endif // #ifndef __ESTATS_H