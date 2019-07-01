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

#ifndef __esocket_h_included
#define __esocket_h_included

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

DECLARE_ERROR_ADVANCED(ESocketError_UnableToCreateSocket);
DECLARE_ERROR_ADVANCED(ESocketError_UnableToBindSocket);
DECLARE_ERROR_ADVANCED(ESocketError_UnableToAcceptSocket);
DECLARE_ERROR_ADVANCED4(ESocketError_GetAddressInfo);
DECLARE_ERROR_ADVANCED4(ESocketError_NoAddressesFound);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ESocketListen;
class ESocketConverse;
class ESocketThread;

class ESocket
{
   friend class ESocketListen;
   friend class ESocketConverse;
   friend class ESocketThread;

public:
   typedef enum
   {
      DISCONNECTED,
      CONNECTING,
      CONNECTED
   } SOCKETSTATE;
   typedef enum
   {
      CONVERSE,
      LISTEN
   } SOCKETSTYLE;

   virtual ~ESocket();

   virtual Void onClose();
   virtual Void onError();

   ESocketThread *getThread() { return m_thread; }

   Void setFamily(Int family) { m_family = family; }
   Int getFamily() { return m_family; }

   Void setType(Int type) { m_type = type; }
   Int getType() { return m_type; }

   Void setProtocol(Int protocol) { m_protocol = protocol; }
   Int getProtocol() { return m_protocol; }

   Int getError() { return m_error; }

   Void setPort(UShort port) { m_port = port; }
   UShort getPort() { return m_port; }

   SOCKETSTATE getState() { return m_state; }
   Bool isConnected() { return getState() == CONNECTED; }

   SOCKETSTYLE getStyle() { return m_style; }

   Void setIpAddress(cpStr addr);
   cpStr getIpAddress();

   Void bind();
   Void bind(UShort port);

   Void disconnect();
   Void close();

   cpStr getStateDescription(ESocket::SOCKETSTATE state);

   Int getHandle() { return m_handle; }

protected:
   ESocket(ESocketThread *pthread, SOCKETSTYLE style, Int family = AF_INET, Int type = SOCK_STREAM, Int protocol = 0);
   Void createSocket(Int family, Int type, Int protocol);
   Void assignAddress(cpStr ipaddr, UShort port, Int family, Int socktype,
                      Int flags, Int protocol, struct addrinfo **address);
   Void setState(SOCKETSTATE state) { m_state = state; }
   Int setError();
   Void setError(Int error) { m_error = error; }
   Void setHandle(Int handle);

private:
   Void setOptions();

   ESocketThread *m_thread;

   SOCKETSTYLE m_style;
   Int m_family;
   Int m_type;
   Int m_protocol;
   Int m_error;

   EString m_ipaddr;
   UShort m_port;

   SOCKETSTATE m_state;

   Int m_handle;
};

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

//
// NOTE:  Since both the write and read routines use m_tmp for buffer
//        operations, all writes should be performed from within the
//        socket thread.
//

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

#endif // #define __esocket_h_included
