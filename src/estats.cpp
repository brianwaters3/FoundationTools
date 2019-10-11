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
std::unordered_map<EStatistics::InterfaceId,EStatistics::Interface> EStatistics::m_interfaces;

Void EStatistics::init(ELogger &logger)
{
   UInt mask_error = HOOK_MASK( HOOK_MESSAGE_PARSING_ERROR, HOOK_MESSAGE_ROUTING_ERROR /*, HOOK_MESSAGE_DROPPED*/  );
   UInt mask_success = HOOK_MASK( HOOK_MESSAGE_RECEIVED, HOOK_MESSAGE_SENDING );

   m_hook_error.registerHook( mask_error );
   m_hook_success.registerHook( mask_success );

   m_hook_error.setLogger( logger );
   m_hook_success.setLogger( logger );
}

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
            return ah->avp_value->i32 == 2001; /* DIAMETER_SUCCESS */

         ret = fd_msg_browse_internal(a, MSG_BRW_NEXT, (msg_or_avp**)&a, NULL);
      }
   }

   return false;
}

Void EStatistics::DiameterHook::process(enum fd_hook_type type, struct msg * msg,
   struct peer_hdr * peer, void * other, struct fd_hook_permsgdata *pmd)
{
   struct msg_hdr* hdr = NULL;

   if ( !msg || !fd_msg_hdr(msg,&hdr) )
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
                  case HOOK_MESSAGE_RECEIVED:   { intfc.incResponseReceivedOkAccepted( peer->info.pi_diamid, msgid ); break; }
                  case HOOK_MESSAGE_SENDING:    { intfc.incResponseSentOkAccepted( peer->info.pi_diamid, msgid ); break; }
               }
            }
            else
            {
               switch (type)
               {
                  case HOOK_MESSAGE_RECEIVED:   { intfc.incResponseReceivedOkRejected( peer->info.pi_diamid, msgid ); break; }
                  case HOOK_MESSAGE_SENDING:    { intfc.incResponseSentOkRejected( peer->info.pi_diamid, msgid ); break; }
               }
            }
         }
         else
         {
            switch (type)
            {
               case HOOK_MESSAGE_PARSING_ERROR: { intfc.incResponseSentErrors( peer->info.pi_diamid, msgid );      break; }
               case HOOK_MESSAGE_ROUTING_ERROR: { intfc.incResponseReceivedErrors( peer->info.pi_diamid, msgid );  break; }
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
