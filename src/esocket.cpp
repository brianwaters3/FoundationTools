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

#include <sys/fcntl.h>
#include <unistd.h>
#include <csignal>

#include "esocket.h"

// #define epc_closesocket(a) ::close(a)
// #define EPC_LASTERROR errno
// #define EPC_INVALID_SOCKET -1
// #define EPC_EINPROGRESS EINPROGRESS
// #define EPC_EWOULDBLOCK EWOULDBLOCK
// #define EPC_EMSGSIZE EMSGSIZE
// #define EPC_SOCKET_ERROR -1
// typedef Int EPC_SOCKET;
// typedef void *PSOCKETOPT;
// typedef void *PSNDRCVBUFFER;
// typedef socklen_t EPC_SOCKLEN;

////////////////////////////////////////////////////////////////////////////////
// ESocketError
////////////////////////////////////////////////////////////////////////////////

namespace ESocket
{

/// @cond DOXYGEN_EXCLUDE

AddressError_UnknownAddressType::AddressError_UnknownAddressType()
{
   setSevere();
   setTextf("%s: Unknown address type", Name());
}

AddressError_CannotConvertInet2Inet6::AddressError_CannotConvertInet2Inet6()
{
   setSevere();
   setTextf("%s: Cannot convert an INET address to an INET6 address", Name());
}

AddressError_CannotConvertInet62Inet::AddressError_CannotConvertInet62Inet()
{
   setSevere();
   setTextf("%s: Cannot convert an INEt6 address to an INET address", Name());
}

AddressError_ConvertingToString::AddressError_ConvertingToString()
{
   setSevere();
   setTextf("%s: Error converting address to a string", Name());
}

AddressError_UndefinedFamily::AddressError_UndefinedFamily()
{
   setSevere();
   setTextf("%s: Undefined family", Name());
}

BaseError_UnableToCreateSocket::BaseError_UnableToCreateSocket()
{
   setSevere();
   setTextf("%s: Error creating socket - ", Name());
   appendLastOsError();
}

BaseError_GetPeerNameError::BaseError_GetPeerNameError()
{
   setSevere();
   setTextf("%s: Error executing getpeername() - ", Name());
   appendLastOsError();
}

TcpListenerError_UnableToBindSocket::TcpListenerError_UnableToBindSocket()
{
   setSevere();
   setTextf("%s: Error binding socket in bind() - ", Name());
   appendLastOsError();
}

TcpListenerError_UnableToAcceptSocket::TcpListenerError_UnableToAcceptSocket()
{
   setSevere();
   setTextf("%s: Error accepting new connection in accept() - ", Name());
   appendLastOsError();
}

TcpListenerError_UnableToListen::TcpListenerError_UnableToListen()
{
   setSevere();
   setTextf("%s: Error executing listen - ", Name());
   appendLastOsError();
}

TcpTalkerError_InvalidRemoteAddress::TcpTalkerError_InvalidRemoteAddress()
{
   setSevere();
   setTextf("%s: Invalid remote address", Name());
}

TcpTalkerError_UnableToConnect::TcpTalkerError_UnableToConnect()
{
   setSevere();
   setTextf("%s: Unable to connect - ", Name());
   appendLastOsError();
}

TcpTalkerError_UnableToRecvData::TcpTalkerError_UnableToRecvData()
{
   setSevere();
   setTextf("%s: Error while executing recv() - ", Name());
   appendLastOsError();
}

TcpTalkerError_InvalidSendState::TcpTalkerError_InvalidSendState(cpStr msg)
{
   setSevere();
   setTextf("%s: Invalid state while sending: %s", Name(), msg);
}

TcpTalkerError_ReadingWritePacketLength::TcpTalkerError_ReadingWritePacketLength(cpStr msg)
{
   setSevere();
   setTextf("%s: Unable to read the write packet length - %s", Name(), msg);
}

TcpTalkerError_SendingPacket::TcpTalkerError_SendingPacket()
{
   setSevere();
   setTextf("%s: Error while attempting to send a packet of data via send() - ", Name());
   appendLastOsError();
}

UdpError_AlreadyBound::UdpError_AlreadyBound()
{
   setSevere();
   setTextf("%s: Socket is already bound", Name());
}

UdpError_UnableToBindSocket::UdpError_UnableToBindSocket()
{
   setSevere();
   setTextf("%s: Error while attempting to bind socket - ", Name());
   appendLastOsError();
}

UdpError_UnableToRecvData::UdpError_UnableToRecvData()
{
   setSevere();
   setTextf("%s: Error receiving data - ", Name());
   appendLastOsError();
}

UdpError_SendingPacket::UdpError_SendingPacket()
{
   setSevere();
   setTextf("%s: Error while attempting to send a packet of data via sendto() - ", Name());
   appendLastOsError();
}

UdpError_ReadingWritePacketLength::UdpError_ReadingWritePacketLength(cpStr msg)
{
   setSevere();
   setTextf("%s: Unable to read the write packet length - %s", Name(), msg);
}

ThreadError_UnableToOpenPipe::ThreadError_UnableToOpenPipe()
{
   setSevere();
   setTextf("%s: Error while opening select pipe - ", Name());
   appendLastOsError();
}

ThreadError_UnableToReadPipe::ThreadError_UnableToReadPipe()
{
   setSevere();
   setTextf("%s: Error while reading select pipe - ", Name());
   appendLastOsError();
}

ThreadError_UnableToWritePipe::ThreadError_UnableToWritePipe()
{
   setSevere();
   setTextf("%s: Error while writing select pipe - ", Name());
   appendLastOsError();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


/// @endcond

#if 0
////////////////////////////////////////////////////////////////////////////////
// ESocket::Base
////////////////////////////////////////////////////////////////////////////////

/// @cond DOXYGEN_EXCLUDE

Base::Base(Thread &thread, SocketType socktype, Int family, Int type, Int protocol)
   : m_thread( thread ),
     m_socktype( socktype ),
     m_family( family ),
     m_type( type ),
     m_protocol( protocol ),
     m_error( 0 ),
     m_handle( EPC_INVALID_SOCKET )
{
}

Base::~Base()
{
   close();
}

Void Base::onReceive()
{
}

Void Base::onConnect()
{
}

Void Base::onClose()
{
}

Void Base::onError()
{
}


Int Base::setError()
{
   m_error = EPC_LASTERROR;
   return m_error;
}

Void Base::createSocket(Int family, Int type, Int protocol)
{
   m_handle = socket(family, type, protocol);
   if (m_handle == EPC_INVALID_SOCKET)
      throw BaseError_UnableToCreateSocket();

   m_family = family;
   m_type = type;
   m_protocol = protocol;

   setOptions();
}

Void Base::disconnect()
{
   getThread().unregisterSocket(this);
   if (m_handle != EPC_INVALID_SOCKET)
   {
      epc_closesocket(m_handle);
      m_handle = EPC_INVALID_SOCKET;
   }
}

Void Base::close()
{
   disconnect();
   onClose();
}

Void Base::setHandle(Int handle)
{
   disconnect();
   m_handle = handle;
   setOptions();
}

Void Base::setOptions()
{
   struct linger l;
   l.l_onoff = 1;
   l.l_linger = 0;
   setsockopt(m_handle, SOL_SOCKET, SO_LINGER, (PSOCKETOPT)&l, sizeof(l));

   fcntl(m_handle, F_SETFL, O_NONBLOCK);

   getThread().registerSocket(this);
}

Address &Base::setLocalAddress(Address &addr)
{
   addr.clear();

   socklen_t sockaddrlen = addr.getSockAddrLen();;

   if (getsockname(m_handle, addr.getSockAddr(), &sockaddrlen) < 0)
      throw BaseError_GetPeerNameError();

   return addr;
}

Address &Base::setRemoteAddress(Address &addr)
{
   addr.clear();

   socklen_t sockaddrlen = addr.getSockAddrLen();;

   if (getpeername(m_handle, addr.getSockAddr(), &sockaddrlen) < 0)
      throw BaseError_GetPeerNameError();

   return addr;
}

/// @endcond

////////////////////////////////////////////////////////////////////////////////
// ESocket::TCP::Talker
////////////////////////////////////////////////////////////////////////////////
namespace TCP
{
   Talker::Talker(Thread &thread, Int bufsize)
      : Base(thread, SocketType::TcpTalker, AF_INET6, SOCK_STREAM, IPPROTO_TCP),
      m_state( State::Undefined ),
      m_sending(False),
      m_rbuf(bufsize),
      m_wbuf(bufsize)
   {
   }

   Talker::~Talker()
   {
   }

   Void Talker::connect()
   {
      if (getRemote().getFamily() != Address::Family::INET && getRemote().getFamily() != Address::Family::INET6)
         throw TcpTalkerError_InvalidRemoteAddress();

      struct addrinfo *pAddress;

      Int family = getRemote().getFamily() == Address::Family::INET ? AF_INET : AF_INET6;
      Int type = getType();
      Int protocol = getProtocol();

      createSocket( family, type, protocol );

      int result = ::connect(getHandle(), getRemote().getSockAddr(), getRemote().getSockAddrLen());

      if (result == 0)
      {
         setState( State::Connected );
         onConnect();
      }
      else if (result == -1)
      {
         setError();
         if (getError() != EPC_EINPROGRESS && getError() != EPC_EWOULDBLOCK)
            throw TcpTalkerError_UnableToConnect();

         setState( State::Connecting );

         getThread().bump();
      }
   }

   /// @cond DOXYGEN_EXCLUDE
   Int Talker::recv()
   {
      //
      // modified this routine to use a buffer allocated from the stack
      // instead of a single buffer allocated from the heap (which had
      // been used for both reading and writing) to avoid between the
      // read and write process
      //
      UChar buf[2048];
      Int totalReceived = 0;

      while (True)
      {
         Int amtReceived = ::recv(getHandle(), (PSNDRCVBUFFER)buf, sizeof(buf), 0);
         if (amtReceived > 0)
         {
            m_rbuf.writeData(buf, 0, amtReceived);
            totalReceived += amtReceived;
         }
         else if (amtReceived == 0)
         {
            setState( State::Disconnected );
            break;
         }
         else
         {
            setError();
            if (getError() == EPC_EWOULDBLOCK)
               break;
            throw TcpTalkerError_UnableToRecvData();
         }
      }

      return totalReceived;
   }
   /// @endcond

   Int Talker::send(pUChar pData, Int length)
   {
      Int result = ::send(getHandle(), (PSNDRCVBUFFER)pData, length, MSG_NOSIGNAL);

      if (result == -1)
      {
         setError();
         if (getError() != EPC_EWOULDBLOCK)
            throw TcpTalkerError_SendingPacket();
      }

      return result;
   }

   Void Talker::send(Bool override)
   {
      UChar buf[2048];

      EMutexLock lck(m_sendmtx, False);
      if (!lck.acquire(False))
         return;

      if (!override && m_sending)
         return;

      if (m_wbuf.isEmpty())
      {
         m_sending = false;
         return;
      }

      if (getState() != State::Connected)
      {
         std::raise(SIGINT);
         throw TcpTalkerError_InvalidSendState(getStateDescription(getState()));
      }

      m_sending = true;
      while (true)
      {
         if (m_wbuf.isEmpty())
         {
            m_sending = false;
            break;
         }

         Int packetLength = 0;
         Int amtRead = m_wbuf.peekData((pUChar)&packetLength, 0, sizeof(packetLength));
         if (amtRead != sizeof(packetLength))
         {
            EString msg;
            msg.format("expected %d bytes, read %d bytes", sizeof(packetLength), amtRead);
            throw TcpTalkerError_ReadingWritePacketLength(msg.c_str());
         }

         Int sentLength = 0;
         while (sentLength < packetLength)
         {
            Int sendLength = packetLength - sentLength;
            if (sendLength > (Int)sizeof(buf))
               sendLength = sizeof(buf);

            // get data from the circular buffer
            amtRead = m_wbuf.peekData((pUChar)buf, sizeof(packetLength) + sentLength, sendLength);
            if (amtRead != sendLength)
            {
               EString msg;
               msg.format("expected %d bytes, read %d bytes", sendLength, amtRead);
               throw TcpTalkerError_ReadingWritePacketLength(msg.c_str());
            }

            // write the data to the socket
            Int amtWritten = send(buf, sendLength);
            if (amtWritten == -1) // EPC_EWOULDBLOCK
               break;

            sentLength += amtWritten;
            if (amtWritten != sendLength) // only part of the data was written
               break;
         }

         packetLength -= sentLength;
         m_wbuf.readData(NULL, 0, sentLength + (!packetLength ? sizeof(packetLength) : 0));
         if (packetLength > 0)
         {
            // need to update the buffer indicating the amount of the
            // message remaining in the circular buffer
            //fprintf(stderr,"wrote %d bytes of %d\n", sentLength, packetLength + sentLength);
            m_wbuf.modifyData((pUChar)&packetLength, 0, (Int)sizeof(packetLength));
            break;
         }
      }
   }

   Int Talker::peek(pUChar dest, Int len)
   {
      return m_rbuf.peekData(dest, 0, len);
   }

   Int Talker::read(pUChar dest, Int len)
   {
      return m_rbuf.readData(dest, 0, len);
   }

   Void Talker::write(pUChar src, Int len)
   {
      {
         EMutexLock l(m_wbuf.getMutex());
         m_wbuf.writeData((pUChar)&len, 0, sizeof(len), True);
         m_wbuf.writeData(src, 0, len, True);
      }

      send();
   }

   Void Talker::disconnect()
   {
      Base::disconnect();
      m_state = State::Disconnected;
      m_remote.clear();
   }

   Void Talker::onReceive()
   {
   }

   Void Talker::onConnect()
   {
   }

   Void Talker::onClose()
   {
      close();
   }

   Void Talker::onError()
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   // ESocket::TCP::Listener
   ////////////////////////////////////////////////////////////////////////////////
   Listener::Listener(Thread &thread, Address::Family family)
      : Base(thread, SocketType::TcpListener, 
         family == Address::Family::INET ? AF_INET : AF_INET6,
         SOCK_STREAM, IPPROTO_TCP),
        m_state( State::Undefined ),
        m_backlog( -1 )
   {
   }

   Listener::Listener(Thread &thread, UShort port, Address::Family family)
      : Base(thread, SocketType::TcpListener,
         family == Address::Family::INET ? AF_INET : AF_INET6,
         SOCK_STREAM, IPPROTO_TCP),
        m_state( State::Undefined ),
        m_backlog( -1 )
   {
      setPort( port );
   }

   Listener::Listener(Thread &thread, UShort port, Int backlog, Address::Family family)
      : Base(thread, SocketType::TcpListener,
         family == Address::Family::INET ? AF_INET : AF_INET6,
         SOCK_STREAM, IPPROTO_TCP),
        m_state( State::Undefined ),
        m_backlog( backlog )
   {
      setPort( port );
   }

   Void Listener::listen()
   {
      bind();
      if (::listen(getHandle(), getBacklog()) == EPC_SOCKET_ERROR)
         throw TcpListenerError_UnableToListen();
      setState( State::Listening );
   }

   Void Listener::onClose()
   {
      Base::onClose();
      setState( State::Undefined );
   }

   Void Listener::onError()
   {
   }

   Void Listener::bind()
   {
      Base::createSocket(getFamily(), getType(), getProtocol());

      int result = ::bind(getHandle(), getLocalAddress().getSockAddr(), getLocalAddress().getSockAddrLen());
      if (result == -1)
      {
         TcpListenerError_UnableToBindSocket err;
         close();
         throw err;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// ESocket::UDP
////////////////////////////////////////////////////////////////////////////////

// (Maximum message length) = (max IP packet size) - (min IP header length) - (udp header length)
// 65507 = 65535 - 20 - 8
#define UPD_MAX_MSG_LENGTH 65507

UDP::UDP(Thread &thread, Int bufsize)
   : Base(thread, SocketType::Udp, AF_INET6, SOCK_DGRAM, IPPROTO_UDP),
     m_sending(False),
     m_rbuf(bufsize),
     m_wbuf(bufsize),
     m_rcvmsg(NULL),
     m_sndmsg(NULL)
{
   m_rcvmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
   m_sndmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
}

UDP::UDP(Thread &thread, UShort port, Int bufsize)
   : Base(thread, SocketType::Udp, AF_INET6, SOCK_DGRAM, IPPROTO_UDP),
     m_sending(False),
     m_rbuf(bufsize),
     m_wbuf(bufsize),
     m_rcvmsg(NULL),
     m_sndmsg(NULL)
{
   m_local = port;
   setFamily( m_local.getFamily() == Address::Family::INET ? AF_INET : AF_INET6 );
   bind();
   m_rcvmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
   m_sndmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
}

UDP::UDP(Thread &thread, cpStr ipaddr, UShort port, Int bufsize)
   : Base(thread, SocketType::Udp, AF_INET6, SOCK_DGRAM, IPPROTO_UDP),
     m_sending(False),
     m_rbuf(bufsize),
     m_wbuf(bufsize),
     m_rcvmsg(NULL),
     m_sndmsg(NULL)
{
   m_local.setAddress( ipaddr, port );
   setFamily( m_local.getFamily() == Address::Family::INET ? AF_INET : AF_INET6 );
   bind();
   m_rcvmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
   m_sndmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
}

UDP::UDP(Thread &thread, Address &addr, Int bufsize)
   : Base(thread, SocketType::Udp, AF_INET6, SOCK_DGRAM, IPPROTO_UDP),
     m_sending(False),
     m_rbuf(bufsize),
     m_wbuf(bufsize),
     m_rcvmsg(NULL),
     m_sndmsg(NULL)
{
   m_local = addr;
   setFamily( m_local.getFamily() == Address::Family::INET ? AF_INET : AF_INET6 );
   bind();
   m_rcvmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
   m_sndmsg = reinterpret_cast<UDPMessage*>(new UChar[sizeof(UDPMessage) + UPD_MAX_MSG_LENGTH]);
}

UDP::~UDP()
{
   if (m_rcvmsg)
      delete [] reinterpret_cast<pUChar>(m_rcvmsg);
   if (m_sndmsg)
      delete [] reinterpret_cast<pUChar>(m_sndmsg);
}

Void UDP::bind(UShort port)
{
   if (getHandle() != EPC_INVALID_SOCKET)
      throw UdpError_AlreadyBound();
   m_local = port;
   setFamily( m_local.getFamily() == Address::Family::INET ? AF_INET : AF_INET6 );
   bind();
}

Void UDP::bind(cpStr ipaddr, UShort port)
{
   if (getHandle() != EPC_INVALID_SOCKET)
      throw UdpError_AlreadyBound();
   m_local.setAddress( ipaddr, port );
   setFamily( m_local.getFamily() == Address::Family::INET ? AF_INET : AF_INET6 );
   bind();
}

Void UDP::bind(const Address &addr)
{
   if (getHandle() != EPC_INVALID_SOCKET)
      throw UdpError_AlreadyBound();
   m_local = addr;
   setFamily( m_local.getFamily() == Address::Family::INET ? AF_INET : AF_INET6 );
   bind();
}

Void UDP::bind()
{
   if (getHandle() != EPC_INVALID_SOCKET)
      throw UdpError_AlreadyBound();

   Base::createSocket(getFamily(), getType(), getProtocol());

   int result = ::bind(getHandle(), getLocal().getSockAddr(), getLocal().getSockAddrLen());
   if (result == -1)
   {
      UdpError_UnableToBindSocket err;
      close();
      throw err;
   }
}

/// @cond DOXYGEN_EXCLUDE
Int UDP::recv()
{
   Int totalReceived = 0;
   Address addr;
   socklen_t addrlen;
   Int flags = 0;

   while (True)
   {
      addrlen = addr.getSockAddrLen();
      Int amtReceived = ::recvfrom(getHandle(), m_rcvmsg->data, UPD_MAX_MSG_LENGTH, flags, addr.getSockAddr(), &addrlen);
      if (amtReceived >= 0)
      {
         m_rcvmsg->total_length = sizeof(UDPMessage) + amtReceived;
         m_rcvmsg->data_length = amtReceived;
         m_rcvmsg->addr = addr;

         m_rbuf.writeData( reinterpret_cast<pUChar>(m_rcvmsg), 0, m_rcvmsg->total_length);
         totalReceived += amtReceived;
      }
      else
      {
         setError();
         if (getError() == EPC_EWOULDBLOCK)
            break;
         throw UdpError_UnableToRecvData();
      }
   }

   return totalReceived;
}
/// @endcond

Int UDP::send(Address &addr, cpVoid pData, Int length)
{
   Int flags = MSG_NOSIGNAL;
   Int result = ::sendto(getHandle(), pData, length, flags, addr.getSockAddr(), addr.getSockAddrLen());

   if (result == -1)
   {
      setError();
      if (getError() != EPC_EMSGSIZE)
         throw UdpError_SendingPacket();
   }

   return result;
}

#include <csignal>

Void UDP::send(Bool override)
{
   EMutexLock lck(m_sendmtx, False);
   if (!lck.acquire(False))
      return;

   if (!override && m_sending)
      return;

   if (m_wbuf.isEmpty())
   {
      m_sending = false;
      return;
   }

   m_sending = true;
   while (true)
   {
      if (m_wbuf.isEmpty())
      {
         m_sending = false;
         break;
      }

      size_t packetLength = 0;
      Int amtRead = m_wbuf.peekData(reinterpret_cast<pUChar>(&packetLength), 0, sizeof(packetLength));
      if (amtRead != sizeof(packetLength))
      {
         EString msg;
         msg.format("expected %d bytes, read %d bytes", sizeof(packetLength), amtRead);
         throw UdpError_ReadingWritePacketLength(msg.c_str());
      }

      amtRead = m_wbuf.peekData(reinterpret_cast<pUChar>(m_sndmsg), 0, packetLength);
      if (amtRead != packetLength)
      {
         EString msg;
         msg.format("expected %d bytes, read %d bytes", sizeof(packetLength), amtRead);
         throw UdpError_ReadingWritePacketLength(msg.c_str());
      }

      if (send(m_sndmsg->addr, m_sndmsg->data, m_sndmsg->data_length) == -1)
      {
         // unable to send this message so get out, it will be sent when the socket is ready for writing
         break;
      }

      m_wbuf.readData(NULL, 0, m_sndmsg->total_length);
   }
}

Bool UDP::readMessage(UDPMessage &msg)
{
   if (m_rbuf.peekData(reinterpret_cast<pUChar>(&msg), 0, sizeof(msg)))
   {
      m_rbuf.readData(reinterpret_cast<pUChar>(&msg), 0, msg.total_length);
      return True;
   }

   return False;
}

Void UDP::write(const Address &addr, pVoid src, Int len)
{
   UDPMessage msg;
   msg.total_length = sizeof(msg) + len;
   msg.data_length = len;
   msg.addr = addr;

   {
      EMutexLock l(m_wbuf.getMutex());
      m_wbuf.writeData(reinterpret_cast<pUChar>(&msg), 0, sizeof(msg), True);
      m_wbuf.writeData(reinterpret_cast<pUChar>(src), 0, len, True);
   }

   send();
}

Void UDP::disconnect()
{
   Base::disconnect();
   m_local.clear();
}

Void UDP::onReceive(const Address &from, pVoid msg, Int len)
{
}

Void UDP::onError()
{
}

Void UDP::onReceive()
{
   while (readMessage(*m_rcvmsg))
   {
      onReceive(m_rcvmsg->addr, reinterpret_cast<pVoid>(m_rcvmsg->data), m_rcvmsg->data_length);
   }
}

////////////////////////////////////////////////////////////////////////////////
// ESocket::Thread
////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(Thread, EThreadPrivate)
END_MESSAGE_MAP()

Thread::Thread()
{
   m_error = 0;

   int result = pipe(m_pipefd);
   if (result == -1)
      throw ThreadError_UnableToOpenPipe();
   fcntl(m_pipefd[0], F_SETFL, O_NONBLOCK);

   FD_ZERO(&m_master);
}

Thread::~Thread()
{
}

/// @cond DOXYGEN_EXCLUDE
Void Thread::onInit()
{
   EThreadPrivate::onInit();
}

Void Thread::onQuit()
{
   EThreadPrivate::onQuit();
}
/// @endcond

Void Thread::registerSocket(Base *psocket)
{
   m_socketmap.insert(std::make_pair(psocket->getHandle(), psocket));
   FD_SET(psocket->getHandle(), &m_master);
   bump();
}

Void Thread::unregisterSocket(Base *psocket)
{
   if (m_socketmap.erase(psocket->getHandle()))
   {
      FD_CLR(psocket->getHandle(), &m_master);
      bump();
   }
}

Void Thread::processSelectConnect(Base *psocket)
{
   if (psocket->getSocketType() == Base::SocketType::TcpTalker)
      ((TCP::Talker *)psocket)->onConnect();
}

Void Thread::processSelectClose(Base *psocket)
{
   psocket->onClose();
}

Void Thread::processSelectAccept(Base *psocket)
{
   if (psocket->getSocketType() == Base::SocketType::TcpListener)
   {
      bool more = true;
      while (more)
      {
         try
         {
            struct sockaddr ipaddr;
            socklen_t ipaddrlen = sizeof(ipaddr);

            EPC_SOCKET handle = ::accept((static_cast<TCP::Listener*>(psocket))->getHandle(), &ipaddr, &ipaddrlen);
            if (handle == EPC_INVALID_SOCKET)
            {
               Int err = EPC_LASTERROR;
               if (err == EPC_EWOULDBLOCK)
                  break;
               throw TcpListenerError_UnableToAcceptSocket();
            }

            TCP::Talker *pnewsocket = (static_cast<TCP::Listener*>(psocket))->createSocket(*this);
            if (pnewsocket)
            {
               pnewsocket->setHandle(handle);
               pnewsocket->setAddresses();
               pnewsocket->setState( TCP::Talker::State::Connected );
               registerSocket(pnewsocket);
               pnewsocket->onConnect();
            }
            else
            {
               // the connection is being refused, so close the handle
               epc_closesocket(handle);
            }
         }
         catch (EError &err)
         {
            if (err.getLastOsError() != EPC_EWOULDBLOCK)
            {
               //printf("errorHandler() 1 %d\n", err->getLastOsError());
               errorHandler(err, NULL);
            }
            more = false;
         }
      }
   }
}

Void Thread::processSelectRead(Base *psocket)
{
   if (psocket->getSocketType() == Base::SocketType::TcpListener)
   {
      processSelectAccept(psocket);
   }
   else if (psocket->getSocketType() == Base::SocketType::TcpTalker)
   {
      if ((static_cast<TCP::Talker*>(psocket))->getState() == TCP::Talker::State::Connecting)
      {
         (static_cast<TCP::Talker*>(psocket))->setState( TCP::Talker::State::Connected );
         (static_cast<TCP::Talker*>(psocket))->setAddresses();
         (static_cast<TCP::Talker*>(psocket))->onConnect();
      }

      while (true)
      {
         try
         {
            Int amtRead = (static_cast<TCP::Talker*>(psocket))->recv();
            if (amtRead <= 0)
               break;
         }
         catch (EError &err)
         {
            //printf("errorHandler() 2\n");
            errorHandler(err, psocket);
         }
      }

      ((TCP::Talker *)psocket)->onReceive();

      if ((static_cast<TCP::Talker*>(psocket))->getState() == TCP::Talker::State::Disconnected)
         processSelectClose(psocket);
   }
   else if (psocket->getSocketType() == Base::SocketType::Udp)
   {
      while (true)
      {
         try
         {
            Int amtRead = (static_cast<UDP*>(psocket))->recv();
            if (amtRead <= 0)
               break;
         }
         catch (EError &err)
         {
            //printf("errorHandler() 2\n");
            errorHandler(err, psocket);
         }
      }

      (reinterpret_cast<UDP*>(psocket))->onReceive();
   }
}

Void Thread::processSelectWrite(Base *psocket)
{
   if (psocket->getSocketType() == Base::SocketType::TcpTalker)
   {
      if ((static_cast<TCP::Talker*>(psocket))->getState() == TCP::Talker::State::Connecting)
      {
         (static_cast<TCP::Talker*>(psocket))->setState(TCP::Talker::State::Connected);
         (static_cast<TCP::Talker*>(psocket))->setAddresses();
         (static_cast<TCP::Talker*>(psocket))->onConnect();
      }
      else
      {
         try
         {
            (static_cast<TCP::Talker*>(psocket))->send(True);
         }
         catch (EError &err)
         {
            //printf("errorHandler() 3\n");
            errorHandler(err, psocket);
         }
      }
   }
   else if (psocket->getSocketType() == Base::SocketType::Udp)
   {
      try
      {
         (static_cast<UDP*>(psocket))->send(True);
      }
      catch (EError &err)
      {
         //printf("errorHandler() 3\n");
         errorHandler(err, psocket);
      }
   }
}

Void Thread::processSelectError(Base *psocket)
{
   psocket->onError();
}

/// @cond DOXYGEN_EXCLUDE
Void Thread::messageQueued()
{
   EThreadPrivate::messageQueued();
   bump();
}

Void Thread::onError()
{
}
/// @endcond

int Thread::getMaxFileDescriptor()
{
   if (m_socketmap.size() == 0)
      return m_pipefd[0];

   int maxfd = m_socketmap.begin()->first;

   return (maxfd > m_pipefd[0]) ? maxfd : m_pipefd[0];
}

/// @cond DOXYGEN_EXCLUDE
Void Thread::bump()
{
   if (write(m_pipefd[1], "~", 1) == -1)
      throw ThreadError_UnableToWritePipe();
}

Void Thread::clearBump()
{
   char buf[1];
   while (true)
   {
      if (read(m_pipefd[0], buf, 1) == -1)
      {
         if (errno == EWOULDBLOCK)
            break;
         throw ThreadError_UnableToReadPipe();
      }
   }
}
/// @endcond

Bool Thread::pumpMessagesInternal()
{
   EThreadMessage msg;

   try
   {
      while (True)
      {
         if (!pumpMessage(msg, false) || msg.getMessageId() == EM_QUIT)
            break;
      }
   }
   catch (...)
   {
      throw;
   }

   ////////////////////////////////////////////////////////////////////
   // get out if the thread has been told to stop
   ////////////////////////////////////////////////////////////////////
   //return (keepGoing() && msg.getMsgId() != EM_QUIT);
   return msg.getMessageId() != EM_QUIT;
}

/// @cond DOXYGEN_EXCLUDE
Void Thread::pumpMessages()
{
   int maxfd, fd, fdcnt;
   BasePtrMap::const_iterator socket_it;
   fd_set readworking, writeworking, errorworking;
   while (true)
   {
      {
         memcpy(&readworking, &m_master, sizeof(m_master));
         FD_SET(m_pipefd[0], &readworking);

         FD_ZERO(&writeworking);
         for (BasePtrMap::const_iterator it = m_socketmap.begin(); it != m_socketmap.end(); it++)
         {
            Base *pSocket = it->second;
            if ((pSocket->getSocketType() == Base::SocketType::TcpTalker &&
                  ((static_cast<TCP::Talker*>(pSocket))->getSending() ||
                   (static_cast<TCP::Talker*>(pSocket))->getState() == TCP::Talker::State::Connecting)) ||
                (pSocket->getSocketType() == Base::SocketType::Udp && (static_cast<UDP*>(pSocket))->getSending()))
            {
               FD_SET(it->first, &writeworking);
            }
         }

         memcpy(&errorworking, &m_master, sizeof(m_master));

         maxfd = getMaxFileDescriptor() + 1;
      }

      fdcnt = select(maxfd, &readworking, &writeworking, &errorworking, NULL);
      if (fdcnt == -1)
      {
         if (errno == EINTR || errno == 514 /*ERESTARTNOHAND*/)
         {
            if (!pumpMessagesInternal())
               break;
         }
         else
         {
            onError();
         }
         continue;
      }

      ////////////////////////////////////////////////////////////////////////
      // Process any thread messages
      ////////////////////////////////////////////////////////////////////////
      if (FD_ISSET(m_pipefd[0], &readworking))
      {
         --fdcnt;
         if (!pumpMessagesInternal())
            break;
      }

      ////////////////////////////////////////////////////////////////////////
      // Process any socket messages
      ////////////////////////////////////////////////////////////////////////
      for (fd = 0; fd < maxfd && fdcnt > 0; fd++)
      {
         if (FD_ISSET(fd, &errorworking))
         {
            socket_it = m_socketmap.find(fd);
            if (socket_it != m_socketmap.end())
            {
               Base *pSocket = socket_it->second;
               if (pSocket)
               {
                  int error;
                  socklen_t optlen = sizeof(error);
                  getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &optlen);
                  pSocket->setError(error);
                  processSelectError(pSocket);
               }
            }
            fdcnt--;
         }

         if (fdcnt > 0 && FD_ISSET(fd, &readworking))
         {
            socket_it = m_socketmap.find(fd);
            if (socket_it != m_socketmap.end())
            {
               Base *pSocket = socket_it->second;
               if (pSocket)
                  processSelectRead(pSocket);
            }
            fdcnt--;
         }

         if (fdcnt > 0 && FD_ISSET(fd, &writeworking))
         {
            socket_it = m_socketmap.find(fd);
            if (socket_it != m_socketmap.end())
            {
               Base *pSocket = socket_it->second;
               if (pSocket)
                  processSelectWrite(pSocket);
            }
            fdcnt--;
         }
      }

      ////////////////////////////////////////////////////////////////////////
      // Process any thread messages that may have been posted while
      //   processing the socket events
      ////////////////////////////////////////////////////////////////////////
      if (!pumpMessagesInternal())
         break;

      clearBump();
   }

   while (true)
   {
      BasePtrMap::iterator it = m_socketmap.begin();
      if (it == m_socketmap.end())
         break;
      Base *psocket = it->second;
      m_socketmap.erase(it);
      delete psocket;
   }
}
#endif
/// @endcond
}