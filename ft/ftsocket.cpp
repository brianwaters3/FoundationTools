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

#include "ftsocket.h"

#define ft_closesocket(a) ::close(a)
#define FT_LASTERROR errno
#define FT_INVALID_SOCKET -1
#define FT_EINPROGRESS EINPROGRESS
#define FT_EWOULDBLOCK EWOULDBLOCK
#define FT_SOCKET_ERROR -1
typedef Int FT_SOCKET;
typedef void *PSOCKETOPT;
typedef void *PSNDRCVBUFFER;
typedef socklen_t FT_SOCKLEN;

////////////////////////////////////////////////////////////////////////////////
// FTSocketError
////////////////////////////////////////////////////////////////////////////////
FTSocketError_UnableToCreateSocket::FTSocketError_UnableToCreateSocket()
{
   setSevere();
   setTextf("%s: Error creating socket - ", Name());
   appendLastOsError();
}

FTSocketError_UnableToBindSocket::FTSocketError_UnableToBindSocket()
{
   setSevere();
   setTextf("%s: Error binding socket in bind() - ", Name());
   appendLastOsError();
}

FTSocketError_UnableToAcceptSocket::FTSocketError_UnableToAcceptSocket()
{
   setSevere();
   setTextf("%s: Error accepting new connection in accept() - ", Name());
   appendLastOsError();
}

FTSocketError_GetAddressInfo::FTSocketError_GetAddressInfo(cpStr msg)
{
   setSevere();
   setTextf("%s: Error getting address info in getaddrinfo() - %s", Name(), msg);
}

FTSocketError_NoAddressesFound::FTSocketError_NoAddressesFound(cpStr msg)
{
   setSevere();
   setTextf("%s: Error no addresses found in getaddrinfo() for %s - ", Name(), msg);
   appendLastOsError();
}

FTSocketListenError_UnableToListen::FTSocketListenError_UnableToListen()
{
   setSevere();
   setTextf("%s: Error executing listen - ", Name());
   appendLastOsError();
}

FTSocketConverseError_UnableToConnect::FTSocketConverseError_UnableToConnect()
{
   setSevere();
   setTextf("%s: Unable to connect - ", Name());
   appendLastOsError();
}

FTSocketConverseError_OutOfMemory::FTSocketConverseError_OutOfMemory(cpStr msg)
{
   setSevere();
   setTextf("%s: Out of memory in %s", Name(), msg);
}

FTSocketConverseError_UnableToRecvData::FTSocketConverseError_UnableToRecvData()
{
   setSevere();
   setTextf("%s: Error while executing recv() - ", Name());
   appendLastOsError();
}

FTSocketConverseError_InvalidSendState::FTSocketConverseError_InvalidSendState(cpStr msg)
{
   setSevere();
   setTextf("%s: Invalid state while sending: %s", Name(), msg);
}

FTSocketConverseError_ReadingWritePacketLength::FTSocketConverseError_ReadingWritePacketLength(cpStr msg)
{
   setSevere();
   setTextf("%s: Unable to read the write packet length - %s", Name(), msg);
}

FTSocketConverseError_SendingPacket::FTSocketConverseError_SendingPacket()
{
   setSevere();
   setTextf("%s: Error while attempting to send a packet of data via send() - ", Name());
   appendLastOsError();
}

FTSocketConverseError_UnrecognizedSendReturnValue::FTSocketConverseError_UnrecognizedSendReturnValue(cpStr msg)
{
   setSevere();
   setTextf("%s: Unrecoginzed return value - %s", Name(), msg);
}

FTSocketThreadError_UnableToOpenPipe::FTSocketThreadError_UnableToOpenPipe()
{
   setSevere();
   setTextf("%s: Error while opening select pipe - ", Name());
   appendLastOsError();
}

FTSocketThreadError_UnableToReadPipe::FTSocketThreadError_UnableToReadPipe()
{
   setSevere();
   setTextf("%s: Error while reading select pipe - ", Name());
   appendLastOsError();
}

FTSocketThreadError_UnableToWritePipe::FTSocketThreadError_UnableToWritePipe()
{
   setSevere();
   setTextf("%s: Error while writing select pipe - ", Name());
   appendLastOsError();
}

////////////////////////////////////////////////////////////////////////////////
// FTSocket
////////////////////////////////////////////////////////////////////////////////
FTSocket::FTSocket(FTSocketThread *pthread, SOCKETSTYLE style, Int family, Int type, Int protocol)
{
   m_thread = pthread;

   m_style = style;
   m_family = family;
   m_type = type;
   m_protocol = protocol;

   m_handle = FT_INVALID_SOCKET;
   m_port = -1;

   m_state = DISCONNECTED;
}

FTSocket::~FTSocket()
{
   close();
}

Void FTSocket::onClose()
{
}

Void FTSocket::onError()
{
}

Int FTSocket::setError()
{
   m_error = errno;
   return m_error;
}

Void FTSocket::createSocket(Int family, Int type, Int protocol)
{
   m_handle = socket(getFamily(), getType(), getProtocol());
   if (m_handle == FT_INVALID_SOCKET)
      throw FTSocketError_UnableToCreateSocket();

   setOptions();
}

Void FTSocket::bind()
{
   bind(getPort());
}

Void FTSocket::bind(UShort port)
{
   struct addrinfo *pai = NULL;

   assignAddress(NULL, port, getFamily(), getType(),
                 AI_PASSIVE, getProtocol(), &pai);

   createSocket(getFamily(), getType(), getProtocol());

   int result = ::bind(m_handle, pai->ai_addr, (FT_SOCKLEN)pai->ai_addrlen);

   freeaddrinfo(pai);

   if (result == -1)
   {
      FTSocketError_UnableToBindSocket *perr = new FTSocketError_UnableToBindSocket();
      close();
      throw perr;
   }
}

Void FTSocket::disconnect()
{
   getThread()->unregisterSocket(this);
   if (m_handle != FT_INVALID_SOCKET)
   {
      ft_closesocket(m_handle);
      m_handle = FT_INVALID_SOCKET;
      m_state = FTSocket::DISCONNECTED;
      m_ipaddr.clear();
   }
}

Void FTSocket::close()
{
   disconnect();
   onClose();
}

Void FTSocket::setHandle(Int handle)
{
   disconnect();
   m_handle = handle;
   setOptions();
}

Void FTSocket::setOptions()
{
   struct linger l;
   l.l_onoff = 1;
   l.l_linger = 0;
   setsockopt(m_handle, SOL_SOCKET, SO_LINGER, (PSOCKETOPT)&l, sizeof(l));

   fcntl(m_handle, F_SETFL, O_NONBLOCK);

   getThread()->registerSocket(this);
}

Void FTSocket::setIpAddress(cpStr addr)
{
   if (addr)
      m_ipaddr = addr;
   else
      m_ipaddr.clear();
}

cpStr FTSocket::getIpAddress()
{
   return m_ipaddr.c_str();
}

Void FTSocket::assignAddress(cpStr ipaddr, UShort port, Int family, Int socktype,
                             Int flags, Int protocol, struct addrinfo **paddrs)
{
   Int result;
   FTString sPort;
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

      throw FTSocketError_GetAddressInfo(errStr);
   }

   if (!*paddrs)
   {
      FTString msg;
      msg.format("%s:%u", ipaddr ? ipaddr : "", port);
      throw FTSocketError_NoAddressesFound(msg.c_str());
   }
}

cpStr FTSocket::getStateDescription(FTSocket::SOCKETSTATE state)
{
   cpStr pState = "Unknown";

   switch (state)
   {
   case FTSocket::CONNECTING:
   {
      pState = "CONNECTING";
      break;
   }
   case FTSocket::CONNECTED:
   {
      pState = "CONNECTED";
      break;
   }
   case FTSocket::DISCONNECTED:
   {
      pState = "DISCONNECTED";
      break;
   }
   }

   return pState;
}

////////////////////////////////////////////////////////////////////////////////
// FTSocketConverse
////////////////////////////////////////////////////////////////////////////////
FTSocketConverse::FTSocketConverse(FTSocketThread *pthread, Int bufSize, Int family, Int type, Int protocol)
    : FTSocket(pthread, FTSocket::CONVERSE, family, type, protocol),
      m_rbuf(bufSize),
      m_wbuf(bufSize)
{
   m_sending = false;
   m_remoteport = 0;
}

FTSocketConverse::~FTSocketConverse()
{
}

Void FTSocketConverse::connect()
{
   struct addrinfo *pAddress;

   assignAddress(getIpAddress(), getPort(), getFamily(),
                 getType(), 0, getProtocol(), &pAddress);

   createSocket(getFamily(), getType(), getProtocol());

   int result = ::connect(getHandle(), pAddress->ai_addr, (FT_SOCKLEN)pAddress->ai_addrlen);

   if (result == 0)
   {
      setState(CONNECTED);
      onConnect();
   }
   else if (result == -1)
   {
      setError();
      if (getError() != FT_EINPROGRESS && getError() != FT_EWOULDBLOCK)
      {
         FTSocketConverseError_UnableToConnect *err = new FTSocketConverseError_UnableToConnect();
         freeaddrinfo(pAddress);
         throw err;
      }

      setState(CONNECTING);

      getThread()->bump();
   }

   freeaddrinfo(pAddress);
}

Void FTSocketConverse::connect(cpStr ipaddr, UShort port)
{
   setIpAddress(ipaddr);
   setPort(port);
   connect();
}

Int FTSocketConverse::recv()
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
         if (getError() == FT_EWOULDBLOCK)
            break;
         throw FTSocketConverseError_UnableToRecvData();
      }
   }

   return totalReceived;
}

Int FTSocketConverse::send(pUChar pData, Int length)
{
   Int result = ::send(getHandle(), (PSNDRCVBUFFER)pData, length, MSG_NOSIGNAL);

   if (result == -1)
   {
      setError();
      if (getError() != FT_EWOULDBLOCK)
         throw FTSocketConverseError_SendingPacket();
   }

   return result;
}

Void FTSocketConverse::send(Bool override)
{
   UChar buf[2048];

   FTMutexLock lck(m_sendmtx, False);
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
      throw FTSocketConverseError_InvalidSendState(getStateDescription(getState()));

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
         FTString msg;
         msg.format("expected %d bytes, read %d bytes", sizeof(packetLength), amtRead);
         throw FTSocketConverseError_ReadingWritePacketLength(msg.c_str());
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
            FTString msg;
            msg.format("expected %d bytes, read %d bytes", sendLength, amtRead);
            throw FTSocketConverseError_ReadingWritePacketLength(msg.c_str());
         }

         // write the data to the socket
         Int amtWritten = send(buf, sendLength);
         if (amtWritten == -1) // FT_EWOULDBLOCK
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

Int FTSocketConverse::peek(pUChar dest, Int len)
{
   return m_rbuf.peekData(dest, 0, len);
}

Int FTSocketConverse::read(pUChar dest, Int len)
{
   return m_rbuf.readData(dest, 0, len);
}

Void FTSocketConverse::write(pUChar src, Int len)
{
   m_wbuf.writeData((pUChar)&len, 0, sizeof(len));
   m_wbuf.writeData(src, 0, len);
   send();
}

Bool FTSocketConverse::onReceive()
{
   return True;
}

Void FTSocketConverse::onConnect()
{
   setState(CONNECTED);
}

Void FTSocketConverse::onClose()
{
   close();
}

Void FTSocketConverse::onError()
{
}

////////////////////////////////////////////////////////////////////////////////
// FTSocketListen
////////////////////////////////////////////////////////////////////////////////
FTSocketListen::FTSocketListen(FTSocketThread *pthread, Int size, Int family, Int type, Int protocol)
    : FTSocket(pthread, LISTEN, family, type, protocol)
{
   m_depth = -1;
   m_bufsize = size;
}

FTSocketListen::FTSocketListen(FTSocketThread *pthread, Int size, UShort port, Int family, Int type, Int protocol)
    : FTSocket(pthread, LISTEN, family, type, protocol)
{
   setPort(port);
   m_depth = -1;
   m_bufsize = size;
}

FTSocketListen::FTSocketListen(FTSocketThread *pthread, Int size, UShort port, Int depth, Int family, Int type, Int protocol)
    : FTSocket(pthread, LISTEN, family, type, protocol)
{
   setPort(port);
   m_depth = depth;
   m_bufsize = size;
}

FTSocketListen::~FTSocketListen()
{
}

Void FTSocketListen::listen()
{
   bind();
   if (::listen(getHandle(), getDepth()) == FT_SOCKET_ERROR)
      throw FTSocketListenError_UnableToListen();
}

Void FTSocketListen::listen(UShort port, Int depth)
{
   setPort(port);
   setDepth(depth);
   listen();
}

FTSocketConverse *FTSocketListen::createSocket(FTSocketThread *pthread)
{
   FTSocketConverse *pSocket = new FTSocketConverse(pthread, getBufferSize());
   return pSocket;
}

Void FTSocketListen::onClose()
{
   FTSocket::onClose();
}

Void FTSocketListen::onError()
{
}

////////////////////////////////////////////////////////////////////////////////
// FTSocketThread
////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(FTSocketThread, FTThreadPrivate)
END_MESSAGE_MAP()

FTSocketThread::FTSocketThread()
{
   m_error = 0;

   int result = pipe(m_pipefd);
   if (result == -1)
      throw FTSocketThreadError_UnableToOpenPipe();
   fcntl(m_pipefd[0], F_SETFL, O_NONBLOCK);

   FD_ZERO(&m_master);
}

FTSocketThread::~FTSocketThread()
{
}

Void FTSocketThread::onInit()
{
   FTThreadPrivate::onInit();
}

Void FTSocketThread::onQuit()
{
   FTThreadPrivate::onQuit();
}

Void FTSocketThread::registerSocket(FTSocket *psocket)
{
   m_socketmap.insert(std::make_pair(psocket->getHandle(), psocket));
   FD_SET(psocket->getHandle(), &m_master);
   bump();
}

Void FTSocketThread::unregisterSocket(FTSocket *psocket)
{
   if (m_socketmap.erase(psocket->getHandle()))
   {
      FD_CLR(psocket->getHandle(), &m_master);
      bump();
   }
}

Void FTSocketThread::processSelectConnect(FTSocket *psocket)
{
   if (psocket->getStyle() == FTSocket::CONVERSE)
      ((FTSocketConverse *)psocket)->onConnect();
}

Void FTSocketThread::processSelectClose(FTSocket *psocket)
{
   psocket->onClose();
}

Void FTSocketThread::processSelectAccept(FTSocket *psocket)
{
   if (psocket->getStyle() == FTSocket::LISTEN)
   {
      bool more = true;
      while (more)
      {
         try
         {
            struct sockaddr ipaddr;
            socklen_t ipaddrlen = sizeof(ipaddr);

            FT_SOCKET handle = ::accept(((FTSocketListen *)psocket)->getHandle(), &ipaddr, &ipaddrlen);
            if (handle == FT_INVALID_SOCKET)
            {
               Int err = FT_LASTERROR;
               if (err == FT_EWOULDBLOCK)
                  break;
               throw FTSocketError_UnableToAcceptSocket();
            }

            FTSocketConverse *pnewsocket = ((FTSocketListen *)psocket)->createSocket(this);
            if (pnewsocket)
            {
               pnewsocket->setHandle(handle);
               pnewsocket->setState(FTSocket::CONNECTED);
               registerSocket(pnewsocket);
               pnewsocket->onConnect();
            }
            else
            {
               // the connection is being refused, so close the handle
               ft_closesocket(handle);
            }
         }
         catch (FTError &err)
         {
            if (err.getLastOsError() != FT_EWOULDBLOCK)
            {
               //printf("errorHandler() 1 %d\n", err->getLastOsError());
               errorHandler(err, NULL);
            }
            more = false;
         }
      }
   }
}

Void FTSocketThread::processSelectRead(FTSocket *psocket)
{
   if (psocket->getStyle() == FTSocket::LISTEN)
   {
      processSelectAccept(psocket);
   }
   else if (psocket->getStyle() == FTSocket::CONVERSE)
   {
      while (true)
      {
         try
         {
            Int amtRead = ((FTSocketConverse *)psocket)->recv();
            if (amtRead <= 0)
               break;
         }
         catch (FTError &err)
         {
            //printf("errorHandler() 2\n");
            errorHandler(err, psocket);
         }
      }

      ((FTSocketConverse *)psocket)->onReceive();

      if (psocket->getState() == FTSocket::DISCONNECTED)
         processSelectClose(psocket);
   }
}

Void FTSocketThread::processSelectWrite(FTSocket *psocket)
{
   if (psocket->getStyle() == FTSocket::CONVERSE)
   {
      if (psocket->getState() == FTSocket::CONNECTING)
      {
         psocket->setState(FTSocket::CONNECTED);
         ((FTSocketConverse *)psocket)->onConnect();
      }
      else
      {
         try
         {
            ((FTSocketConverse *)psocket)->send(True);
         }
         catch (FTError &err)
         {
            //printf("errorHandler() 3\n");
            errorHandler(err, psocket);
         }
      }
   }
}

Void FTSocketThread::processSelectError(FTSocket *psocket)
{
   psocket->onError();
}

Void FTSocketThread::messageQueued()
{
   FTThreadPrivate::messageQueued();
   bump();
}

Void FTSocketThread::onError()
{
}

int FTSocketThread::getMaxFileDescriptor()
{
   if (m_socketmap.size() == 0)
      return m_pipefd[0];

   int maxfd = m_socketmap.rbegin()->first;

   return (maxfd > m_pipefd[0]) ? maxfd : m_pipefd[0];
}

Void FTSocketThread::bump()
{
   if (write(m_pipefd[1], "~", 1) == -1)
      throw FTSocketThreadError_UnableToWritePipe();
}

Void FTSocketThread::clearBump()
{
   char buf[1];
   while (true)
   {
      if (read(m_pipefd[0], buf, 1) == -1)
      {
         if (errno == EWOULDBLOCK)
            break;
         throw FTSocketThreadError_UnableToReadPipe();
      }
   }
}

Bool FTSocketThread::pumpMessagesInternal()
{
   FTThreadMessage msg;

   try
   {
      while (True)
      {
         if (!pumpMessage(msg, false) || msg.getMsgId() == FTM_QUIT)
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
   //return (keepGoing() && msg.getMsgId() != FTM_QUIT);
   return msg.getMsgId() != FTM_QUIT;
}

Void FTSocketThread::pumpMessages()
{
   int maxfd, fd, fdcnt;
   FTSocketMap::const_iterator socket_it;
   fd_set readworking, writeworking, errorworking;
   while (true)
   {
      {
         memcpy(&readworking, &m_master, sizeof(m_master));
         FD_SET(m_pipefd[0], &readworking);

         FD_ZERO(&writeworking);
         for (FTSocketMap::const_iterator it = m_socketmap.begin(); it != m_socketmap.end(); it++)
         {
            FTSocket *pSocket = it->second;
            if (pSocket->getStyle() == FTSocket::CONVERSE &&
                (((FTSocketConverse *)pSocket)->getSending() ||
                 pSocket->getState() == FTSocket::CONNECTING))
               FD_SET(it->first, &writeworking);
            //                if (it->second->getStyle() == FTSocket::CONVERSE &&
            //                    (((FTSocketConverse*)it->second)->getSending() ||
            //                     it->second->getState() == FTSocket::CONNECTING))
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
               FTSocket *pSocket = socket_it->second;
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
               FTSocket *pSocket = socket_it->second;
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
               FTSocket *pSocket = socket_it->second;
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
      FTSocketMap::iterator it = m_socketmap.begin();
      if (it == m_socketmap.end())
         break;
      FTSocket *psocket = it->second;
      m_socketmap.erase(it);
      delete psocket;
   }
}
