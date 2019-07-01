/*
* Copyright (c) 2009-2019 Brian Waters
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

#include "esocket.h"

#define epc_closesocket(a) ::close(a)
#define EPC_LASTERROR errno
#define EPC_INVALID_SOCKET -1
#define EPC_EINPROGRESS EINPROGRESS
#define EPC_EWOULDBLOCK EWOULDBLOCK
#define EPC_SOCKET_ERROR -1
typedef Int EPC_SOCKET;
typedef void *PSOCKETOPT;
typedef void *PSNDRCVBUFFER;
typedef socklen_t EPC_SOCKLEN;

////////////////////////////////////////////////////////////////////////////////
// ESocketError
////////////////////////////////////////////////////////////////////////////////
ESocketError_UnableToCreateSocket::ESocketError_UnableToCreateSocket()
{
   setSevere();
   setTextf("%s: Error creating socket - ", Name());
   appendLastOsError();
}

ESocketError_UnableToBindSocket::ESocketError_UnableToBindSocket()
{
   setSevere();
   setTextf("%s: Error binding socket in bind() - ", Name());
   appendLastOsError();
}

ESocketError_UnableToAcceptSocket::ESocketError_UnableToAcceptSocket()
{
   setSevere();
   setTextf("%s: Error accepting new connection in accept() - ", Name());
   appendLastOsError();
}

ESocketError_GetAddressInfo::ESocketError_GetAddressInfo(cpStr msg)
{
   setSevere();
   setTextf("%s: Error getting address info in getaddrinfo() - %s", Name(), msg);
}

ESocketError_NoAddressesFound::ESocketError_NoAddressesFound(cpStr msg)
{
   setSevere();
   setTextf("%s: Error no addresses found in getaddrinfo() for %s - ", Name(), msg);
   appendLastOsError();
}

ESocketListenError_UnableToListen::ESocketListenError_UnableToListen()
{
   setSevere();
   setTextf("%s: Error executing listen - ", Name());
   appendLastOsError();
}

ESocketConverseError_UnableToConnect::ESocketConverseError_UnableToConnect()
{
   setSevere();
   setTextf("%s: Unable to connect - ", Name());
   appendLastOsError();
}

ESocketConverseError_OutOfMemory::ESocketConverseError_OutOfMemory(cpStr msg)
{
   setSevere();
   setTextf("%s: Out of memory in %s", Name(), msg);
}

ESocketConverseError_UnableToRecvData::ESocketConverseError_UnableToRecvData()
{
   setSevere();
   setTextf("%s: Error while executing recv() - ", Name());
   appendLastOsError();
}

ESocketConverseError_InvalidSendState::ESocketConverseError_InvalidSendState(cpStr msg)
{
   setSevere();
   setTextf("%s: Invalid state while sending: %s", Name(), msg);
}

ESocketConverseError_ReadingWritePacketLength::ESocketConverseError_ReadingWritePacketLength(cpStr msg)
{
   setSevere();
   setTextf("%s: Unable to read the write packet length - %s", Name(), msg);
}

ESocketConverseError_SendingPacket::ESocketConverseError_SendingPacket()
{
   setSevere();
   setTextf("%s: Error while attempting to send a packet of data via send() - ", Name());
   appendLastOsError();
}

ESocketConverseError_UnrecognizedSendReturnValue::ESocketConverseError_UnrecognizedSendReturnValue(cpStr msg)
{
   setSevere();
   setTextf("%s: Unrecoginzed return value - %s", Name(), msg);
}

ESocketThreadError_UnableToOpenPipe::ESocketThreadError_UnableToOpenPipe()
{
   setSevere();
   setTextf("%s: Error while opening select pipe - ", Name());
   appendLastOsError();
}

ESocketThreadError_UnableToReadPipe::ESocketThreadError_UnableToReadPipe()
{
   setSevere();
   setTextf("%s: Error while reading select pipe - ", Name());
   appendLastOsError();
}

ESocketThreadError_UnableToWritePipe::ESocketThreadError_UnableToWritePipe()
{
   setSevere();
   setTextf("%s: Error while writing select pipe - ", Name());
   appendLastOsError();
}

////////////////////////////////////////////////////////////////////////////////
// ESocket
////////////////////////////////////////////////////////////////////////////////
ESocket::ESocket(ESocketThread *pthread, SOCKETSTYLE style, Int family, Int type, Int protocol)
{
   m_thread = pthread;

   m_style = style;
   m_family = family;
   m_type = type;
   m_protocol = protocol;

   m_handle = EPC_INVALID_SOCKET;
   m_port = -1;

   m_state = DISCONNECTED;
}

ESocket::~ESocket()
{
   close();
}

Void ESocket::onClose()
{
}

Void ESocket::onError()
{
}

Int ESocket::setError()
{
   m_error = errno;
   return m_error;
}

Void ESocket::createSocket(Int family, Int type, Int protocol)
{
   m_handle = socket(getFamily(), getType(), getProtocol());
   if (m_handle == EPC_INVALID_SOCKET)
      throw ESocketError_UnableToCreateSocket();

   setOptions();
}

Void ESocket::bind()
{
   bind(getPort());
}

Void ESocket::bind(UShort port)
{
   struct addrinfo *pai = NULL;

   assignAddress(NULL, port, getFamily(), getType(),
                 AI_PASSIVE, getProtocol(), &pai);

   createSocket(getFamily(), getType(), getProtocol());

   int result = ::bind(m_handle, pai->ai_addr, (EPC_SOCKLEN)pai->ai_addrlen);

   freeaddrinfo(pai);

   if (result == -1)
   {
      ESocketError_UnableToBindSocket *perr = new ESocketError_UnableToBindSocket();
      close();
      throw perr;
   }
}

Void ESocket::disconnect()
{
   getThread()->unregisterSocket(this);
   if (m_handle != EPC_INVALID_SOCKET)
   {
      epc_closesocket(m_handle);
      m_handle = EPC_INVALID_SOCKET;
      m_state = ESocket::DISCONNECTED;
      m_ipaddr.clear();
   }
}

Void ESocket::close()
{
   disconnect();
   onClose();
}

Void ESocket::setHandle(Int handle)
{
   disconnect();
   m_handle = handle;
   setOptions();
}

Void ESocket::setOptions()
{
   struct linger l;
   l.l_onoff = 1;
   l.l_linger = 0;
   setsockopt(m_handle, SOL_SOCKET, SO_LINGER, (PSOCKETOPT)&l, sizeof(l));

   fcntl(m_handle, F_SETFL, O_NONBLOCK);

   getThread()->registerSocket(this);
}

Void ESocket::setIpAddress(cpStr addr)
{
   if (addr)
      m_ipaddr = addr;
   else
      m_ipaddr.clear();
}

cpStr ESocket::getIpAddress()
{
   return m_ipaddr.c_str();
}

Void ESocket::assignAddress(cpStr ipaddr, UShort port, Int family, Int socktype,
                             Int flags, Int protocol, struct addrinfo **paddrs)
{
   Int result;
   EString sPort;
   struct addrinfo hints;

   sPort.format("%u", port);

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;       //family;
   hints.ai_socktype = SOCK_STREAM; //socktype;
   hints.ai_flags = AI_PASSIVE;     //flags;
   hints.ai_protocol = protocol;

   result = getaddrinfo(ipaddr, sPort.c_str(), &hints, paddrs);
   if (result)
   {
      cpStr errStr;

      switch (result)
      {
      case EAI_ADDRFAMILY:
         errStr = "EAI_ADDRFAMILY";
         break;
      case EAI_AGAIN:
         errStr = "EAI_AGAIN";
         break;
      case EAI_BADFLAGS:
         errStr = "EAI_BADFLAGS";
         break;
      case EAI_FAIL:
         errStr = "EAI_FAIL";
         break;
      case EAI_FAMILY:
         errStr = "EAI_FAMILY";
         break;
      case EAI_MEMORY:
         errStr = "EAI_MEMORY";
         break;
      case EAI_NODATA:
         errStr = "EAI_NODATA";
         break;
      case EAI_NONAME:
         errStr = "EAI_NONAME";
         break;
      case EAI_SERVICE:
         errStr = "EAI_SERVICE";
         break;
      case EAI_SOCKTYPE:
         errStr = "EAI_SOCKTYPE";
         break;
      case EAI_SYSTEM:
         errStr = "EAI_SYSTEM";
         break;
      default:
         errStr = "UNKNOWN";
         break;
      }

      throw ESocketError_GetAddressInfo(errStr);
   }

   if (!*paddrs)
   {
      EString msg;
      msg.format("%s:%u", ipaddr ? ipaddr : "", port);
      throw ESocketError_NoAddressesFound(msg.c_str());
   }
}

cpStr ESocket::getStateDescription(ESocket::SOCKETSTATE state)
{
   cpStr pState = "Unknown";

   switch (state)
   {
   case ESocket::CONNECTING:
   {
      pState = "CONNECTING";
      break;
   }
   case ESocket::CONNECTED:
   {
      pState = "CONNECTED";
      break;
   }
   case ESocket::DISCONNECTED:
   {
      pState = "DISCONNECTED";
      break;
   }
   }

   return pState;
}

////////////////////////////////////////////////////////////////////////////////
// ESocketConverse
////////////////////////////////////////////////////////////////////////////////
ESocketConverse::ESocketConverse(ESocketThread *pthread, Int bufSize, Int family, Int type, Int protocol)
    : ESocket(pthread, ESocket::CONVERSE, family, type, protocol),
      m_rbuf(bufSize),
      m_wbuf(bufSize)
{
   m_sending = false;
   m_remoteport = 0;
}

ESocketConverse::~ESocketConverse()
{
}

Void ESocketConverse::connect()
{
   struct addrinfo *pAddress;

   assignAddress(getIpAddress(), getPort(), getFamily(),
                 getType(), 0, getProtocol(), &pAddress);

   createSocket(getFamily(), getType(), getProtocol());

   int result = ::connect(getHandle(), pAddress->ai_addr, (EPC_SOCKLEN)pAddress->ai_addrlen);

   if (result == 0)
   {
      setState(CONNECTED);
      onConnect();
   }
   else if (result == -1)
   {
      setError();
      if (getError() != EPC_EINPROGRESS && getError() != EPC_EWOULDBLOCK)
      {
         ESocketConverseError_UnableToConnect *err = new ESocketConverseError_UnableToConnect();
         freeaddrinfo(pAddress);
         throw err;
      }

      setState(CONNECTING);

      getThread()->bump();
   }

   freeaddrinfo(pAddress);
}

Void ESocketConverse::connect(cpStr ipaddr, UShort port)
{
   setIpAddress(ipaddr);
   setPort(port);
   connect();
}

Int ESocketConverse::recv()
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
         setState(DISCONNECTED);
         break;
      }
      else
      {
         setError();
         if (getError() == EPC_EWOULDBLOCK)
            break;
         throw ESocketConverseError_UnableToRecvData();
      }
   }

   return totalReceived;
}

Int ESocketConverse::send(pUChar pData, Int length)
{
   Int result = ::send(getHandle(), (PSNDRCVBUFFER)pData, length, MSG_NOSIGNAL);

   if (result == -1)
   {
      setError();
      if (getError() != EPC_EWOULDBLOCK)
         throw ESocketConverseError_SendingPacket();
   }

   return result;
}

#include <csignal>

Void ESocketConverse::send(Bool override)
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

   if (getState() != CONNECTED)
   {
      std::raise(SIGINT);
      throw ESocketConverseError_InvalidSendState(getStateDescription(getState()));
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
         throw ESocketConverseError_ReadingWritePacketLength(msg.c_str());
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
            throw ESocketConverseError_ReadingWritePacketLength(msg.c_str());
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

Int ESocketConverse::peek(pUChar dest, Int len)
{
   return m_rbuf.peekData(dest, 0, len);
}

Int ESocketConverse::read(pUChar dest, Int len)
{
   return m_rbuf.readData(dest, 0, len);
}

Void ESocketConverse::write(pUChar src, Int len)
{
   m_wbuf.writeData((pUChar)&len, 0, sizeof(len));
   m_wbuf.writeData(src, 0, len);
   send();
}

Bool ESocketConverse::onReceive()
{
   return True;
}

Void ESocketConverse::onConnect()
{
   setState(CONNECTED);
}

Void ESocketConverse::onClose()
{
   close();
}

Void ESocketConverse::onError()
{
}

////////////////////////////////////////////////////////////////////////////////
// ESocketListen
////////////////////////////////////////////////////////////////////////////////
ESocketListen::ESocketListen(ESocketThread *pthread, Int size, Int family, Int type, Int protocol)
    : ESocket(pthread, LISTEN, family, type, protocol)
{
   m_depth = -1;
   m_bufsize = size;
}

ESocketListen::ESocketListen(ESocketThread *pthread, Int size, UShort port, Int family, Int type, Int protocol)
    : ESocket(pthread, LISTEN, family, type, protocol)
{
   setPort(port);
   m_depth = -1;
   m_bufsize = size;
}

ESocketListen::ESocketListen(ESocketThread *pthread, Int size, UShort port, Int depth, Int family, Int type, Int protocol)
    : ESocket(pthread, LISTEN, family, type, protocol)
{
   setPort(port);
   m_depth = depth;
   m_bufsize = size;
}

ESocketListen::~ESocketListen()
{
}

Void ESocketListen::listen()
{
   bind();
   if (::listen(getHandle(), getDepth()) == EPC_SOCKET_ERROR)
      throw ESocketListenError_UnableToListen();
}

Void ESocketListen::listen(UShort port, Int depth)
{
   setPort(port);
   setDepth(depth);
   listen();
}

ESocketConverse *ESocketListen::createSocket(ESocketThread *pthread)
{
   ESocketConverse *pSocket = new ESocketConverse(pthread, getBufferSize());
   return pSocket;
}

Void ESocketListen::onClose()
{
   ESocket::onClose();
}

Void ESocketListen::onError()
{
}

////////////////////////////////////////////////////////////////////////////////
// ESocketThread
////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(ESocketThread, EThreadPrivate)
END_MESSAGE_MAP()

ESocketThread::ESocketThread()
{
   m_error = 0;

   int result = pipe(m_pipefd);
   if (result == -1)
      throw ESocketThreadError_UnableToOpenPipe();
   fcntl(m_pipefd[0], F_SETFL, O_NONBLOCK);

   FD_ZERO(&m_master);
}

ESocketThread::~ESocketThread()
{
}

Void ESocketThread::onInit()
{
   EThreadPrivate::onInit();
}

Void ESocketThread::onQuit()
{
   EThreadPrivate::onQuit();
}

Void ESocketThread::registerSocket(ESocket *psocket)
{
   m_socketmap.insert(std::make_pair(psocket->getHandle(), psocket));
   FD_SET(psocket->getHandle(), &m_master);
   bump();
}

Void ESocketThread::unregisterSocket(ESocket *psocket)
{
   if (m_socketmap.erase(psocket->getHandle()))
   {
      FD_CLR(psocket->getHandle(), &m_master);
      bump();
   }
}

Void ESocketThread::processSelectConnect(ESocket *psocket)
{
   if (psocket->getStyle() == ESocket::CONVERSE)
      ((ESocketConverse *)psocket)->onConnect();
}

Void ESocketThread::processSelectClose(ESocket *psocket)
{
   psocket->onClose();
}

Void ESocketThread::processSelectAccept(ESocket *psocket)
{
   if (psocket->getStyle() == ESocket::LISTEN)
   {
      bool more = true;
      while (more)
      {
         try
         {
            struct sockaddr ipaddr;
            socklen_t ipaddrlen = sizeof(ipaddr);

            EPC_SOCKET handle = ::accept(((ESocketListen *)psocket)->getHandle(), &ipaddr, &ipaddrlen);
            if (handle == EPC_INVALID_SOCKET)
            {
               Int err = EPC_LASTERROR;
               if (err == EPC_EWOULDBLOCK)
                  break;
               throw ESocketError_UnableToAcceptSocket();
            }

            ESocketConverse *pnewsocket = ((ESocketListen *)psocket)->createSocket(this);
            if (pnewsocket)
            {
               pnewsocket->setHandle(handle);
               pnewsocket->setState(ESocket::CONNECTED);
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

Void ESocketThread::processSelectRead(ESocket *psocket)
{
   if (psocket->getStyle() == ESocket::LISTEN)
   {
      processSelectAccept(psocket);
   }
   else if (psocket->getStyle() == ESocket::CONVERSE)
   {
      if (psocket->getState() == ESocket::CONNECTING)
         ((ESocketConverse *)psocket)->onConnect();

      while (true)
      {
         try
         {
            Int amtRead = ((ESocketConverse *)psocket)->recv();
            if (amtRead <= 0)
               break;
         }
         catch (EError &err)
         {
            //printf("errorHandler() 2\n");
            errorHandler(err, psocket);
         }
      }

      ((ESocketConverse *)psocket)->onReceive();

      if (psocket->getState() == ESocket::DISCONNECTED)
         processSelectClose(psocket);
   }
}

Void ESocketThread::processSelectWrite(ESocket *psocket)
{
   if (psocket->getStyle() == ESocket::CONVERSE)
   {
      if (psocket->getState() == ESocket::CONNECTING)
      {
         psocket->setState(ESocket::CONNECTED);
         ((ESocketConverse *)psocket)->onConnect();
      }
      else
      {
         try
         {
            ((ESocketConverse *)psocket)->send(True);
         }
         catch (EError &err)
         {
            //printf("errorHandler() 3\n");
            errorHandler(err, psocket);
         }
      }
   }
}

Void ESocketThread::processSelectError(ESocket *psocket)
{
   psocket->onError();
}

Void ESocketThread::messageQueued()
{
   EThreadPrivate::messageQueued();
   bump();
}

Void ESocketThread::onError()
{
}

int ESocketThread::getMaxFileDescriptor()
{
   if (m_socketmap.size() == 0)
      return m_pipefd[0];

   int maxfd = m_socketmap.rbegin()->first;

   return (maxfd > m_pipefd[0]) ? maxfd : m_pipefd[0];
}

Void ESocketThread::bump()
{
   if (write(m_pipefd[1], "~", 1) == -1)
      throw ESocketThreadError_UnableToWritePipe();
}

Void ESocketThread::clearBump()
{
   char buf[1];
   while (true)
   {
      if (read(m_pipefd[0], buf, 1) == -1)
      {
         if (errno == EWOULDBLOCK)
            break;
         throw ESocketThreadError_UnableToReadPipe();
      }
   }
}

Bool ESocketThread::pumpMessagesInternal()
{
   EThreadMessage msg;

   try
   {
      while (True)
      {
         if (!pumpMessage(msg, false) || msg.getMsgId() == EM_QUIT)
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
   return msg.getMsgId() != EM_QUIT;
}

Void ESocketThread::pumpMessages()
{
   int maxfd, fd, fdcnt;
   ESocketMap::const_iterator socket_it;
   fd_set readworking, writeworking, errorworking;
   while (true)
   {
      {
         memcpy(&readworking, &m_master, sizeof(m_master));
         FD_SET(m_pipefd[0], &readworking);

         FD_ZERO(&writeworking);
         for (ESocketMap::const_iterator it = m_socketmap.begin(); it != m_socketmap.end(); it++)
         {
            ESocket *pSocket = it->second;
            if (pSocket->getStyle() == ESocket::CONVERSE &&
                (((ESocketConverse *)pSocket)->getSending() ||
                 pSocket->getState() == ESocket::CONNECTING))
               FD_SET(it->first, &writeworking);
            //                if (it->second->getStyle() == ESocket::CONVERSE &&
            //                    (((ESocketConverse*)it->second)->getSending() ||
            //                     it->second->getState() == ESocket::CONNECTING))
            //                    FD_SET(it->first, &writeworking);
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
               ESocket *pSocket = socket_it->second;
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
               ESocket *pSocket = socket_it->second;
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
               ESocket *pSocket = socket_it->second;
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
      ESocketMap::iterator it = m_socketmap.begin();
      if (it == m_socketmap.end())
         break;
      ESocket *psocket = it->second;
      m_socketmap.erase(it);
      delete psocket;
   }
}
