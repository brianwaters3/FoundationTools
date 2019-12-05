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

/// @file
/// @brief Records the message statistics by interface, peer and message
///        type.

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
/// @cond DOXYGEN_EXCLUDE
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
/// @endcond

class EStatistics
{
public:
   enum class ProtocolType
   {
      diameter,
      gtpv2c,
      gtpv1u,
      pfcp,
      ikev2
   };

   typedef UInt InterfaceId;
   typedef UInt MessageId;

   /// @brief Hooks into the freeDiameter internals to increment
   ///        the message statistics.
   class DiameterHook : public FDHook
   {
   public:
   /// @cond DOXYGEN_EXCLUDE
      DiameterHook()
         : m_logger( NULL )
      {
      }

      Void process(enum fd_hook_type type, struct msg * msg, struct peer_hdr * peer,
         Void * other, struct fd_hook_permsgdata *pmd);
      
      Void setLogger(ELogger &logger) { m_logger = &logger; }
   /// @endcond

   private:
      Bool getResult(struct msg *m);

      ELogger *m_logger;
   };

   /// @brief Contains the statistics for an individual message.
   class MessageStats
   {
   public:
      MessageStats(EStatistics::MessageId id, cpStr name);
      MessageStats(EStatistics::MessageId id, const EString &name);
      MessageStats(const MessageStats &m);

      Void reset();

      EStatistics::MessageId getId() { return m_id; }
      const EString &getName() { return m_name; }

      /// @brief Retrieves the request send errors for this message.
      /// @return the request send errors for this message.
      UInt getRequestSentErrors() { return m_rqst_sent_err; }
      /// @brief Retrieves the request receive errors for this message.
      /// @return the request receive errors for this message.
      UInt getRequestReceivedErrors() { return m_rqst_rcvd_err; }
      /// @brief Retrieves the request send successes for this message.
      /// @return the request send successes for this message.
      UInt getRequestSentOk() { return m_rqst_sent_ok; }
      /// @brief Retrieves the request received successes for this message.
      /// @return the request received successes for this message.
      UInt getRequestReceivedOk() { return m_rqst_rcvd_ok; }

      /// @brief Retrieves the response send errors for this message.
      /// @return the response send errors for this message.
      UInt getResponseSentErrors() { return m_resp_sent_err; }
      /// @brief Retrieves the response receive errors for this message.
      /// @return the response receive errors for this message.
      UInt getResponseReceivedErrors() { return m_resp_rcvd_err; }
      /// @brief Retrieves the response send successes that were accepted for this message.
      /// @return the request send successes that were accepted for this message.
      UInt getResponseSentOkAccepted() { return m_resp_sent_accept; }
      /// @brief Retrieves the response send successes that were rejected for this message.
      /// @return the request send successes that were rejected for this message.
      UInt getResponseSentOkRejected() { return m_resp_sent_reject; }
      /// @brief Retrieves the response receive successes that were accepted for this message.
      /// @return the request receive successes that were accepted for this message.
      UInt getResponseReceivedOkAccepted() { return m_resp_rcvd_accept; }
      /// @brief Retrieves the response receive successes that were rejected for this message.
      /// @return the response receive successes that were rejected for this message.
      UInt getResponseReceivedOkRejected() { return m_resp_rcvd_reject; }

      /// @brief Increments the request send errors for this message.
      /// @return the request send errors for this message.
      UInt incRequestSentErrors() { return ++m_rqst_sent_err; }
      /// @brief Increments the request receive errors for this message.
      /// @return the request receive errors for this message.
      UInt incRequestReceivedErrors() { return ++m_rqst_rcvd_err; }
      /// @brief Increments the request send successes for this message.
      /// @return the request send successes for this message.
      UInt incRequestSentOk() { return ++m_rqst_sent_ok; }
      /// @brief Increments the request received successes for this message.
      /// @return the request received successes for this message.
      UInt incRequestReceivedOk() { return ++m_rqst_rcvd_ok; }

      /// @brief Increments the response send errors for this message.
      /// @return the response send errors for this message.
      UInt incResponseSentErrors() { return ++m_resp_sent_err; }
      /// @brief Increments the response receive errors for this message.
      /// @return the response receive errors for this message.
      UInt incResponseReceivedErrors() { return ++m_resp_rcvd_err; }
      /// @brief Increments the response send successes that were accepted for this message.
      /// @return the request send successes that were accepted for this message.
      UInt incResponseSentOkAccepted() { return ++m_resp_sent_accept; }
      /// @brief Increments the response send successes that were rejected for this message.
      /// @return the request send successes that were rejected for this message.
      UInt incResponseSentOkRejected() { return ++m_resp_sent_reject; }
      /// @brief Increments the response receive successes that were accepted for this message.
      /// @return the request receive successes that were accepted for this message.
      UInt incResponseReceivedOkAccepted() { return ++m_resp_rcvd_accept; }
      /// @brief Increments the response receive successes that were rejected for this message.
      /// @return the response receive successes that were rejected for this message.
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

   /// @brief Contains the peer information including the statistics for the individual messages.
   class Peer
   {
   public:
      /// @brief Class constructor.
      /// @param name the peer name.
      /// @param tmplt the list of messages to maintain statistics for.
      Peer(cpStr name, const EStatistics::MessageStatsMap &tmplt);
      /// @brief Class constructor.
      /// @param name the peer name.
      /// @param tmplt the list of messages to maintain statistics for.
      Peer(const EString &name, const EStatistics::MessageStatsMap &tmplt);
      /// @brief Class constructor.
      /// @param p the Peer object to copy.
      Peer(const Peer &p);

      /// @brief Retrieves the statistics for a specific message.
      /// @return the statistics for a specific message.
      EStatistics::MessageStats &getMessageStats(UInt msgid);

      /// @brief Retrieves the peer name.
      /// @return the peer name.
      EString &getName() { return m_name; }
      /// @brief Retrieves the time stamp of the last activity.
      /// @return the time stamp of the last activity.
      ETime &getLastActivity() { return m_lastactivity; }
      /// @brief Assigns the time stamp of the last activity.
      /// @return the time stamp of the last activity.
      ETime &setLastActivity() { return m_lastactivity = ETime::Now(); }
      /// @brief Retrieves the message statistics collection for this peer.
      /// @return the message statistics collection for this peer.
      EStatistics::MessageStatsMap &getMessageStats() { return m_msgstats; }
      /// @brief Sets all message statistics counters to zero.
      Void reset();

      /// @cond DOXYGEN_EXCLUDE
      #define INCREMENT_MESSAGE_STAT(__id,__func)  \
      {                                            \
         auto srch = m_msgstats.find(__id);        \
         if (srch == m_msgstats.end() )            \
            return 0;                              \
         setLastActivity();                        \
         return srch->second.__func();             \
      }
      /// @endcond

      /// @brief Increments the request send errors for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated request send errors.
      UInt incRequestSentErrors(UInt msgid)           { INCREMENT_MESSAGE_STAT(msgid, incRequestSentErrors); }
      /// @brief Increments the request receive errors for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated request receive errors.
      UInt incRequestReceivedErrors(UInt msgid)       { INCREMENT_MESSAGE_STAT(msgid, incRequestReceivedErrors); }
      /// @brief Increments the request send successes for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated request send successes.
      UInt incRequestSentOk(UInt msgid)               { INCREMENT_MESSAGE_STAT(msgid, incRequestSentOk); }
      /// @brief Increments the request receive successes for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated request receive successes.
      UInt incRequestReceivedOk(UInt msgid)           { INCREMENT_MESSAGE_STAT(msgid, incRequestReceivedOk); }

      /// @brief Increments the response send errors for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated response send errors.
      UInt incResponseSentErrors(UInt msgid)          { INCREMENT_MESSAGE_STAT(msgid, incResponseSentErrors); }
      /// @brief Increments the response receive errors for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated response receive errors.
      UInt incResponseReceivedErrors(UInt msgid)      { INCREMENT_MESSAGE_STAT(msgid, incResponseReceivedErrors); }
      /// @brief Increments the response send success aceepted for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated response send success aceepted.
      UInt incResponseSentOkAccepted(UInt msgid)      { INCREMENT_MESSAGE_STAT(msgid, incResponseSentOkAccepted); }
      /// @brief Increments the response send success rejected for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated response send success rejected.
      UInt incResponseSentOkRejected(UInt msgid)      { INCREMENT_MESSAGE_STAT(msgid, incResponseSentOkRejected); }
      /// @brief Increments the response received success aceepted for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated response received success aceepted.
      UInt incResponseReceivedOkAccepted(UInt msgid)  { INCREMENT_MESSAGE_STAT(msgid, incResponseReceivedOkAccepted); }
      /// @brief Increments the response received success rejected for the specified message.
      /// @param msgid the message identifier.
      /// @return the updated response received success rejected.
      UInt incResponseReceivedOkRejected(UInt msgid)  { INCREMENT_MESSAGE_STAT(msgid, incResponseReceivedOkRejected); }

      /// @cond DOXYGEN_EXCLUDE
      #undef INCREMENT_MESSAGE_STAT
      /// @endcond

   private:
      EString m_name;
      ETime m_lastactivity;
      ERWLock m_lock;
      EStatistics::MessageStatsMap m_msgstats;
   };
   /// @brief Defines a collection of peers.
   typedef std::unordered_map<std::string,EStatistics::Peer> PeerMap;

   /// @brief Represents an interface to track peer and message statistics for.
   class Interface
   {
   public:
      /// @brief Class constructor.
      /// @param id the interface identifier.
      /// @param protocol the interface protocol.
      /// @param name the interface name.
      Interface(EStatistics::InterfaceId id, EStatistics::ProtocolType protocol, cpStr name);
      /// @brief Class constructor.
      /// @param id the interface identifier.
      /// @param protocol the interface protocol.
      /// @param name the interface name.
      Interface(EStatistics::InterfaceId id, EStatistics::ProtocolType protocol, const EString &name);
      /// @brief Copy constructor.
      /// @param i the interface to copy.
      Interface(const Interface &i);

      /// @brief Retrieves the interface ID.
      /// @return the interface ID.
      EStatistics::InterfaceId getId() { return m_id; }
      /// @brief Retrieves the interface name.
      /// @return the interface name.
      EString &getName() { return m_name; }
      /// @brief Retrieves the interface protocol.
      /// @return the interface protocol.
      ProtocolType getProtocol() { return m_protocol; }
      /// @brief Retrieves the collection of peers for this interface.
      /// @return the collection of peers for this interface.
      PeerMap &getPeers() { return m_peers; }

      /// @brief Retrieves the specified peer with option to add if it does not exist.
      /// @return the specified peer object.
      Peer &getPeer(const EString &peer, Bool addFlag = True);
      /// @brief Adds the specified peer.
      /// @return the added peer.
      Peer &addPeer(const EString &peer);
      /// @brief Removes the specified peer.
      Void removePeer(const EString &peer);
      /// @brief Resets the message counters to zeroes for all peers.
      Void reset();

      /// @brief Adds a message to the statistics message template for this interface.
      /// @return the added message statistics template.
      MessageStats &addMessageStatsTemplate(EStatistics::MessageId msgid, const EString &name);
      /// @brief Retrieves the collection of message statistics.
      /// @return the collection of message statistics.
      MessageStatsMap &getMessageStatsTemplate() { return m_msgstats_template; }

      /// @cond DOXYGEN_EXCLUDE
      #define INCREMENT_MESSAGE_STAT(__peer,__id,__func) \
      {                                                  \
         EStatistics::Peer &__p( getPeer(__peer) );      \
         return __p.__func(__id);                        \
      }
      /// @endcond

      /// @brief Increments the request send errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request send errors for the specified peer and message.
      UInt incRequestSentErrors(cpStr peer, UInt msgid)           { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestSentErrors); }
      /// @brief Increments the request receive errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request receive errors for the specified peer and message.
      UInt incRequestReceivedErrors(cpStr peer, UInt msgid)       { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestReceivedErrors); }
      /// @brief Increments the request send successes for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request send successes for the specified peer and message.
      UInt incRequestSentOk(cpStr peer, UInt msgid)               { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestSentOk); }
      /// @brief Increments the request receive successes for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request receive successes for the specified peer and message.
      UInt incRequestReceivedOk(cpStr peer, UInt msgid)           { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incRequestReceivedOk); }

      /// @brief Increments the response send errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send errors for the specified peer and message.
      UInt incResponseSentErrors(cpStr peer, UInt msgid)          { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseSentErrors); }
      /// @brief Increments the response receive errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response receive errors for the specified peer and message.
      UInt incResponseReceivedErrors(cpStr peer, UInt msgid)      { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseReceivedErrors); }
      /// @brief Increments the response send success accepted for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send success accepted for the specified peer and message.
      UInt incResponseSentOkAccepted(cpStr peer, UInt msgid)      { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseSentOkAccepted); }
      /// @brief Increments the response send success rejected for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send success rejected for the specified peer and message.
      UInt incResponseSentOkRejected(cpStr peer, UInt msgid)      { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseSentOkRejected); }
      /// @brief Increments the response receive success accepted for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send receive accepted for the specified peer and message.
      UInt incResponseReceivedOkAccepted(cpStr peer, UInt msgid)  { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseReceivedOkAccepted); }
      /// @brief Increments the response receive rejected accepted for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send receive rejected for the specified peer and message.
      UInt incResponseReceivedOkRejected(cpStr peer, UInt msgid)  { EString p(peer); INCREMENT_MESSAGE_STAT(p, msgid, incResponseReceivedOkRejected); }

      /// @brief Increments the request send errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request send errors for the specified peer and message.
      UInt incRequestSentErrors(const std::string &peer, UInt msgid)          { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestSentErrors); }
      /// @brief Increments the request receive errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request receive errors for the specified peer and message.
      UInt incRequestReceivedErrors(const std::string &peer, UInt msgid)      { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestReceivedErrors); }
      /// @brief Increments the request send successes for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request send successes for the specified peer and message.
      UInt incRequestSentOk(const std::string &peer, UInt msgid)              { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestSentOk); }
      /// @brief Increments the request receive successes for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the request receive successes for the specified peer and message.
      UInt incRequestReceivedOk(const std::string &peer, UInt msgid)          { INCREMENT_MESSAGE_STAT(peer, msgid, incRequestReceivedOk); }

      /// @brief Increments the response send errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send errors for the specified peer and message.
      UInt incResponseSentErrors(const std::string &peer, UInt msgid)         { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseSentErrors); }
      /// @brief Increments the response receive errors for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response receive errors for the specified peer and message.
      UInt incResponseReceivedErrors(const std::string &peer, UInt msgid)     { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseReceivedErrors); }
      /// @brief Increments the response send success accepted for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send success accepted for the specified peer and message.
      UInt incResponseSentOkAccepted(const std::string &peer, UInt msgid)     { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseSentOkAccepted); }
      /// @brief Increments the response send success rejected for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send success rejected for the specified peer and message.
      UInt incResponseSentOkRejected(const std::string &peer, Int msgid)      { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseSentOkRejected); }
      /// @brief Increments the response receive success accepted for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send receive accepted for the specified peer and message.
      UInt incResponseReceivedOkAccepted(const std::string &peer, UInt msgid) { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseReceivedOkAccepted); }
      /// @brief Increments the response receive rejected accepted for the specified peer and message.
      /// @param peer the associated peer.
      /// @param msgid the associated message ID.
      /// @return the response send receive rejected for the specified peer and message.
      UInt incResponseReceivedOkRejected(const std::string &peer, UInt msgid) { INCREMENT_MESSAGE_STAT(peer, msgid, incResponseReceivedOkRejected); }

      /// @cond DOXYGEN_EXCLUDE
      #undef INCREMENT_MESSAGE_STAT
      /// @endcond

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

   /// @brief Retrieves the requested interface object.
   /// @param id the ID of the requested interface object.
   /// @return the requested interface object.
   /// @throws EError (interface not found)
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

   /// @brief Adds/updates the interface.
   /// @param id the interface ID.
   /// @param protocol the interface protocol.
   /// @param intfc the interface name.
   /// @return the interface object.
   static Interface &addInterface(EStatistics::InterfaceId id, ProtocolType protocol, const EString &intfc)
   {
      EWRLock l(m_lock);
      auto it = m_interfaces.emplace(id, Interface(id,protocol,intfc));
      return it.first->second;
   }

   /// @brief Removes the specified interface.
   /// @param id the ID of the interface to remove.
   static Void removeInterface(EStatistics::InterfaceId id)
   {
      EWRLock l(m_lock);
      auto srch = m_interfaces.find(id);
      if (srch != m_interfaces.end())
         m_interfaces.erase( srch );
   }

   /// @brief Retrieves the interface collection.
   /// @return the interface collection.
   static InterfaceMap &getInterfaces() { return m_interfaces; }

   /// @brief Initializes the statistics class.
   /// @param logger the log object to write messages to.
   static Void init(ELogger &logger);
   /// @brief Uninitializes teh statistics class.
   static Void uninit();
   /// @brief Sets the message counters to zero for all interfaces, peers and messages.
   static Void reset();

private:
   static DiameterHook m_hook_error;
   static DiameterHook m_hook_success;

   static ERWLock m_lock;
   static EStatistics::InterfaceMap m_interfaces;
};

#endif // #ifndef __ESTATS_H
