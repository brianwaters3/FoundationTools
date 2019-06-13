////////////////////////////////////////////////////
// (c) 2012 Brian Waters (bdwaters@sbcglobal.net) //
////////////////////////////////////////////////////
#ifndef __FTSOCKET_H
#define __FTSOCKET_H

#include "ftbase.h"
#include "ftcbuf.h"
#include "fterror.h"
#include "ftstatic.h"
#include "ftstring.h"
#include "ftthread.h"

#if defined(FT_WINDOWS)
#include <WinSock2.h>
#include <WS2tcpip.h>
#elif defined(FT_GCC)
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#elif defined(FT_SOLARIS)
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#error "Unrecognized platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(FTSocketError_UnableToCreateSocket);
DECLARE_ERROR_ADVANCED(FTSocketError_UnableToBindSocket);
DECLARE_ERROR_ADVANCED(FTSocketError_UnableToAcceptSocket);
DECLARE_ERROR_ADVANCED4(FTSocketError_GetAddressInfo);
DECLARE_ERROR_ADVANCED4(FTSocketError_NoAddressesFound);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTSocketListen;
class FTSocketConverse;
class FTSocketThread;

class FTSocket
{
    friend class FTSocketListen;
    friend class FTSocketConverse;
    friend class FTSocketThread;

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

    virtual ~FTSocket();

    virtual Void onClose();
    virtual Void onError();

	FTSocketThread* getThread()		{ return m_thread; }

    Void setFamily(Int family)      { m_family = family; }
    Int getFamily()                 { return m_family; }

    Void setType(Int type)          { m_type = type; }
    Int getType()                   { return m_type; }

    Void setProtocol(Int protocol)  { m_protocol = protocol; }
    Int getProtocol()               { return m_protocol; }

    Int getError()                  { return m_error; }

    Void setPort(UShort port)       { m_port = port; }
    UShort getPort()                { return m_port; }

    SOCKETSTATE getState()          { return m_state; }
    Bool isConnected()              { return getState() == CONNECTED; }

    SOCKETSTYLE getStyle()          { return m_style; }

    Void setIpAddress(cpStr addr);
    cpStr getIpAddress();

    Void bind();
    Void bind(UShort port);

    Void disconnect();
    Void close();

    cpStr getStateDescription(FTSocket::SOCKETSTATE state);

#if defined(FT_WINDOWS)
    SOCKET getHandle()              { return m_handle; }
#elif defined(FT_GCC) || defined(FT_SOLARIS)
    Int getHandle()                 { return m_handle; }
#else
#error "Unrecognized platform"
#endif

protected:
    FTSocket(FTSocketThread* pthread, SOCKETSTYLE style, Int family=AF_INET, Int type=SOCK_STREAM, Int protocol=0);
    Void createSocket(Int family, Int type, Int protocol);
    Void assignAddress(cpStr ipaddr, UShort port, Int family, Int socktype,
                       Int flags, Int protocol, struct addrinfo **address);
    Void setState(SOCKETSTATE state)    { m_state = state; }
	Int setError();
    Void setError(Int error)            { m_error = error; }
#if defined(FT_WINDOWS)
    Void setHandle(SOCKET handle);
	WSAEVENT getEvent()					{ return m_event; }
#elif defined(FT_GCC) || defined(FT_SOLARIS)
    Void setHandle(Int handle);
#else
#error "Unrecognized platform"
#endif

private:
    Void setOptions();

	FTSocketThread*	m_thread;

    SOCKETSTYLE m_style;
    Int         m_family;
    Int         m_type;
    Int         m_protocol;
    Int         m_error;

    FTString    m_ipaddr;
    UShort      m_port;

    SOCKETSTATE m_state;

#if defined(FT_WINDOWS)
    SOCKET      m_handle;
	WSAEVENT	m_event;
#elif defined(FT_GCC) || defined(FT_SOLARIS)
    Int         m_handle;
#else
#error "Unrecognized platform"
#endif
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(FTSocketConverseError_UnableToConnect);
DECLARE_ERROR_ADVANCED4(FTSocketConverseError_OutOfMemory);
DECLARE_ERROR_ADVANCED(FTSocketConverseError_UnableToRecvData);
DECLARE_ERROR_ADVANCED4(FTSocketConverseError_InvalidSendState);
DECLARE_ERROR_ADVANCED4(FTSocketConverseError_ReadingWritePacketLength);
DECLARE_ERROR_ADVANCED(FTSocketConverseError_SendingPacket);
DECLARE_ERROR_ADVANCED4(FTSocketConverseError_UnrecognizedSendReturnValue);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// NOTE:  Since both the write and read routines use m_tmp for buffer
//        operations, all writes should be performed from within the
//        socket thread.
//

class FTSocketThread;

class FTSocketConverse : public FTSocket
{
    friend class FTSocketThread;

public:
    FTSocketConverse(FTSocketThread* pthread, Int bufSize, Int family=AF_INET, Int type=SOCK_STREAM, Int protocol=0);
    ~FTSocketConverse();

    Void setRemoteIpAddress(cpStr ipaddr)   { m_remoteipaddr = ipaddr; }
    cpStr getRemoteIpAddress()              { return m_remoteipaddr.c_str(); }

    Void setRemotePort(UShort port)         { m_remoteport = port; }
    UShort getRemotePort()                  { return m_remoteport; }

    Void connect();
    Void connect(cpStr ipaddr, UShort port);

    Int bytesPending()                      { return m_rbuf.used(); }

    Int peek(pUChar dest, Int len);
    Int read(pUChar dest, Int len);
    Void write(pUChar src, Int len);

    Bool getSending()   { return m_sending; }

    virtual Bool onReceive();
    virtual Void onConnect();
    virtual Void onClose();
    virtual Void onError();

protected:
    Int recv();
    Void send(Bool override = False);

private:
    FTSocketConverse() : FTSocket(NULL,FTSocket::CONVERSE), m_rbuf(0), m_wbuf(0) {}

    Int send(pUChar pData, Int length);

    FTMutex             m_sendmtx;
    Bool                m_sending;
    FTCircularBuffer    m_rbuf;
    FTCircularBuffer    m_wbuf;

    FTString    m_remoteipaddr;
    UShort      m_remoteport;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DECLARE_ERROR_ADVANCED(FTSocketListenError_UnableToListen);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class FTSocketListen : public FTSocket
{
public:
    FTSocketListen(FTSocketThread* pthread, Int size, Int family=AF_INET, Int type=SOCK_STREAM, Int protocol=0);
    FTSocketListen(FTSocketThread* pthread, Int size, UShort port, Int family=AF_INET, Int type=SOCK_STREAM, Int protocol=0);
    FTSocketListen(FTSocketThread* pthread, Int size, UShort port, Int depth, Int family=AF_INET, Int type=SOCK_STREAM, Int protocol=0);
    virtual ~FTSocketListen();

    Void setDepth(Int depth)    { m_depth = depth; };
    Int getDepth()              { return m_depth;}

    Void setBufferSize(Int size)    { m_bufsize = size; }
    Int getBufferSize()             { return m_bufsize; }

    Void listen();
    Void listen(UShort port, Int depth);

    virtual FTSocketConverse* createSocket(FTSocketThread* pthread);

    virtual Void onClose();
    virtual Void onError();

private:
    FTSocketListen() : FTSocket(NULL,FTSocket::LISTEN) {}

    Int     m_depth;
    Int     m_bufsize;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if defined(FT_WINDOWS)
DECLARE_ERROR_ADVANCED(FTSocketThreadError_TooManyWaitEvents);
DECLARE_ERROR_ADVANCED(FTSocketThreadError_UnableToCreateWaitEvent);
#elif defined(FT_GCC) || defined(FT_SOLARIS)
DECLARE_ERROR_ADVANCED(FTSocketThreadError_UnableToOpenPipe);
DECLARE_ERROR_ADVANCED(FTSocketThreadError_UnableToReadPipe);
DECLARE_ERROR_ADVANCED(FTSocketThreadError_UnableToWritePipe);
#else
#error "Unrecognized platform"
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if defined(FT_WINDOWS)
class FTSocketMap : public map<WSAEVENT, FTSocket*>
#elif defined(FT_GCC) || defined(FT_SOLARIS)
class FTSocketMap : public map<Int, FTSocket*>
#else
#error "Unrecognized platform"
#endif
{
};

class FTSocketThread : public FTThreadPrivate
{
    friend class FTSocketConverse;

public:
    FTSocketThread();
    virtual ~FTSocketThread();

    Void registerSocket(FTSocket* psocket);
    Void unregisterSocket(FTSocket* psocket);

	Int getError() { return m_error; }

protected:
    virtual Void pumpMessages();
    virtual Void errorHandler(FTError* err, FTSocket* psocket) = 0;

    virtual Void onInit();
    virtual Void onQuit();

    virtual Void messageQueued();

    virtual Void onError();

    Void bump();
    Void clearBump();

    DECLARE_MESSAGE_MAP()

private:
    Void setError(Int error)    { m_error = error; }

    Bool pumpMessagesInternal();

	Void processSelectAccept(FTSocket* psocket);
	Void processSelectConnect(FTSocket* psocket);
    Void processSelectRead(FTSocket* psocket);
    Void processSelectWrite(FTSocket* psocket);
    Void processSelectError(FTSocket* psocket);
	Void processSelectClose(FTSocket* psocket);

    Int         m_error;
    FTSocketMap m_socketmap;

#if defined(FT_WINDOWS)
	Int			m_eventCount;
	WSAEVENT	m_events[WSA_MAXIMUM_WAIT_EVENTS];
#elif defined(FT_GCC) || defined(FT_SOLARIS)
    int getMaxFileDescriptor();

    fd_set m_master;
    int m_pipefd[2];
#else
#error "Unrecognized platform"
#endif
};

#endif // __FTSOCKET_H
