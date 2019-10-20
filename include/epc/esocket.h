/*
* Copyright (c) 2009-2019 Brian Waters
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

#ifndef __esocket_h_included
#define __esocket_h_included

#include <unordered_map>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "ebase.h"
#include "ecbuf.h"
#include "eerror.h"
#include "estatic.h"
#include "estring.h"
#include "ethread.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace ESocket
{
   namespace TCP
   {
      class Talker;
      class Listener;
   }
   class UDP;
   class Thread;

   DECLARE_ERROR_ADVANCED(AddressError_UnknownAddressType);
   DECLARE_ERROR_ADVANCED(AddressError_CannotConvertInet2Inet6);
   DECLARE_ERROR_ADVANCED(AddressError_CannotConvertInet62Inet);
   DECLARE_ERROR_ADVANCED(AddressError_ConvertingToString);
   DECLARE_ERROR_ADVANCED(AddressError_UndefinedFamily);

   DECLARE_ERROR_ADVANCED(BaseError_UnableToCreateSocket);
   DECLARE_ERROR_ADVANCED(BaseError_GetPeerNameError);

   DECLARE_ERROR_ADVANCED(TcpTalkerError_InvalidRemoteAddress);
   DECLARE_ERROR_ADVANCED(TcpTalkerError_UnableToConnect);
   DECLARE_ERROR_ADVANCED(TcpTalkerError_UnableToRecvData);
   DECLARE_ERROR_ADVANCED4(TcpTalkerError_InvalidSendState);
   DECLARE_ERROR_ADVANCED4(TcpTalkerError_ReadingWritePacketLength);
   DECLARE_ERROR_ADVANCED(TcpTalkerError_SendingPacket);

   DECLARE_ERROR_ADVANCED(TcpListenerError_UnableToListen);
   DECLARE_ERROR_ADVANCED(TcpListenerError_UnableToBindSocket);
   DECLARE_ERROR_ADVANCED(TcpListenerError_UnableToAcceptSocket);

   DECLARE_ERROR_ADVANCED(UdpError_AlreadyBound);
   DECLARE_ERROR_ADVANCED(UdpError_UnableToBindSocket);
   DECLARE_ERROR_ADVANCED(UdpError_UnableToRecvData);
   DECLARE_ERROR_ADVANCED(UdpError_SendingPacket);
   DECLARE_ERROR_ADVANCED4(UdpError_ReadingWritePacketLength);

   DECLARE_ERROR_ADVANCED(ThreadError_UnableToOpenPipe);
   DECLARE_ERROR_ADVANCED(ThreadError_UnableToReadPipe);
   DECLARE_ERROR_ADVANCED(ThreadError_UnableToWritePipe);

   class Address
   {
   public:
      enum class Family
      {
         Undefined,
         INET,
         INET6
      };

      Address() : m_addr() {}
      Address(cpStr addr, UShort port) { setAddress(addr,port); }
      Address(struct sockaddr_in &addr) { memcpy(&m_addr, &addr, sizeof(addr)); }
      Address(struct sockaddr_in6 &addr) { memcpy(&m_addr, &addr, sizeof(addr)); }
      Address(const Address &addr) { memcpy(&m_addr, &addr, sizeof(addr)); }

      operator EString() const
      {
         Char buf[INET6_ADDRSTRLEN];

         if (m_addr.ss_family == AF_INET)
         {
            if (!inet_ntop(m_addr.ss_family,&((struct sockaddr_in*)&m_addr)->sin_addr.s_addr,buf,sizeof(buf)))
               throw AddressError_ConvertingToString();
         }
         else // AF_INET6
         {
            if (!inet_ntop(m_addr.ss_family,&((struct sockaddr_in6*)&m_addr)->sin6_addr.s6_addr,buf,sizeof(buf)))
               throw AddressError_ConvertingToString();
         }
         return EString(buf);
      }

      operator UShort() const
      {
         if (m_addr.ss_family == AF_INET)
            return ntohs(((struct sockaddr_in*)&m_addr)->sin_port);
         if (m_addr.ss_family == AF_INET6)
            return ntohs(((struct sockaddr_in6*)&m_addr)->sin6_port);
         throw AddressError_UndefinedFamily();
      }

      EString getAddress() const { return *this; }
      UShort getPort() const { return *this; }

      struct sockaddr *getSockAddr()
      {
         return (struct sockaddr *)&m_addr;
      }

      socklen_t getSockAddrLen() const
      {
         if (m_addr.ss_family == AF_INET)
            return sizeof(struct sockaddr_in);
         if (m_addr.ss_family == AF_INET6)
            return sizeof(struct sockaddr_in6);
         return sizeof(struct sockaddr_storage);
      }

      Address &operator=(const Address& addr)
      {
         memcpy(&m_addr, &addr, sizeof(m_addr));
         return *this;
      }

      Address &operator=(UShort port)
      {
         return setAddress(port);
      }

      Family getFamily() const
      {
         return m_addr.ss_family == AF_INET ? Family::INET :
                m_addr.ss_family == AF_INET6 ? Family::INET6 : Family::Undefined;
      }

      struct sockaddr_in &getInet()
      {
         if (m_addr.ss_family != AF_INET)
            throw AddressError_CannotConvertInet2Inet6();
         return (struct sockaddr_in &)m_addr;
      }

      struct sockaddr_in6 &getInet6()
      {
         if (m_addr.ss_family != AF_INET6)
            throw AddressError_CannotConvertInet62Inet();
         return (struct sockaddr_in6 &)m_addr;
      }

      Address &setAddress(cpStr addr, UShort port)
      {
         clear();
         if (inet_pton(AF_INET,addr,&((struct sockaddr_in*)&m_addr)->sin_addr) == 1)
         {
            ((struct sockaddr_in*)&m_addr)->sin_family = AF_INET;
            ((struct sockaddr_in*)&m_addr)->sin_port = htons(port);
            return *this;
         }

         clear();
         if (inet_pton(AF_INET6,addr,&((struct sockaddr_in6*)&m_addr)->sin6_addr) == 1)
         {
            ((struct sockaddr_in6*)&m_addr)->sin6_family = AF_INET6;
            ((struct sockaddr_in6*)&m_addr)->sin6_port = htons(port);
            ((struct sockaddr_in6*)&m_addr)->sin6_flowinfo = 0;
            ((struct sockaddr_in6*)&m_addr)->sin6_scope_id = 0;
            return *this;
         }

         throw AddressError_UnknownAddressType();
      }

      Address &setAddress(UShort port)
      {
         ((struct sockaddr_in6*)&m_addr)->sin6_family = AF_INET6;
         ((struct sockaddr_in6*)&m_addr)->sin6_port = htons(port);
         ((struct sockaddr_in6*)&m_addr)->sin6_flowinfo = 0;
         ((struct sockaddr_in6*)&m_addr)->sin6_scope_id = 0;
         ((struct sockaddr_in6*)&m_addr)->sin6_addr = in6addr_any;
         return *this;
      }

      Address &clear()
      {
         memset( &m_addr, 0, sizeof(m_addr) );
         return *this;
      }

   private:
      struct sockaddr_storage m_addr;
   };

   class Base
   {
      friend class TCP::Talker;
      friend class TCP::Listener;
      friend class UDP;
      friend class Thread;

   public:
      enum class SocketType
      {
         Undefined,
         TcpTalker,
         TcpListener,
         Udp
      };

      virtual ~Base();

      Thread &getThread() { return m_thread; }

      SocketType getSocketType() { return m_socktype; }
      Int getFamily() { return m_family; }
      Int getType() { return m_type; }
      Int getProtocol() { return m_protocol; }

      Int getError() { return m_error; }

      Void close();
      virtual Void disconnect();

      Int getHandle() { return m_handle; }

   protected:
      Base(Thread &thread, SocketType socktype, Int family, Int type, Int protocol);
      Void createSocket(Int family, Int type, Int protocol);
      Void assignAddress(cpStr ipaddr, UShort port, Int family, Int socktype,
                        Int flags, Int protocol, struct addrinfo **address);
      Int setError();
      Void setError(Int error) { m_error = error; }
      Void setHandle(Int handle);
      Base &setFamily(Int family)
      {
         m_family = family;
         return *this;
      }

      Address &setLocalAddress(Address &addr);
      Address &setRemoteAddress(Address &addr);

      virtual Bool onReceive();
      virtual Void onConnect();
      virtual Void onClose();
      virtual Void onError();

   private:
      Void setOptions();

      Thread &m_thread;

      SocketType m_socktype;
      Int m_family;
      Int m_type;
      Int m_protocol;
      Int m_error;

      Int m_handle;
   };
   typedef Base* BasePtr;
   typedef std::unordered_map<Int,BasePtr> BasePtrMap;

   namespace TCP
   {
      class Talker : public Base
      {
         friend Thread;

      public:
         enum class State
         {
            Undefined,
            Disconnected,
            Connecting,
            Connected
         };

         Talker(Thread &thread, Int bufsize=2097152);
         virtual ~Talker();

         Address getLocal() { return m_local; }
         EString getLocalAddress() { return m_local; }
         UShort getLocalPort() { return m_local; }
         Talker &setLocal(cpStr addr, UShort port) { m_local.setAddress(addr,port); return *this; }
         Talker &setLocal(const Address &addr) { m_local = addr; return *this; }

         Address getRemote() { return m_remote; }
         EString getRemoteAddress() { return m_remote; }
         UShort getRemotePort() { return m_remote; }
         Talker &setRemote(cpStr addr, UShort port) { m_remote.setAddress(addr,port); return *this; }
         Talker &setRemote(const Address &addr) { m_remote = addr; return *this; }

         Void connect();
         Void connect(Address &addr)
         {
            m_remote = addr;
            connect();
         }
         Void connect(cpStr addr, UShort port)
         {
            m_remote.setAddress( addr, port );
            connect();
         }

         Int bytesPending() { return m_rbuf.used(); }

         Int peek(pUChar dest, Int len);
         Int read(pUChar dest, Int len);
         Void write(pUChar src, Int len);

         Bool getSending() { return m_sending; }

         State getState() { return m_state; }

         cpStr getStateDescription(State state)
         {
            switch (state)
            {
               case State::Disconnected:  return "DISCONNECTED";
               case State::Connecting:    return "CONNECTING";
               case State::Connected:     return "CONNECTED";
               default:                   return "UNDEFINED";
            }
         }

         Void disconnect();

         virtual Bool onReceive();
         virtual Void onConnect();
         virtual Void onClose();
         virtual Void onError();

      protected:
         Talker &setAddresses()
         {
            Base::setLocalAddress( m_local );
            Base::setRemoteAddress( m_remote );
         }

         Talker &setState(State state) { m_state = state; return *this; }
         Int recv();
         Void send(Bool override = False);

      private:
         Int send(pUChar pData, Int length);

         State m_state;
         Address m_local;
         Address m_remote;
         EMutexPrivate m_sendmtx;
         Bool m_sending;

         ECircularBuffer m_rbuf;
         ECircularBuffer m_wbuf;
      };

      class Listener : public Base
      {
         friend Thread;

      public:
         enum class State
         {
            Undefined,
            Listening
         };

         Listener(Thread &thread, Address::Family family = Address::Family::INET6);
         Listener(Thread &thread, UShort port, Address::Family family = Address::Family::INET6);
         Listener(Thread &thread, UShort port, Int backlog, Address::Family family = Address::Family::INET6);

         virtual ~Listener() {}

         State getState() { return m_state; }

         Address &getLocalAddress() { return m_local; }

         Void setPort(UShort port) { m_local = port; setFamily( m_local.getFamily() == Address::Family::INET?AF_INET:AF_INET6 ); }
         UShort getPort() { return m_local; }

         Void setBacklog(Int backlog) { m_backlog = backlog; };
         Int getBacklog() { return m_backlog; }

         Void listen();
         Void listen(UShort port, Int backlog)
         {
            setPort(port);
            setBacklog(backlog);
            listen();
         }

         virtual Talker *createSocket(Thread &thread) = 0;
         Talker *createSocket() { return createSocket(getThread()); }

         virtual Void onClose();
         virtual Void onError();

      private:
         Void bind();
         Listener &setState( State state ) { m_state = state; }

         State m_state;
         Address m_local;
         Int m_backlog;
      };
   }

   class UDP : public Base
   {
      friend Thread;

   public:
      UDP(Thread &thread, Int bufsize=2097152);
      UDP(Thread &thread, UShort port, Int bufsize=2097152);
      UDP(Thread &thread, cpStr ipaddr, UShort port, Int bufsize=2097152);
      UDP(Thread &thread, Address &addr, Int bufsize=2097152);
      virtual ~UDP();

      Address getLocal() { return m_local; }
      EString getLocalAddress() { return m_local; }
      UShort getLocalPort() { return m_local; }
      UDP &setLocal(cpStr addr, UShort port) { m_local.setAddress(addr,port); return *this; }
      UDP &setLocal(const Address &addr) { m_local = addr; return *this; }

      Int bytesPending() { return m_rbuf.used(); }

      Void write(const Address &to, pVoid src, Int len);

      Bool getSending() { return m_sending; }

      Void bind(UShort port);
      Void bind(cpStr ipaddr, UShort port);
      Void bind(const Address &addr);

      Void disconnect();

      virtual Void onReceive(const Address &from, pVoid msg, Int len);
      virtual Void onError();

   protected:
      UDP &setAddresses()
      {
         Base::setLocalAddress( m_local );
      }

      Int recv();
      Void send(Bool override = False);

   private:
      #pragma pack(push,1)
      struct UDPMessage
      {
         size_t total_length;
         size_t data_length;
         Address addr;
         UChar data[0];
      };
      #pragma pack(pop)

      Void onConnect() {}
      Void onClose() {}
      Bool onReceive();
      Void bind();
      Bool readMessage(UDPMessage &msg);
      Int send(Address &addr, cpVoid pData, Int length);

      Address m_local;
      EMutexPrivate m_sendmtx;
      Bool m_sending;

      UDPMessage *m_rcvmsg;
      UDPMessage *m_sndmsg;
      ECircularBuffer m_rbuf;
      ECircularBuffer m_wbuf;
   };

   class Thread : public EThreadPrivate
   {
      friend class TCP::Talker;
      friend class TCP::Listener;
      friend class UDP;

   public:
      Thread();
      virtual ~Thread();

      Void registerSocket(BasePtr socket);
      Void unregisterSocket(BasePtr socket);

      Int getError() { return m_error; }

   protected:
      virtual Void pumpMessages();
      virtual Void errorHandler(EError &err, Base *psocket) = 0;

      virtual Void onInit();
      virtual Void onQuit();

      virtual Void messageQueued();

      virtual Void onError();

      Void bump();
      Void clearBump();

      DECLARE_MESSAGE_MAP()

   private:
      Void setError(Int error) { m_error = error; }

      Bool pumpMessagesInternal();

      Void processSelectAccept(Base *psocket);
      Void processSelectConnect(Base *psocket);
      Void processSelectRead(Base *psocket);
      Void processSelectWrite(Base *psocket);
      Void processSelectError(Base *psocket);
      Void processSelectClose(Base *psocket);

      Int m_error;
      BasePtrMap m_socketmap;

      int getMaxFileDescriptor();

      fd_set m_master;
      int m_pipefd[2];
   };
}


#if 0
// class ESocketListen;
// class ESocketConverse;
class ESocketThread;

class ESocket
{
   friend class ESocketTcp;
   friend class ESocketUdp;
   friend class ESocketThread;

public:
   virtual ~ESocket();

   ESocketThread &getThread() { return m_thread; }

   Int getFamily() { return m_family; }
   Int getType() { return m_type; }
   Int getProtocol() { return m_protocol; }

   Int getError() { return m_error; }

   Void disconnect();
   Void close();

   Int getHandle() { return m_handle; }

protected:
   ESocket(ESocketThread &thread, Int family, Int type, Int protocol);
   Void createSocket(Int family, Int type, Int protocol);
   Void assignAddress(cpStr ipaddr, UShort port, Int family, Int socktype,
                      Int flags, Int protocol, struct addrinfo **address);
   Void setState(SOCKETSTATE state) { m_state = state; }
   Int setError();
   Void setError(Int error) { m_error = error; }
   Void setHandle(Int handle);

   Void bind();
   Void bind(UShort port);

   virtual Bool onReceive();
   virtual Void onConnect();
   virtual Void onClose();
   virtual Void onError();

private:
   Void setOptions();

   ESocketThread &m_thread;

   Int m_family;
   Int m_type;
   Int m_protocol;
   Int m_error;

   Int m_handle;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESocketTcpTalker : public ESocket
{
   friend ESocketThread;

public:
   enum class SocketState
   {
      undefined,
      disconnected,
      connecting,
      connected
   };

protected:

private:
};

typedef std::shared_ptr<ESocketTcpTalker> ESocketTcpTalkerPtr;

class ESocketTcpListener : public ESocket
{
   friend ESocketThread;

public:
   enum class State
   {
      undefined,
      listening
   };

   ESocketTcp(ESocketThread &thread)
      : ESocket(thread, AF_INET6, SOCK_STREAM, IPPROTO_TCP),
        m_state( State::undefined ),
        m_port( 0 ),
        m_backlog( -1 )
   {
   }

   ESocketTcp(ESocketThread &thread, UShort port)
      : ESocket(thread, AF_INET6, SOCK_STREAM, IPPROTO_TCP),
        m_state( State::undefined ),
        m_port( port ),
        m_backlog( -1 )
      {
      }

   ESocketTcp(ESocketThread &thread, UShort port, Int backlog)
      : ESocket(thread, AF_INET6, SOCK_STREAM, IPPROTO_TCP),
        m_state( State::undefined ),
        m_port( port ),
        m_backlog( backlog )
      {
      }

   State getState() { return m_state; }
   Void setPort(UShort port) { m_port = port; }
   UShort getPort() { return m_port; }

   Void setBacklog(Int backlog) { m_backlog = backlog; };
   Int getBacklog() { return m_backlog; }

   Void listen();
   Void listen(UShort port, Int backlog)
   {
      setPort(port);
      setBacklog(backlog);
      listen();
   }

   virtual ESocketTcpTalkerPtr createSocket(ESocketThread &sthread) = 0;
   ESocketTcpTalkerPtr createSocket() { return createSocket(getThread()); }

   virtual Void onClose();
   virtual Void onError();

private:
   State m_state;
   UShort m_port;
   Int m_backlog;
};



   Bool isConnected() { return getState() == CONNECTED; }
   EString m_ipaddr;
   Void setIpAddress(cpStr addr);
   cpStr getIpAddress();

   SOCKETSTATE m_state;
   cpStr getStateDescription(ESocket::SOCKETSTATE state);



   enum class SocketStyle
   {
      CONVERSE,
      LISTEN
   };
   SOCKETSTYLE m_style;





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(ESocketConverseError_UnableToConnect);
DECLARE_ERROR_ADVANCED4(ESocketConverseError_OutOfMemory);
DECLARE_ERROR_ADVANCED(ESocketConverseError_UnableToRecvData);
DECLARE_ERROR_ADVANCED4(ESocketConverseError_InvalidSendState);
DECLARE_ERROR_ADVANCED4(ESocketConverseError_ReadingWritePacketLength);
DECLARE_ERROR_ADVANCED(ESocketConverseError_SendingPacket);
DECLARE_ERROR_ADVANCED4(ESocketConverseError_UnrecognizedSendReturnValue);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESocketThread;

class ESocketConverse : public ESocket
{
   friend class ESocketThread;

public:
   ESocketConverse(ESocketThread *pthread, Int bufSize, Int family = AF_INET, Int type = SOCK_STREAM, Int protocol = 0);
   ~ESocketConverse();

   Void setRemoteIpAddress(cpStr ipaddr) { m_remoteipaddr = ipaddr; }
   cpStr getRemoteIpAddress() { return m_remoteipaddr.c_str(); }

   Void setRemotePort(UShort port) { m_remoteport = port; }
   UShort getRemotePort() { return m_remoteport; }

   Void connect();
   Void connect(cpStr ipaddr, UShort port);

   Int bytesPending() { return m_rbuf.used(); }

   Int peek(pUChar dest, Int len);
   Int read(pUChar dest, Int len);
   Void write(pUChar src, Int len);

   Bool getSending() { return m_sending; }

   virtual Bool onReceive();
   virtual Void onConnect();
   virtual Void onClose();
   virtual Void onError();

protected:
   Int recv();
   Void send(Bool override = False);

private:
   ESocketConverse() : ESocket(NULL, ESocket::CONVERSE), m_rbuf(0), m_wbuf(0) {}

   Int send(pUChar pData, Int length);

   EMutexPrivate m_sendmtx;
   Bool m_sending;
   ECircularBuffer m_rbuf;
   ECircularBuffer m_wbuf;

   EString m_remoteipaddr;
   UShort m_remoteport;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(ESocketListenError_UnableToListen);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESocketListen : public ESocket
{
public:
   ESocketListen(ESocketThread *pthread, Int size, Int family = AF_INET, Int type = SOCK_STREAM, Int protocol = 0);
   ESocketListen(ESocketThread *pthread, Int size, UShort port, Int family = AF_INET, Int type = SOCK_STREAM, Int protocol = 0);
   ESocketListen(ESocketThread *pthread, Int size, UShort port, Int depth, Int family = AF_INET, Int type = SOCK_STREAM, Int protocol = 0);
   virtual ~ESocketListen();

   Void setDepth(Int depth) { m_depth = depth; };
   Int getDepth() { return m_depth; }

   Void setBufferSize(Int size) { m_bufsize = size; }
   Int getBufferSize() { return m_bufsize; }

   Void listen();
   Void listen(UShort port, Int depth);

   virtual ESocketConverse *createSocket(ESocketThread *pthread);

   virtual Void onClose();
   virtual Void onError();

private:
   ESocketListen() : ESocket(NULL, ESocket::LISTEN) {}

   Int m_depth;
   Int m_bufsize;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(ESocketThreadError_UnableToOpenPipe);
DECLARE_ERROR_ADVANCED(ESocketThreadError_UnableToReadPipe);
DECLARE_ERROR_ADVANCED(ESocketThreadError_UnableToWritePipe);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESocketMap : public map<Int, ESocket *>
{
};

class ESocketThread : public EThreadPrivate
{
   friend class ESocketConverse;

public:
   ESocketThread();
   virtual ~ESocketThread();

   Void registerSocket(ESocket *psocket);
   Void unregisterSocket(ESocket *psocket);

   Int getError() { return m_error; }

protected:
   virtual Void pumpMessages();
   virtual Void errorHandler(EError &err, ESocket *psocket) = 0;

   virtual Void onInit();
   virtual Void onQuit();

   virtual Void messageQueued();

   virtual Void onError();

   Void bump();
   Void clearBump();

   DECLARE_MESSAGE_MAP()

private:
   Void setError(Int error) { m_error = error; }

   Bool pumpMessagesInternal();

   Void processSelectAccept(ESocket *psocket);
   Void processSelectConnect(ESocket *psocket);
   Void processSelectRead(ESocket *psocket);
   Void processSelectWrite(ESocket *psocket);
   Void processSelectError(ESocket *psocket);
   Void processSelectClose(ESocket *psocket);

   Int m_error;
   ESocketMap m_socketmap;

   int getMaxFileDescriptor();

   fd_set m_master;
   int m_pipefd[2];
};
#endif // #if 0

#endif // #define __esocket_h_included
