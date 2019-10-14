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
#include "etime.h"
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
      MessageStats(EStatistics::MessageId id, cpStr name);
      MessageStats(EStatistics::MessageId id, const EString &name);
      MessageStats(const MessageStats &m);

      Void reset();

      EStatistics::MessageId getId() { return m_id; }
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

      EStatistics::MessageId m_id;
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

   typedef std::unordered_map<EStatistics::MessageId,EStatistics::MessageStats> MessageStatsMap;

   class Peer
   {
   public:
      Peer(cpStr name, const EStatistics::MessageStatsMap &tmplt);
      Peer(const EString &name, const EStatistics::MessageStatsMap &tmplt);
      Peer(const Peer &p);

      EStatistics::MessageStats &getMessageStats(UInt msgid);

      EString &getName() { return m_name; }
      ETime &getLastActivity() { return m_lastactivity; }
      ETime &setLastActivity() { return m_lastactivity = ETime::Now(); }
      EStatistics::MessageStatsMap &getMessageStats() { return m_msgstats; }
      Void reset();

      #define INCREMENT_MESSAGE_STAT(__id,__func)  \
      {                                            \
         auto srch = m_msgstats.find(__id);        \
         if (srch == m_msgstats.end() )            \
            return 0;                              \
         setLastActivity();                        \
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
      ETime m_lastactivity;
      ERWLock m_lock;
      EStatistics::MessageStatsMap m_msgstats;
   };

   typedef std::unordered_map<std::string,EStatistics::Peer> PeerMap;

   class Interface
   {
   public:
      Interface(EStatistics::InterfaceId id, EStatistics::ProtocolType protocol, cpStr name);
      Interface(EStatistics::InterfaceId id, EStatistics::ProtocolType protocol, const EString &name);
      Interface(const Interface &i);

      EStatistics::InterfaceId getId() { return m_id; }
      EString &getName() { return m_name; }
      ProtocolType getProtocol() { return m_protocol; }
      PeerMap &getPeers() { return m_peers; }

      Peer &getPeer(const EString &peer, Bool addFlag = True);
      Peer &addPeer(const EString &peer);
      Void removePeer(const EString &peer);
      Void reset();

      MessageStats &addMessageStatsTemplate(EStatistics::MessageId msgid, const EString &name);
      MessageStatsMap &getMessageStatsTemplate() { return m_msgstats_template; }

      #define INCREMENT_MESSAGE_STAT(__peer,__id,__func) \
      {                                                  \
         EStatistics::Peer &__p( getPeer(__peer) );      \
         return __p.__func(__id);                        \
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

      Peer &_addPeer(const EString &peer);

      EStatistics::InterfaceId m_id;
      ProtocolType m_protocol;
      EString m_name;
      ERWLock m_lock;
      PeerMap m_peers;
      MessageStatsMap m_msgstats_template;
   };

   typedef std::unordered_map<EStatistics::InterfaceId,EStatistics::Interface> InterfaceMap;

   static Interface &getInterface(EStatistics::InterfaceId id)
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

   static Interface &addInterface(EStatistics::InterfaceId id, ProtocolType protocol, const EString &intfc)
   {
      EWRLock l(m_lock);
      auto it = m_interfaces.emplace(id, Interface(id,protocol,intfc));
      return it.first->second;
   }

   static Void removeInterface(EStatistics::InterfaceId id)
   {
      EWRLock l(m_lock);
      auto srch = m_interfaces.find(id);
      if (srch != m_interfaces.end())
         m_interfaces.erase( srch );
   }

   static InterfaceMap &getInterfaces() { return m_interfaces; }

   static Void init(ELogger &logger);
   static Void reset();

private:
   static DiameterHook m_hook_error;
   static DiameterHook m_hook_success;

   static ERWLock m_lock;
   static EStatistics::InterfaceMap m_interfaces;
};

#endif // #ifndef __ESTATS_H