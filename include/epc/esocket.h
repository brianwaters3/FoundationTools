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

/// @brief The namespace for all socket related classes.
namespace ESocket
{
   namespace TCP
   {
      class Talker;
      class Listener;
   }
   class UDP;
   class Thread;

   /// @cond DOXYGEN_EXCLUDE
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
   /// @endcond

   /// @brief Encapsulates a sockaddr_storage structure that represents a socket address.
   class Address
   {
   public:
      /// @brief Defines the possible address family values.
      enum class Family
      {
         /// undefined
         Undefined,
         /// IPv4 address
         INET,
         /// IPv6 address
         INET6
      };

      /// @brief Default constructor.
      Address() : m_addr() {}
      /// @brief Class constructor.
      /// @param addr the IP address string (IPv4 or IPv6).
      /// @param port the IP port.
      Address(cpStr addr, UShort port) { setAddress(addr,port); }
      /// @brief Class constructor.
      /// @param addr the IPv4 socket address.
      Address(struct sockaddr_in &addr) { memcpy(&m_addr, &addr, sizeof(addr)); }
      /// @brief Class constructor.
      /// @param addr the IPv6 socket address.
      Address(struct sockaddr_in6 &addr) { memcpy(&m_addr, &addr, sizeof(addr)); }
      /// @brief Copy constructor.
      /// @param addr the Address object to copy.
      Address(const Address &addr) { memcpy(&m_addr, &addr, sizeof(addr)); }

      /// @brief Extracts a string object with the printable IP address.
      /// @return EString representation of the IP address.
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

      /// @brief Extracts the port.
      /// @return the port.
      operator UShort() const
      {
         if (m_addr.ss_family == AF_INET)
            return ntohs(((struct sockaddr_in*)&m_addr)->sin_port);
         if (m_addr.ss_family == AF_INET6)
            return ntohs(((struct sockaddr_in6*)&m_addr)->sin6_port);
         throw AddressError_UndefinedFamily();
      }

      /// @brief Retrieves the printable IP address.
      /// @return the printable IP address.
      EString getAddress() const { return *this; }
      /// @brief Retrievs the port.
      /// @return the port.
      UShort getPort() const { return *this; }

      /// @brief Retrieves a sockaddr pointer to the socket address.
      /// @return a sockaddr pointer to the socket address.
      struct sockaddr *getSockAddr()
      {
         return (struct sockaddr *)&m_addr;
      }

      /// @brief retrieves the length of the current socket address.
      /// @return the length of the current socket address.
      socklen_t getSockAddrLen() const
      {
         if (m_addr.ss_family == AF_INET)
            return sizeof(struct sockaddr_in);
         if (m_addr.ss_family == AF_INET6)
            return sizeof(struct sockaddr_in6);
         return sizeof(struct sockaddr_storage);
      }

      /// @brief Assignment operator.
      /// @param addr the Address object to copy.
      /// @return a reference to this Address object.
      Address &operator=(const Address& addr)
      {
         memcpy(&m_addr, &addr, sizeof(m_addr));
         return *this;
      }

      /// @brief Assigns a port value (allowing IPADDR_ANY).
      /// @param port the port.
      /// @return a reference to this Address object.
      Address &operator=(UShort port)
      {
         return setAddress(port);
      }

      /// @brief Retrieves the address family for this address.
      /// @return the address family for this address.
      Family getFamily() const
      {
         return m_addr.ss_family == AF_INET ? Family::INET :
                m_addr.ss_family == AF_INET6 ? Family::INET6 : Family::Undefined;
      }

      /// @brief Retrieves a reference to this address as an IPv4 address.
      /// @return a reference to this address as an IPv4 address.
      struct sockaddr_in &getInet()
      {
         if (m_addr.ss_family != AF_INET)
            throw AddressError_CannotConvertInet2Inet6();
         return (struct sockaddr_in &)m_addr;
      }

      /// @brief Retrieves a reference to this address as an IPv6 address.
      /// @return a reference to this address as an IPv6 address.
      struct sockaddr_in6 &getInet6()
      {
         if (m_addr.ss_family != AF_INET6)
            throw AddressError_CannotConvertInet62Inet();
         return (struct sockaddr_in6 &)m_addr;
      }

      /// @brief Assigns the socket address.
      /// @param addr the IP address.
      /// @param port the port.
      /// @return a reference to this address object.
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

      /// @brief Assigns the socket address.
      /// @param port the port.
      /// @return a reference to this address object.
      Address &setAddress(UShort port)
      {
         ((struct sockaddr_in6*)&m_addr)->sin6_family = AF_INET6;
         ((struct sockaddr_in6*)&m_addr)->sin6_port = htons(port);
         ((struct sockaddr_in6*)&m_addr)->sin6_flowinfo = 0;
         ((struct sockaddr_in6*)&m_addr)->sin6_scope_id = 0;
         ((struct sockaddr_in6*)&m_addr)->sin6_addr = in6addr_any;
         return *this;
      }

      /// @brief Clears this address.
      /// @return a reference to this address object.
      Address &clear()
      {
         memset( &m_addr, 0, sizeof(m_addr) );
         return *this;
      }

   private:
      struct sockaddr_storage m_addr;
   };

   /// @brief The base socket class.
   class Base
   {
      friend class TCP::Talker;
      friend class TCP::Listener;
      friend class UDP;
      friend class Thread;

   public:
      /// @brief Defines the possible socket types.
      enum class SocketType
      {
         /// an undefined socket
         Undefined,
         /// a TCP talker socket
         TcpTalker,
         /// a TCP listener socket
         TcpListener,
         /// a UDP socket
         Udp
      };

      /// @brief Virtual class destructor.
      virtual ~Base();

      /// @brief Retrieves the socket thread that this socket is associated with.
      /// @return the socket thread that this socket is associated with.
      Thread &getThread() { return m_thread; }

      /// @brief Retrieves the socket type.
      /// @return the socket type.
      SocketType getSocketType() { return m_socktype; }
      /// @brief Retrieves the address family.
      /// @return the address family.
      Int getFamily() { return m_family; }
      /// @brief Retrieves the socket type.
      /// @return the address family.
      Int getType() { return m_type; }
      /// @brief Retrieves the protocol.
      /// @return the protocol.
      Int getProtocol() { return m_protocol; }

      /// @brief Retrieves the last error value.
      /// @return the last error value.
      Int getError() { return m_error; }

      /// @brief Closes this socket.
      Void close();
      /// @brief Disconnects this socket.
      virtual Void disconnect();

      /// @brief Retrieves the socket file handle.
      /// @return the socket file handle.
      Int getHandle() { return m_handle; }

   protected:
      /// @cond DOXYGEN_EXCLUDE
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

      virtual Void onReceive();
      virtual Void onConnect();
      virtual Void onClose();
      virtual Void onError();
      /// @endcond

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

   /// @brief Namespace for TCP related classes.
   namespace TCP
   {
      /// @brief A TCP socket class capabile of sending and receiving data.
      class Talker : public Base
      {
         friend Thread;

      public:
         /// @brief The socket connection state.
         enum class State
         {
            /// undefined
            Undefined,
            /// socket is disconnected
            Disconnected,
            /// socket is connecting
            Connecting,
            /// socket is connected
            Connected
         };

         /// @brief Class constructor.
         /// @param thread the socket thread that this socket is associated with.
         /// @param bufsize the size of the send and receive circular buffers.
         Talker(Thread &thread, Int bufsize=2097152);
         /// @brief Class destrucor.
         virtual ~Talker();

         /// @brief Retrieves the local socket address.
         /// @return the local socket address.
         Address &getLocal() { return m_local; }
         /// @brief Retrieves the IP address associated with the local socket.
         /// @return the IP address associated with the local socket.
         EString getLocalAddress() const { return m_local; }
         /// @brief Retrieves the port associated with the local socket.
         /// @brief the port associated with the local socket.
         UShort getLocalPort() const { return m_local; }
         /// @brief Assigns the local socket address.
         /// @param addr the IP address.
         /// @param port the port.
         /// @return a reference to this Talker object.
         Talker &setLocal(cpStr addr, UShort port) { m_local.setAddress(addr,port); return *this; }
         /// @brief Assigns the local socket address.
         /// @param addr the address object to copy.
         /// @return a reference to this Talker object.
         Talker &setLocal(const Address &addr) { m_local = addr; return *this; }

         /// @brief Retrieves the remote socket address.
         /// @return the remote socket address.
         Address &getRemote() { return m_remote; }
         /// @brief Retrieves the IP address associated with the remote socket.
         /// @return the IP address associated with the remote socket.
         EString getRemoteAddress() const { return m_remote; }
         /// @brief Retrieves the port associated with the remote socket.
         /// @brief the port associated with the remote socket.
         UShort getRemotePort() const { return m_remote; }
         /// @brief Assigns the remote socket address.
         /// @param addr the IP address.
         /// @param port the port.
         /// @return a reference to this Talker object.
         Talker &setRemote(cpStr addr, UShort port) { m_remote.setAddress(addr,port); return *this; }
         /// @brief Assigns the remote socket address.
         /// @param addr the address object to copy.
         /// @return a reference to this Talker object.
         Talker &setRemote(const Address &addr) { m_remote = addr; return *this; }

         /// @brief Initiates an IP connection with to the previously assigned remote socket address.
         Void connect();
         /// @brief Initiates an IP connection.
         /// @param addr the remote socket address.
         Void connect(Address &addr)
         {
            m_remote = addr;
            connect();
         }
         /// @brief Initiates an IP connection.
         /// @param addr the remote socket IP address.
         /// @param port the remote socket port.
         Void connect(cpStr addr, UShort port)
         {
            m_remote.setAddress( addr, port );
            connect();
         }

         /// @brief Retrieves the number of bytes in the receive buffer.
         /// @return the number of bytes in the receive buffer.
         Int bytesPending() { return m_rbuf.used(); }

         /// @brief Rtrieves the specified number of bytes from the receive buffer
         ///    without updating the read position.
         /// @param dest the location to write the data read.
         /// @param len the desired number of bytes to read.
         /// @return the number of actual bytes read.
         Int peek(pUChar dest, Int len);
         /// @brief Rtrieves the specified number of bytes from the receive buffer.
         /// @param dest the location to write the data read.
         /// @param len the desired number of bytes to read.
         /// @return the number of actual bytes read.
         Int read(pUChar dest, Int len);
         /// @brief Writes data to the socket.  This is a thread safe method.
         /// @param src the location to the data to write.
         /// @param len the desired number of bytes to write.
         Void write(pUChar src, Int len);

         /// @brief Retrieves indication if this socket is in the process of sending data.
         /// @return True indicates that data is being sent, otherwise False.
         Bool getSending() { return m_sending; }

         /// @brief Retrieves the connection state.
         /// @return the connection state.
         State getState() { return m_state; }

         /// @brief Retrieves the description of the current connection state.
         /// @return the description of the current connection state.
         cpStr getStateDescription()
         {
            return getStateDescription( m_state );
         }
         /// @brief Retrieves the description of the connection state.
         /// @return the description of the connection state.
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

         /// @brief Disconnects this socket.
         Void disconnect();

         /// @brief Called when data has been received.
         virtual Void onReceive();
         /// @brief Called when a connection has been established.
         virtual Void onConnect();
         /// @brief Called when the socket has been closed.
         virtual Void onClose();
         /// @brief Called when an error is detected on the socket.
         virtual Void onError();

      protected:
         /// @cond DOXYGEN_EXCLUDE
         Talker &setAddresses()
         {
            Base::setLocalAddress( m_local );
            Base::setRemoteAddress( m_remote );
         }

         Talker &setState(State state) { m_state = state; return *this; }
         Int recv();
         Void send(Bool override = False);
         /// @endcond

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

      /// @brief Listens for incoming TCP/IP connections.
      class Listener : public Base
      {
         friend Thread;

      public:
         /// @brief Listener connection state.
         enum class State
         {
            /// undefined
            Undefined,
            /// listening for incoming connections
            Listening
         };

         /// @brief Class constructor.
         /// @param thread the socket thread that this socket is associated with.
         /// @param family the default address family.
         Listener(Thread &thread, Address::Family family = Address::Family::INET6);
         /// @brief Class constructor.
         /// @param thread the socket thread that this socket is associated with.
         /// @param port the port to listen on.
         /// @param family the default address family.
         Listener(Thread &thread, UShort port, Address::Family family = Address::Family::INET6);
         /// @brief Class constructor.
         /// @param thread the socket thread that this socket is associated with.
         /// @param port the port to listen on.
         /// @param backlog the maximum number of "unaccepted" connections.
         /// @param family the default address family.
         Listener(Thread &thread, UShort port, Int backlog, Address::Family family = Address::Family::INET6);

         /// @brief Class destructor.
         virtual ~Listener() {}

         /// @brief Retrieves the current socket state.
         /// @return the current socket state.
         State getState() { return m_state; }

         /// @brief Retrieves the local listening address.
         /// @return the local listening address.
         Address &getLocalAddress() { return m_local; }

         /// @brief Assigns the port to listen for incoming connections on.
         /// @param port the port to listen for incoming connections on.
         Void setPort(UShort port) { m_local = port; setFamily( m_local.getFamily() == Address::Family::INET?AF_INET:AF_INET6 ); }
         /// @brief Retrieves the port being listened on for incoming connections.
         /// @return the port being listened on for incoming connections.
         UShort getPort() { return m_local; }

         /// @brief Assigns the maximum number of "unaccepted" connections.
         /// @param backlog the maximum number of "unaccepted" connections.
         Void setBacklog(Int backlog) { m_backlog = backlog; };
         /// @brief Retrieves the maximum number of "unaccepted" connections.
         /// @return the maximum number of "unaccepted" connections.
         Int getBacklog() { return m_backlog; }

         /// @brief Starts listening for incoming connections.
         Void listen();
         /// @brief Starts listening for incoming connections.
         /// @param port the port to listen on.
         /// @param backlog the maximum number of "unaccepted" connections.
         Void listen(UShort port, Int backlog)
         {
            setPort(port);
            setBacklog(backlog);
            listen();
         }

         /// @brief Called to create a talking socket when a incoming connection is received.
         /// @param thread the socket thread the talking socket will be associated with.
         /// @return the created talking socket.
         virtual Talker *createSocket(Thread &thread) = 0;
         /// @brief Called to create a talking socket when a incoming connection is received.
         /// @return the created talking socket.
         Talker *createSocket() { return createSocket(getThread()); }

         /// @brief Called when this socket is closed.
         virtual Void onClose();
         /// @brief Called when an error is detected on this socket.
         virtual Void onError();

      private:
         Void bind();
         Listener &setState( State state ) { m_state = state; }

         State m_state;
         Address m_local;
         Int m_backlog;
      };
   }

   /// @brief A UDP socket class capabile of sending and receiving data.
   class UDP : public Base
   {
      friend Thread;

   public:
      /// @brief Class constructor.
      /// @param thread the socket thread the talking socket will be associated with.
      /// @param bufsize the size of the send and receive circular buffers.
      UDP(Thread &thread, Int bufsize=2097152);
      /// @brief Class constructor.
      /// @param thread the socket thread the talking socket will be associated with.
      /// @param port the local port to listen on.
      /// @param bufsize the size of the send and receive circular buffers.
      UDP(Thread &thread, UShort port, Int bufsize=2097152);
      /// @brief Class constructor.
      /// @param thread the socket thread the talking socket will be associated with.
      /// @param ipaddr the local IP address to listen on. 
      /// @param port the local port to listen on.
      /// @param bufsize the size of the send and receive circular buffers.
      UDP(Thread &thread, cpStr ipaddr, UShort port, Int bufsize=2097152);
      /// @brief Class constructor.
      /// @param thread the socket thread the talking socket will be associated with.
      /// @param addr the local socket address to listen on. 
      /// @param bufsize the size of the send and receive circular buffers.
      UDP(Thread &thread, Address &addr, Int bufsize=2097152);
      /// @brief Class destructor.
      virtual ~UDP();

      /// @brief Retrieves the local address for this socket.
      /// @return the local address for this socket.
      Address getLocal() { return m_local; }
      /// @brief Retrieves the IP address for this socket.
      /// @return the IP address for this socket.
      EString getLocalAddress() { return m_local; }
      /// @brief Retrieves the port for this socket.
      /// @return the port for this socket.
      UShort getLocalPort() { return m_local; }
      /// @brief Assigns the socket address for this socket.
      /// @param addr the IP address for this socket.
      /// @param port the port for this socket.
      /// @return a reference to this object.
      UDP &setLocal(cpStr addr, UShort port) { m_local.setAddress(addr,port); return *this; }
      /// @brief Assigns the socket address for this socket.
      /// @param addr the socket address for this socket.
      /// @return a reference to this object.
      UDP &setLocal(const Address &addr) { m_local = addr; return *this; }

      /// @brief Sends data to the specified recipient address.
      /// @param to the address to send the data to.
      /// @param src a pointer to the beginning of the data buffer to send.
      /// @param len the number of bytes to send.
      Void write(const Address &to, pVoid src, Int len);

      /// @brief Retrieves indication if this socket is in the process of sending data.
      /// @return True indicates that data is being sent, otherwise False.
      Bool getSending() { return m_sending; }

      /// @brief Binds this socket to a local port and IPADDR_ANY.
      /// @param port the port.
      Void bind(UShort port);
      /// @brief Binds this socket to a local address.
      /// @param ipaddr the IP address.
      /// @param port the port.
      Void bind(cpStr ipaddr, UShort port);
      /// @brief Binds this socket to a local address.
      /// @param addr the local socket address.
      Void bind(const Address &addr);

      /// @brief Disconnects the socket.
      Void disconnect();

      /// @brief Called for each message that is received.
      /// @param from the socket address that the data was received from.
      /// @param msg pointer to the received data.
      /// @param len number of bytes received.
      virtual Void onReceive(const Address &from, pVoid msg, Int len);
      /// @brief Called when an error is detected on this socket.
      virtual Void onError();

   protected:
      /// @cond DOXYGEN_EXCLUDE
      UDP &setAddresses()
      {
         Base::setLocalAddress( m_local );
      }

      Int recv();
      Void send(Bool override = False);
      /// @endcond

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
      Void onReceive();
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

   /// @brief The socket thread base class. An event based thread class capable of surfacing socket events.
   class Thread : public EThreadPrivate
   {
      friend class TCP::Talker;
      friend class TCP::Listener;
      friend class UDP;

   public:
      /// @brief Default constructor.
      Thread();
      /// @brief Class destructor.
      virtual ~Thread();

      /// @brief Called by the framework to register a Base derived socket object with this thread.
      /// @param socket the socket to register.
      Void registerSocket(BasePtr socket);
      /// @brief Called by the framework to unregister a Base derived socket object with this thread.
      /// @param socket the socket to unregister.
      Void unregisterSocket(BasePtr socket);

      /// @brief Called when an error is detected.
      Int getError() { return m_error; }

   protected:
      /// @cond DOXYGEN_EXCLUDE
      virtual Void pumpMessages();
      virtual Void errorHandler(EError &err, Base *psocket) = 0;

      virtual Void onInit();
      virtual Void onQuit();

      virtual Void messageQueued();

      virtual Void onError();

      Void bump();
      Void clearBump();

      DECLARE_MESSAGE_MAP()
      /// @endcond

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

#endif // #define __esocket_h_included
