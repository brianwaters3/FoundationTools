
# Enhanced Packet Core Application Development Tools for Linux

***EpcTools*** is a set of C++ classes that simplifies the development and management of EPC applications. This library is intended for use on any Linux based system that supports g++ and pthreads.
  
# Contents
<div  class="mycustomlist">
<ol>
	<li>[Dependencies](#dependencies)</li>
	<li>[Installation](#installation)</li>
	<li>[Configuration](#configuration)
		<ol>
			<li>[Command Line](#configuration-command-line)</li>
			<li>[Configuration File](#configuration-file)</li>
		</ol>
	</li>
	<li>[Feature Overview](#feature-overview)
		<ol>
			<li>[Public vs. Private](#feature-overview-public-private)</li>
			<li>[Shared Memory](#feature-overview-shared-memory)</li>
			<li>[Threads](#feature-overview-threads)
				<ol>
					<li>[Basic Thread](#feature-overview-threads-basic-thread)</li>
					<li>[Event Thread](#feature-overview-threads-event-thread)
						<ol>
							<li>[Event Queues](#feature-overview-threads-event-thread-queue)</li>
							<li>[Event Message](#feature-overview-threads-event-thread-message)</li>
							<li>[Custom Event Message](#feature-overview-threads-event-thread-message-custom)</li>
							<li>[Sending an Event](#feature-overview-threads-event-thread-event-send)</li>
							<li>[Event Processing](#feature-overview-threads-event-thread-event-processing)</li>
							<li>[Timers](#feature-overview-threads-event-thread-timers)</li>
							<li>[Example](#feature-overview-threads-event-thread-example)</li>
						</ol>
					</li>
					<li>[Public Event Threads](#feature-overview-threads-public-event-threads)</li>
				</ol>
			</li>
			<li>[Message Queue](#feature-overview-message-queue)
				<ol>
					<li>[Pack/Unpack](#feature-overview-message-queue-pack-unpack)</li>
				</ol>
			</li>
			<li>[Synchronization](#feature-overview-synchronization)
				<ol>
					<li>[Mutex](#feature-overview-synchronization-mutex)</li>
					<li>[Semaphore](#feature-overview-synchronization-semaphore)</li>
					<li>[Event](#feature-overview-synchronization-event)</li>
					<li>[Read/Write Lock](#feature-overview-synchronization-read-write-lock)</li>
				</ol>
			</li>
			<li>[Socket Communications](#feature-overview-socket-communications)
				<ol>
					<li>[Socket Thread](#feature-overview-socket-communications-socket-thread)</li>
					<li>[TCP](#feature-overview-socket-communications-tcp)</li>
					<li>[UDP](#feature-overview-socket-communications-udp)</li>
				</ol>
			</li>
			<li>[Logging](#feature-overview-logging)</li>
				<ol>
					<li>[Configuration](#feature-overview-logging-configuration)</li>
						<ol>
							<li>[JSON](#feature-overview-logging-configuration-json)</li>
							<li>[Code](#feature-overview-logging-configuration-code)</li>
							<li>[Sink Sets](#feature-overview-logging-configuration-sinksets)</li>
							<li>[Logs](#feature-overview-logging-configuration-logs)</li>
							<li>[Log Levels](#feature-overview-logging-configuration-loglevels)</li>
						</ol>
					<li>[Log Messages](#feature-overview-logging-logmessages)</li>
				</ol>
			<li>[DNS](#feature-overview-dns)
				<ol>
					<li>[Cache](#feature-overview-dns-cache)</li>
					<li>[EPC Node Discovery](#feature-overview-dns-epc-node-discovery)</li>
					<li>[Diameter S-NAPTR](#feature-overview-dns-diameter-s-naptr)</li>
				</ol>
			</li>
			<li>[REST Server](#feature-overview-rest-server)</li>
			<li>[freeDiameter](#feature-overview-freediameter)</li>
			<li>[Interface Statistics](#feature-overview-statistics)</li>
			<li>[Timer Pool](#feature-overview-timer-pool)</li>
			<li>[Miscellaneous](#feature-overview-miscellaneous)
				<ol>
					<li>[String](#feature-overview-miscellaneous-string)</li>
					<li>[Utilities](#feature-overview-miscellaneous-utilities)</li>
					<li>[Time](#feature-overview-miscellaneous-time)</li>
					<li>[Timer](#feature-overview-miscellaneous-timer)</li>
					<li> [Path](#feature-overview-miscellaneous-path)</li>
					<li>[Directory](#feature-overview-miscellaneous-directory)</li>
					<li> [Error](#feature-overview-miscellaneous-error)</li>
					<li>[Hash](#feature-overview-miscellaneous-hash)</li>
					<li>[BZip2](#feature-overview-miscellaneous-bzip2)</li>
					<li>[Circular Buffer](#feature-overview-miscellaneous-circular-buffer)</li>
				</ol>
			</li>
		</ol>
	</li>
</ol>
</div>

<a  name="dependencies"></a>
# Dependencies
* [freeDiameter](https://github.com/omec-project/freediameter) - an implementation of the Diameter protocol
* [RapidJSON](http://rapidjson.org) - a JSON parser and generator for C++
* [spdlog](https://github.com/gabime/spdlog) - very fast, header-only/compiled, C++ logging library
* [pistache](http://pistache.io/) - an elegant C++ REST framework
* [bzip2](https://www.sourceware.org/bzip2/) - a freely available, patent free (see below), high-quality data compressor
  
<a  name="installation"></a>
# Installation
***Update your system.***
```sh
$ sudo apt-get update
```
***Install Ubuntu 16.04 prerequisites.***
```sh
sudo apt-get install mercurial cmake make gcc g++ bison flex libsctp-dev libgnutls-dev libgcrypt-dev libidn11-dev m4 automake libtool
```
***Install Ubuntu 18.04 prerequisites.***
```sh
sudo apt-get install mercurial cmake make gcc g++ bison flex libsctp-dev libgnutls28-dev libgcrypt-dev libidn11-dev m4 automake libtool
```
***Clone the project, install the dependencies (via configure), build the static library and install.***
```sh
$ git clone https://github.com/brianwaters3/FoundationTools.git epctools
$ cd epctools
$ git checkout sprint
$ ./configure
$ make
$ sudo make install
```
  
<a  name="feature-overview"></a>
# Feature Overview
<a  name="feature-overview-public-private"></a>
## Public vs. Private
For some classes, ***EpcTools*** supports the concept of public classes and private. A public class/object is one that can be accessed from a different process, while a private class/object can only be accessed from within the current process. This is achieved by storing the data associated with the public object in shared memory, thereby giving access to the public objects to any process.
  
***NOTE:*** Since public objects have data stored in shared memory, each application that wants to have access to the public object must enable the EpcTools/EnablePublicObjects configuration option to TRUE. By default, the EpcTools/EnablePublicObjects configuration option is `FALSE`. Additionally, if an application does not call [EpcTools::UnInitialize()](@ref EpcTools::UnInitialize) prior to exiting, the objects in shared memory will be remain until the shared memory is released. ***This condition can lead to unexpected/unexplained application behavior.***
  
The classes that support public/private are:
| Public | Private | Description |
| :------- | :--------- | :----------- |
| [EMutexPublic](@ref EMutexPublic) | [EMutexPrivate](@ref EMutexPrivate) | A mutex (mutual exclusion object) is a program object that is created so that multiple threads can take turns sharing the same resource, such as a file or list. An application can attach to a public mutex specifying the mutex ID when calling the [attach()](@ref EMutexPublic::attach) method.|
| [ESemaphorePublic](@ref ESemaphorePublic) | [ESemaphorePrivate](@ref ESemaphorePrivate) | A semaphore is simply a variable which is non-negative and shared between threads. The ***EpcTools*** semaphore is what is considered a counting semaphore. Semaphores are used internally in ***EpcTools*** as part of various queue implementations to represent the number of messages in the queue. If you attempt to read a queue that is empty, the semaphore will block until a message has been added to the queue. An application can attach to a public semaphore by specifying the semaphore ID when calling the [attach()](@ref ESemaphorePublic::attach) method.|
| [EQueuePublic](@ref EQueuePublic) | [EQueuePrivate](@ref EQueuePrivate) | A message queue is a FIFO (first in first out) list of messages. Since these messages may be shared across processes, pointer values are not allowed. A message class derived from [EMessage](@ref EMessage) provides utilities to pack and unpack data that can be written to and read from a queue. An application can attach to a public queue by specifying the queue ID when calling the [init()](@ref EQueuePublic::init) method.|
| [EThreadPublic](@ref EThreadPublic) | [EThreadPrivate](@ref EThreadPrivate) | A thread provides multiple threads of execution within the same program in a shared memory address space. Threads allow for concurrent programming and, on multiple processor/core systems, true parallelism. To send an event message to a public thread from a different process, the application must first create an instance of [EThreadPublic](@ref EThreadPublic) that refers to the same application ID and thread ID that the target thread was initialized with. |
 
<a  name="feature-overview-shared-memory"></a>
## Shared Memory
Inter process communication through shared memory is a concept where two or more processes can access the common memory and changes to the shared memory made by one process can viewed (and changed) by another process. The [ESharedMemory](@ref ESharedMemory) class provides functionality to allocate, access and release a shared memory block. Once created, a pointer to the shared memory can be retrieved using the [getDataPtr()](@ref ESharedMemory::getDataPtr) method. Concurrent access to the shared memory should be controlled via an instance of [EMutexPublic](@ref EMutexPublic).
  
```cpp
ESharedMemory sm;
sm.init( "/tmp/sm1", 1, 1048576 );
void *p = sm.getDataPtr();
```
 
This example either creates or attaches to the shared memory identified by the file `"/tmp/sm1"` and a shared memory ID of 1 and is 1MB in size. The variable `p` is assigned the first address of the 1MB shared memory block. When `sm` goes out of scope, the shared memory will be released if no other clients are attached.
  
<a  name="feature-overview-threads"></a>
## Threads
<a  name="feature-overview-threads-basic-thread"></a>
### Basic Thread
A basic thread is a thread wrapper that will execute a user provided procedure/function in a separate thread. The basic thread is defined by deriving a class from [EThreadBasic](@ref EThreadBasic) and overloading the `threadProc()` method. To initialize and start the thread simply call the `init(pVoid arg, Dword stackSize = 0)` method. Call [join()](@ref EThreadBasic::join) to wait for the thread to exit. Other useful [EThreadBasic](@ref EThreadBasic) methods include [sleep(Int milliseconds)](@ref EThreadBasic::sleep) and [yield()](@ref EThreadBasic::yield).
  
In this example, a basic thread is defined in the class `EThreadBasicTest`.
  
```cpp
class  EThreadBasicTest : public  EThreadBasic {
public:
	EThreadBasicTest() : m_timetoquit(false) {}
	  
	Dword  threadProc(Void  *arg)
	{
		while (!m_timetoquit)
		{
			cout << "Inside the thread [" << (cpStr)arg << "]" << endl;
			sleep(1000);
		}
		cout << "Exiting EThreadTest::threadProc()" << endl;
		return  0;
	}
	  
	Void  setTimeToQuit()
	{
		m_timetoquit = true;
	}
  
private:
	bool m_timetoquit;
};
  
Void  EThreadBasic_test()
{
	cout << "EThread_test() Start" << endl;
	  
	EThreadBasicTest t;
	  
	t.init((Void *)"this is the thread argument");
	cout << "before 5 second sleep sleep" << endl;
	t.sleep(5000);
	cout << "before setTimeToQuit()" << endl;
	t.setTimeToQuit();
	cout << "before join" << endl;
	t.join();
	  
	cout << "EThread_test() Complete" << endl;
}
```
  
<a  name="feature-overview-threads-event-thread"></a>
### Event Thread
An event thread responds to event messages sent to the thread by invoking the associated message handler method defined in the class. The event message queue for the thread can either be allocated from the heap, a private event thread, or allocated from shared memory, a public event thread. [EThreadEvent](@ref EThreadEvent) is a templated class that takes two parameters 1) the queue class and 2) the event message class to be used for this thread.
  
<a  name="feature-overview-threads-event-thread-queue"></a>
#### Event Queues
The two event queue template classes, [EThreadQueuePublic](@ref EThreadQueuePublic) and [EThreadQueuePrivate](@ref EThreadQueuePrivate), each take a template parameter of an event message class. [EThreadQueuePublic](@ref EThreadQueuePublic) creates the event queue in shared memory allowing thread events to be posted from any process, and [EThreadQueuePrivate](@ref EThreadQueuePrivate) creates the event queue on the heap which limits thread events to only be posted from within the same process.
  
<a  name="feature-overview-threads-event-thread-message"></a>
#### Event Message
The event message object will be passed to the event handler method defined in the derived thread class. The basic functionality of an event message is encapsulated in the [EThreadEventMessageBase](@ref EThreadEventMessageBase) and contains the event message ID and a timer that measures the amount of time an event message spends in the event queue. Additionally, the event message must contain a void pointer. This is needed to distribute timer events.

The standard implementation of an event message is contained in [EThreadMessage](@ref EThreadMessage). The event data for this class is a union defined as follows:

```cpp
typedef union {
	pVoid voidptr;
	LongLong int64;
	Long int32[sizeof(pVoid) / sizeof(Long)];
	Short int16[sizeof(pVoid) / sizeof(Short)];
	Char int8[sizeof(pVoid) / sizeof(Char)];
	ULongLong uint64;
	ULong uint32[sizeof(pVoid) / sizeof(ULong)];
	UShort uint16[sizeof(pVoid) / sizeof(UShort)];
	UChar uint8[sizeof(pVoid) / sizeof(UChar)];
} DataUnion;
```

[EThreadMessage](@ref EThreadMessage) has several overloaded constructors that can populate various elements of the union. Additionally, one constructor takes a reference to an instance of [EThreadEventMessageData](@ref EThreadEventMessageData) which provides the developer with the ability to initialize any combination of union members.
  
<a  name="feature-overview-threads-event-thread-message-custom"></a>
#### Custom Event Message
A custom or user-defined event message class can be used for a thread by developing the following classes:
  
1. Define the data that will be contained in the custom event message by deriving a class from [EThreadEventMessageDataBase](@ref EThreadEventMessageDataBase).
```cpp
class  MyCustomEventData : public  EThreadEventMessageDataBase
{
public:
	MyCustomEventData() : EThreadEventMessageDataBase(), m_voidptr(), m_print(False), m_val() {}
	MyCustomEventData(UInt  msgid) : EThreadEventMessageDataBase(msgid), m_voidptr(), m_val() {}
	  
	pVoid  getVoidPtr() { return m_voidptr; }
	Void  setVoidPtr(pVoid  p) { m_voidptr = p; }
	  
	Void  setValue(Int  idx, Int  val) { if (idx>=0 && idx<4) m_val[idx] = val; }
	Int  getValue(Int  idx) { if (idx>=0 && idx<4) return  m_val[idx]; return -1; }
	  
	Void  setPrint(Bool  print) { m_print = print; }
	Bool  getPrint() { return m_print; }
	  
private:
	pVoid m_voidptr;
	Bool m_print;
	Int m_val[4];
};
```

**NOTE:** The custom event data object must provide access to a void pointer and provide definitions for [EThreadEventMessageDataBase::getVoidPtr()](@ref EThreadEventMessageDataBase::getVoidPtr) and [EThreadEventMessageDataBase::setVoidPtr()](@ref EThreadEventMessageDataBase::setVoidPtr).
  
2. Define the custom event message class by deriving a class from [EThreadEventMessageBase](@ref EThreadEventMessageBase) utilizing the custom event data class previously defined.
```cpp
typedef EThreadEventMessageBase<MyCustomEventData> MyCustomEvent;
```
  
3. Define the custom event queue class utilizing the previously defined custom event message class.
```cpp
typedef EThreadQueuePublic<MyCustomEvent> MyCustomEventPublicQueue;
```
  
4. Finally, define the custom thread class that will utilize the previously defined custom event queue class.
```cpp
typedef EThreadEvent<MyCustomEventPublicQueue, MyCustomEvent> MyCustomThreadEventPublic;
```
  
5. Derive a thread class from the newly defined custom thread class and define the message handlers using the custom event message class.
```cpp
class  MyCustomThread : public  MyCustomThreadEventPublic
{
public:
	...
	Void  myhandler(MyCustomEvent  &msg)
	{
		if (msg.data().getPrint())
		{
		std::cout << "MyCustomThread::myhandler() -"
				  << " event=" << msg.data().getMessageId()
				  << " m_voidptr=" << msg.data().getVoidPtr()
				  << " m_print=" << (msg.data().getPrint()?"TRUE":"FALSE")
				  << " m_val[0]=" << msg.data().getValue(0)
				  << " m_val[1]=" << msg.data().getValue(1)
				  << " m_val[2]=" << msg.data().getValue(2)
				  << " m_val[3]=" << msg.data().getValue(3)
				  << std::endl;
		}
	}
	...
};
```
  
<a  name="feature-overview-threads-event-thread-event-processing"></a>
#### Event Processing
  
**Standard Events and Callbacks**
| Event | Callback Method | Description |
| ------- | -------------------- | :----------- |
| EM_INIT |[onInit()](@ref EThreadEvent::onInit) | The EM_INIT message will is posted to the thread when the thread is started. Normally, this is the first message that will be processed. However, for a public thread, it is possible that the process sending thread events could start before the thread is started which would result in the EM_INIT event to not be the first event processed.|
| EM_QUIT | [onQuit()](@ref EThreadEvent::onQuit) | This event is posted to the thread when the [quit()](@ref EThreadEvent::quit) method is invoked on the thread. This will be the last event processed by the thread before the thread exits.|
| EM_SUSPEND | [onSuspend()](@ref EThreadEvent::onSuspend) | This event is posted to a thread when the [suspend()](@ref EThreadEvent::suspend) method has been invoked. This will be the last event processed prior to the thread suspending event processing.|
| EM_TIMER | [onTimer()](@ref EThreadEvent::onTimer) | The EM_TIMER event will be posted to a thread when a timer associated with the thread expires. A pointer to the timer object that has expired is included as an argument to the [onTimer()](@ref EThreadEvent::onTimer) method.|
  
User defined events ID's start with EM_USER. The association between the event ID and the event handler is defined in the message map. The following macros are used for declaring the message map and creating the association between the event ID and the event handler.
  
**Message Map Macros**
| Macro | Arguments | Description |
| ----- | :-------- | :----------- |
| [DECLARE_MESSAGE_MAP()](@ref DECLARE_MESSAGE_MAP) | None | This macro must appear in the class definition. It's inclusion will create the necessary definitions within the class to support the message map definition and usage. |
| [BEGIN_MESSAGE_MAP()](@ref BEGIN_MESSAGE_MAP) | The class<br>The base class | This macro starts the definition of the message map. The first argument is the current class name and the second argument is base class of the current class. |
| [ON_MESSAGE()](@ref ON_MESSAGE) | Event ID<br>Event handler method | Each user event that is to be handled by the class must have an entry in the message map that establishes the association between the event ID and the event handler method. The event handler method name must be fully qualified. |
| [END_MESSAGE_MAP()](@ref END_MESSAGE_MAP) | None | This macro closes the message map declaration. |
  
The message map declaration within the class should be performed as follows:
```cpp
#define  MYEVENT (EM_USER + 1)
  
class  MyThread : public  EThreadPrivate
{
public:
	...
	Void  myhandler(EThreadMessage  &msg);
	DECLARE_MESSAGE_MAP()
	...
};
```
  
The definition of the message map should happen in code as follows:
```cpp
BEGIN_MESSAGE_MAP(MyThread, EThreadPrivate)
	ON_MESSAGE(MYEVENT, MyThread::myhandler)
END_MESSAGE_MAP()
```
 
<a  name="feature-overview-threads-event-thread-inheritance"></a>
#### Event Dispatching and Inheritance
When an event is posted to the thread's event queue, the thread will dequeue the event message and call the event handler associated with the event ID. This process is referred to as event dispatching. The association between the event ID and the event handler is defined in the message map.
  
```cpp
BEGIN_MESSAGE_MAP(EThreadTest, EThreadPrivate)
	ON_MESSAGE(EM_USER1, EThreadTest::userFunc1)
	ON_MESSAGE(EM_USER2, EThreadTest::userFunc2)
END_MESSAGE_MAP()
```
In this example, `EThreadTest` is derived from [EThreadPrivate](@ref EThreadPrivate) and there are two event handlers defined in `EThreadTest`: `userFunc1()` and `userFunc2()`. According to the message map, when event `EM_USER1` is received, the dispatcher will call `userFunc1()` and when `EM_USER2` is received, the dispatcher will call `userFunc2()`.
  
The event dispatcher searches the message map looking for the event ID. If the event ID is not found, the dispatcher will then search the parent class which is the second parameter in the `BEGIN_MESSAGE_MAP` macro. This process will continue until a handler is identified or there are no more base classes to evaluate. In this case, the [defMessageHandler()](@ref EThreadEvent) will be called to process the message.
```cpp
class  EThreadTestParent : EThreadPrivate
{
	....
	Void  userFunc1(EThreadMessage  &msg);
	Void  userFunc2(EThreadMessage  &msg);
	....
};
  
BEGIN_MESSAGE_MAP(EThreadTestParent, EThreadPrivate)
	ON_MESSAGE(EM_USER1, EThreadTestParent::userFunc1)
	ON_MESSAGE(EM_USER2, EThreadTestParent::userFunc2)
END_MESSAGE_MAP()
  
class  EThreadTestChild : public  EThreadTestParent
{
	....
	Void  childUserFunc1(EThreadMessage  &msg);
	....
};
  
BEGIN_MESSAGE_MAP(EThreadTestChild, EThreadTestParent)
	ON_MESSAGE(EM_USER1, EThreadTestChild::childUserFunc1)
END_MESSAGE_MAP()
```

The above example demonstrates the inheritance behavior of the event dispatcher. When an instance of `EThreadTestParent` is created and `EM_USER1` is sent to it, `EThreadTestParent::userFunc1()` will be called to process the event. Similarly, when an instance of `EThreadTestChild` is created and `EM_USER1` is sent to it, `EThreadTestChild::childUserFunc1()` will be called. Finally, when `EM_USER2` is sent to the instance of `EThreadTestChild`, the event dispatcher will call `EThreadTestParent::userFunc2()` to process the event message since there isn't an event handler defined in `EThreadTestChild` to process `EM_USER2`.

<a  name="feature-overview-threads-event-thread-timers"></a>
#### Timers
[EThreadEvent](@ref EThreadEvent) supports two types of timers: a periodic timer and a one-shot timer. A periodic timer will emit a timer expiration event X number of milliseconds as defined by the interval of the timer. These timer expiration events will continue until the timer is stopped or destroyed. By contrast, a one-shot timer will generate a single timer expiration event after the duration specified by the timer's interval value.
  
Both timer types, periodic and one-shot, are represented by the [EThreadEventTimer](@ref EThreadEventTimer) class.
  
**Periodic Timer Setup**
```cpp
EThreadEventTimer mytimer;
mytimer.setInterval(1000);
mytimer.setOneShot(False);
mythread.initTimer(mytimer);
mytimer.start();
```

This example will create a periodic timer that will expire once every 1,000 milliseconds (1 second). Each time the timer expires, the onTimer() method of the `mythread` object will be invoked.
  
**One-Shot Timer Setup**
```cpp
EThreadEventTimer mytimer;
mytimer.setInterval(1000);
mytimer.setOneShot(True);
mythread.initTimer(mytimer);
mytimer.start();
```

This example will create a one-shot timer that will expire after 1,000 milliseconds (1 second). The timer can be re-used by calling `mytimer.start()`.
  
Both periodic and one-shot timers can be stopped by calling the timer `stop()` method.
  
<a  name="feature-overview-threads-event-thread-example"></a>
#### Example
In this example, the has 2 timers that are associated with it, the periodic timer and the overall timer. When the periodic timer expires, for example every 1 second, the onTimer() method will be called which will in turn send the MYEVENT event to the thread which will in turn invoke the myhandler() thread event handler method. When the overall timer expires, for example after 10 seconds, the onTimer() method will call quit() which will trigger the onQuit method to be invoked and the thead will exit. Both timers are initialized and started when the thread is started and invokes the onInit() method. The threadExample() function instantiates the thread and waits for the thread to exit.
  
```cpp
#define  MYEVENT (EM_USER + 1)

class  MyThread : public  EThreadPrivate
{
public:
	MyThread(Long  periodic_ms, Long  overall_ms)
	{
		m_periodic_ms = periodic_ms;
		m_overall_ms = overall_ms;
		m_count = 0;
	}
	  
	Void  onInit()
	{
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " invoked" << std::endl << std::flush;
		  
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " initializing periodic timer(" << m_periodic.getId() << ") to "
			<< m_periodic_ms << "ms" << std::endl << std::flush;
		m_periodic.setInterval(m_periodic_ms);
		m_periodic.setOneShot(False);
		initTimer(m_periodic);
		
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " initializing overall timer(" << m_overall.getId() << ") to "
			<< m_overall_ms << "ms" << std::endl << std::flush;
		m_overall.setInterval(m_overall_ms);
		m_overall.setOneShot(False);
		initTimer(m_overall);
		
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " starting periodic timer" << "ms" << std::endl << std::flush;
		m_periodic.start();
		  
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " starting overall timer" << "ms" << std::endl << std::flush;
		m_overall.start();
		m_count = 0;
	}

	Void  onQuit()
	{
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " invoked - count=" << m_count << std::endl << std::flush;
	}

	Void  onTimer(EThreadEventTimer  *ptimer)
	{
		if (ptimer->getId() == m_periodic.getId())
		{
			sendMessage(MYEVENT);
		}
		else  if (ptimer->getId() == m_overall.getId())
		{
			quit();
		}
	}
	  
	Void  myhandler(EThreadMessage  &msg)
	{
		std::cout << ETime::Now().Format("%i",True) << " " << __PRETTY_FUNCTION__
			<< " invoked" << std::endl;
		m_count++;
	}
  
	DECLARE_MESSAGE_MAP()
  
private:
	MyThread();
	Long m_periodic_ms;
	Long m_overall_ms;
	Int m_count;
	EThreadEventTimer m_periodic;
	EThreadEventTimer m_overall;
};

BEGIN_MESSAGE_MAP(MyThread, EThreadPrivate)
	ON_MESSAGE(MYEVENT, MyThread::myhandler)
END_MESSAGE_MAP()
  
Void  threadExample()
{
	static Long periodic_ms = 1000;
	static Long overall_ms = 10000;
	Char buffer[128];
	
	cout << "Enter the periodic timer duration in milliseconds [" << periodic_ms << "]: ";
	cin.getline(buffer, sizeof(buffer));
	periodic_ms = *buffer ? std::stol(buffer) : periodic_ms;
	
	cout << "Enter the overall timer duration in milliseconds [" << overall_ms << "]: ";
	cin.getline(buffer, sizeof(buffer));
	overall_ms = *buffer ? std::stol(buffer) : overall_ms;
	
	MyThread t(periodic_ms, overall_ms);
	
	std::cout << ETime::Now().Format("%i",True) << " Starting thread example" << std::endl;
	t.init(1,1,NULL);
	t.join();
	std::cout << ETime::Now().Format("%i",True) << " Thread example complete" << std::endl;
}
```
  
### Public Event Threads
The benefit of a public event thread over a private event thread is that thread event messages can be sent from a thread in one process to a thread in another process. This is achieved by storing the event message queue in shared memory. When a process creates an instance of a thread class that has a event message queue derived from [EThreadQueuePublic](@ref EThreadQueuePublic) the event message queue will be stored in shared memory. The application ID and thread ID that the thread is initialized with are used to uniquely identify the event message queue in shared memory.
  
Here is sample code for the application that will host the public thread.
```cpp
class  MyPublicThread : public  EThreadPublic
{
public:
	MyPublicThread()
	{
		m_count = 0;
	}
	
	Void  onInit()
	{
		std::cout << "MyPublicThread::onInit()" << std::endl << std::flush;
	}
	
	Void  onQuit()
	{
		m_timer.Stop();
		std::cout << "MyPublicThread::onQuit()" << std::endl << std::flush;
		double persec = ((double)m_count) / (((double)m_timer.MicroSeconds()) / 1000000);
		std::string s = numberFormatWithCommas<double>(persec);
		cout << "Processed " << m_count
			<< " messages in << ((double)m_timer.MicroSeconds()) / 1000000
			<< "  seconds (" << s << " per second)" << std::endl;
	}

	Void myfunc1(EThreadMessage &msg)
	{
		if (m_count == 0)
			m_timer.Start();
		m_count++;
	}

	DECLARE_MESSAGE_MAP()
private:
	Int m_count;
	ETimer m_timer;
};

BEGIN_MESSAGE_MAP(MyPublicThread, EThreadPublic)
	ON_MESSAGE(MYPUBLICEVENT, MyPublicThread::myfunc1)
END_MESSAGE_MAP()
```
  
And here is sample code for the application that will send event messages.
```cpp
...
EThreadQueuePublic<EThreadMessage> q;
Long id = MYPUBLICAPPID * 10000 + MYPUBLICTHREADID;
q.init(queueSize, id, True, EThreadQueueMode::WriteOnly);

EThreadMessage event;
event.data().setMessageId(MYPUBLICEVENT);

for (Int idx=0; idx<msgcnt; idx++)
{
	event.data().data().int64 = idx;
	q.push(event);
}
...
```
  
You will notice that application that will send the messages creates an instance of the thread queue object and the application that will host the thread and process the thread events creates an instance of the thread class.
  
***NOTE:*** Timing is important. The application that starts first will create the event message queue in shared memory. If this is the application that only sends messages, then it is possible that this application can start posting messages to the event queue before the thread has started. When the application where the thread will exist starts, it will post an `EM_INIT` message to the thread's event message queue which will trigger the [onInit()](@ref EThreadPublic::onInit) method to be called. Depending on which application starts first, it is possible that other events will be processed by the thread before the `EM_INIT` event is processed.
  
<a  name="feature-overview-message-queue"></a>
## Message Queue
  
<a  name="feature-overview-message-queue-pack-unpack"></a>
### Pack/Unpack
  
<a  name="feature-overview-synchronization"></a>
## Synchronization
  ***EpcTools*** supports several synchronization mechanisms which can be used to synchronize operations in multiple threads and processes.  

| Type | Private | Public |
| ----- | :------| :----- |
| Mutex | [EMutexPrivate](@ref EMutexPrivate) | [EMutexPublic](@ref EMutexPublic) |
| Semaphore | [ESemaphorePrivate](@ref ESemaphorePrivate) | [ESemaphorePublic](@ref ESemaphorePublic) |
| Event | [EEvent](@ref EEvent) | N/A |
| Read/Write Lock | [ERWLock](@ref ERWLock) | N/A |
  
<a  name="feature-overview-synchronization-mutex"></a>
### Mutex
A mutual exclusion or mutex allows for resource sharing by limiting access to that resource to a single thread.  An example of this would be to limit adding and removing entries to a linked list to a single thread since performing maintenance on the linked list from multiple threads would most likely cause the list pointers to become corrupted.

A private mutex is allocated from the stack or heap and provides a locking mechanism to threads in the same process.
**Mutex Example**
```cpp
EMutexPrivate m;
```

A public mutex is allocated from shared memory and provides synchronization to a single resource across processes.  Multiple processes can gain access to the same mutex by attaching to an existing mutex using the mutex ID.  The process that creates the mutex does so by simply declaring the mutex.  The mutex ID can be retrieved by using the [mutexId()](@ref EMutexPublic::mutexId) method.

```cpp
EMutexPublic m;
std::cout << "Mutex ID = "  <<  m.mutexId() << std::endl;
```
Another process can attach to an existing mutex by bypassing the mutex initialization and calling the [attach()](@ref EMutexPublic::attach) method.  The method that the "other" process finds out about the mutex ID is up to the application developer.  Locking of a public mutex is performed the same way that a private mutex is locked.

**Public Mutex Attach Example**
```cpp
EMutexPublic m(False);
m.attach( mutex_id );
```

**Locking a Mutex**

A mutex can be locked by creating an instance of [EMutexLock](@ref EMutexLock) which takes a reference to [EMutexData](@ref EMutexData) in it's constructor.  Both the [EMutexPrivate](@ref EMutexPrivate) and [EMutexPublic](@ref EMutexPublic) are derived from [EMutexData](@ref EMutexData), so either object can be passed as the constructor argument.  By default, the [EMutexLock](@ref EMutexLock::EMutexLock) constructor will wait for the lock to be obtained when object is created.  However, an optional flag can be passed to the [EMutexLock](@ref EMutexLock::EMutexLock) constructor to not acquire the lock.  In this case, the lock can be manually acquired by calling [acquire(Bool wait=True)](@ref EMutexLock::acquire).  If `wait` is True, [acquire()](@ref EMutexLock::acquire) will block until the lock can be obtained.  If `wait` is `False` and the lock cannot immediately obtained, [acquire()](@ref EMutexLock::acquire) will return False indicating that the lock was not obtained.

Regardless of how the lock was obtained, when the [EMutexLock](@ref EMutexLock) object goes out of scope, the lock on the mutex will be released.

**Mutex Lock Example**
```cpp
...
EMutexPrivate m;
...
{
	EMutexLock l(m, False);
	if (l.acquire(False))
	{
		std::cout << "The lock has been acquired." << std::endl;
	}
	else
	{
		std::cout << "The lock has NOT been acquired." << std::endl;
	}
}
```
  
<a  name="feature-overview-synchronization-semaphore"></a>
### Semaphore
A counting semaphore is simply a numeric variable that can be incremented and decremented.  For example, a semaphore could represent the number of records in a queue or the number of free entries in a pre-allocated array.

A semaphore value can be incremented using the [Increment()](@ref ESemaphoreBase::Increment) method, and the semaphore value can be decremented using the [Decrement(Bool wait=True)](@ref ESemaphoreBase::Decrement) method.  When [Decrement(Bool wait=True)](@ref ESemaphoreBase::Decrement) is called, if the numeric value is zero and `wait` is `True`, the function will block until the value is greater than zero before decrementing it.  [Decrement(Bool wait=True)](@ref ESemaphoreBase::Decrement) returns `True` if the value was successfully decremented and `False` if the value was not successfully decremented.

```cpp
// initialize a semaphore with an initial value of 5
ESemaphorePrivate s1(5);

cout <<  "Decrementing";
for (Int i = 1; i <= 5; i++)
{
	if (!s1.Decrement())
		std::cout <<  "Error decrementing semaphore on pass " << i << std::endl;
	std::cout <<  ".";
}

cout <<  "Checking for decrement action at zero...";
if (s1.Decrement(False))
	cout <<  " failed - Decrement returned true"  << endl;
else
	cout <<  "success"  << endl;
```
  
A semaphore can be either public or private as represented by [ESemaphorePublic](@ref ESemaphorePublic) and [ESemaphorePrivate](@ref ESemaphorePrivate).

<a  name="feature-overview-synchronization-event"></a>
### Event
An event object is an object that can be waited on for something to occur.  An example could be waiting for a process to complete.

**Event Example**
```cpp
class  EventExampleThread  :  public EThreadBasic
{
public:
	EThreadBasicTest()  :  m_timetoquit(false)
	{
	}

	Dword threadProc(Void *arg)
	{
		EEvent *ev1 = (EEvent*)arg;
		...
		ev1->set();
		...
		return 0;
	}
};

...
	EEvent ev1;
	EventExampleThread t;
	t.init(&ev1);
	std::cout << "waiting for ev1" << std::endl;
	ev1.wait();
	std::cout << "ev1 has been set" << std::endl;
	t.join();
...
```

<a  name="feature-overview-synchronization-read-write-lock"></a>
### Read/Write Lock
An Read/Write lock allows concurrent access for read-only operations, while write operations require exclusive access.  An example of this is accessing a list would obtain a read lock while updating the list would require a write lock.  When a write lock is obtained, it will prevent other read and/or write locks from being granted.  Similarly, if a write lock is requested and there is one or more active read locks, the write will only be granted after the read locks have been released.

**Read/Write Lock Example**
In this example, two threads will be created that will obtain read locks and a third thread will be created that will obtain a write lock.  These threads will run in different combinations that will demonstrate that multiple read locks can be obtained concurrently and only a single exclusive (of other read and write locks) write can be obtained at a time.

```cpp
#define  EM_RWLOCKTEST (EM_USER + 1)
  
class  ERWLockTestThread : public  EThreadPrivate
{
public:
	ERWLockTestThread(ERWLock  &rwl, Bool  reader, cpStr  name)
		: m_rwlock(rwl),
		  m_reader(reader),
		  m_name(name)
	{
	}
	  
	Void  handleRequest(EThreadMessage  &msg)
	{
		Int delay = (Int)msg.data().data().int32[0];
		Int hold = (Int)msg.data().data().int32[1];
		ETimer tmr;
		  
		EThreadBasic::sleep(delay);
		std::cout <<  "thread ["  << m_name <<  "] starting after "
			<< delay <<  "ms ("  <<  tmr.MilliSeconds() <<  ")"  << std::endl << std::flush;
		  
		if (m_reader)
		{
			{
				std::cout << "thread [" << m_name << "] waiting for read lock" << std::endl << std::flush;
				// obtain a read lock, the lock will be released when rdlck goes out of scope
				ERDLock rdlck(m_rwlock);
				epctime_t elapsed =  tmr.MilliSeconds();
				std::cout << "thread [" << m_name << "] read lock obtained after "
					<< elapsed << "ms - holding lock for " << hold << "ms" << std::endl << std::flush;
				EThreadBasic::sleep(hold);
			}
			std::cout <<  "thread [" << m_name << "] read lock released" << std::endl << std::flush;
		}
		else
		{
			{
				std::cout << "thread [" << m_name << "] waiting for write lock" << std::endl << std::flush;
				// obtain a write lock, the lock will be released when wrlck goes out of scope
				EWRLock wrlck(m_rwlock);
				epctime_t elapsed =  tmr.MilliSeconds();
				std::cout << "thread [" << m_name << "] write lock obtained after "
					<< elapsed << "ms - holding lock for " << hold << "ms" << std::endl << std::flush;
				EThreadBasic::sleep(hold);
			}
			std::cout << "thread [" << m_name << "] write lock released" << std::endl << std::flush;
		}
	}
	  
	DECLARE_MESSAGE_MAP()
  
private:
	ERWLockTestThread();	  
	ERWLock &m_rwlock;
	Bool m_reader;
	cpStr m_name;
};
  
BEGIN_MESSAGE_MAP(ERWLockTestThread, EThreadPrivate)
	ON_MESSAGE(EM_RWLOCKTEST, ERWLockTestThread::handleRequest)
END_MESSAGE_MAP()
  
Void  ERWLock_test()
{
	std::cout << "ERWLock_test() Start" << std::endl;
	  
	ERWLock rwl;
	  
	ERWLockTestThread read1(rwl, True, "READ1");
	ERWLockTestThread read2(rwl, True, "READ2");
	ERWLockTestThread write1(rwl, False, "WRITE1");
	  
	std::cout << "ERWLock_test - initializing threads" << std::endl << std::flush;
	read1.init(1, 1, NULL, 20000);
	read2.init(1, 2, NULL, 20000);
	write1.init(1, 3, NULL, 20000);
	  
	std::cout << "ERWLock_test - starting 1st test" << std::endl << std::flush;
	read1.sendMessage(EThreadMessage(EM_RWLOCKTEST, 0, 4000));
	read2.sendMessage(EThreadMessage(EM_RWLOCKTEST, 50, 4000));
	write1.sendMessage(EThreadMessage(EM_RWLOCKTEST, 1000, 4000));
	EThreadBasic::sleep(10000);
	std::cout << "ERWLock_test - 1st test complete" << std::endl << std::flush;
	  
	std::cout << "ERWLock_test - starting 2nd test" << std::endl << std::flush;
	read1.sendMessage(EThreadMessage(EM_RWLOCKTEST, 1000, 4000));
	read2.sendMessage(EThreadMessage(EM_RWLOCKTEST, 1050, 4000));
	write1.sendMessage(EThreadMessage(EM_RWLOCKTEST, 0, 4000));
	EThreadBasic::sleep(10000);
	std::cout << "ERWLock_test - 2nd test complete" << std::endl << std::flush;
	  
	read1.quit();
	read2.quit();
	write1.quit();
	  
	read1.join();
	read2.join();
	write1.join();
	  
	std::cout <<  "ERWLock_test() Complete"  << std::endl;
}
```

<a  name="feature-overview-socket-communications"></a>
## Socket Communications
Asynchronous socket communications is supported by ***EpcTools*** in the ESocket namespace. Currently, support for IPv4 and IPv6 with both TCP and UDP have been implemented. The framework can be enhanced to support additional socket types such as Unix domain socket.

<a  name="feature-overview-socket-communications-socket-thread"></a>
### Socket Thread
[ESocket::Thread](@ref ESocket::Thread) is derived from [EThreadEvent](@ref EThreadEvent) and has a custom message pump and dispatcher to identify and process socket events for sockets that are registered with the socket thread. The message pump function utilizes `select()` to detect when a socket can be read from, written to or when an error has occurred. These events are then dispatched to appropriate callback methods depending on the role and state of the socket.  

In addition to processing socket events [ESocket::Thread](@ref ESocket::Thread) is also capable of processing standard event messages. See [Event Thread](#feature-overview-threads-event-thread) for more information.

Since [ESocket::Thread](@ref ESocket::Thread) is derived from [EThreadEvent](@ref EThreadEvent), [ESocket::Thread](@ref ESocket::Thread) supports either a public or a private event message queue with the option of either [EThreadMessage](@ref EThreadMessage) or a custom event thread message.

**Example Socket Thread**
```cpp
class  TcpWorker : public  ESocket::ThreadPrivate
{
public:
	TcpWorker()
	{
		m_listen = False;
		m_port =  0;
		m_cnt =  0;
		m_talker =  NULL;
	}
	  
	Void  onInit()
	{
		UShort port =  12345;
		if (getListen())
		{
			m_listener =  new  Listener(*this);
			m_listener->listen(port, 10);
			std::cout.imbue(defaultLocale);
			std::cout <<  "waiting for client to attach on port "  << port <<  std::endl
					  <<  std::flush;
			std::cout.imbue(mylocale);
		}
		else
		{
			std::cout.imbue(defaultLocale);
			std::cout <<  "connecting to server on port "  << port <<  std::endl
					  <<  std::flush;
			std::cout.imbue(mylocale);
			createTalker()->connect("127.0.0.1", 12345);
		}
		  
		std::cout << std::endl << std::flush;
	}

	Void  onQuit()
	{
	}
	
	Void  onClose()
	{
		if (m_talker)
		{
			Talker *t = m_talker;
			m_talker =  NULL;
			delete t;
			quit();
		}
	}
	
	Void  errorHandler(EError  &err, ESocket::BasePrivate  *psocket)
	{
		//std::cout << "Socket exception - " << err << std::endl << std::flush;
	}	  

	Talker  *createTalker()
	{
		return m_talker =  new  Talker(*this);
	}	  
	
	Void  setListen(Bool  v) { m_listen = v; }
	Bool  getListen() { return m_listen; }
	  
	Void  setCount(Int  cnt) { m_cnt = cnt; }
	Int  getCnt() { return m_cnt; }
	  
	Void  setPort(UShort  port) { m_port = port; }
	UShort  getPort() { return m_port; }
  
private:
	Bool m_listen;
	UShort m_port;
	Int m_cnt;
	Listener *m_listener;
	Talker *m_talker;
};
```

<a  name="feature-overview-socket-communications-tcp"></a>
### TCP
TCP is a streaming connection based protocol. As such, a TCP application will either act as a client (connects to a server) or as a server (accepts connections from clients). The [ESocket::TCP](@ref ESocket::TCP) namespace contains the class definitions used by the client and server applications.

A server application listens for incoming connections using a class derived from [TCP::Listener](@ref ESocket::TCP::Listener). When the listener detects an incoming connection, the [TCP::Listener::createSocket()](@ref ESocket::TCP::Listener::createSocket) method will be called to create an instance of a class derived from [TCP::Talker](@ref ESocket::TCP::Talker) that will be used to received data from and send data to the client.

Conversely, a client application initiates a connection to a server by calling the [connect()](@ref ESocket::TCP::Talker::connect) method with the IP address, IPv4 or IPv6, and port of the server. When the connection is complete, the [onConnect()](@ref ESocket::TCP::Talker::onConnect) method of the socket object will be called by the dispatcher indicating that the connection is up and communication can proceed.
  
**[TCP::Listener](@ref ESocket::TCP::Listener) Events**
| Event | Callback Method | Description |
| ------- | -------------------- | :----------- |
| Read |[TCP::Talker::onConnect](@ref ESocket::TCP::Talker::onConnect) | A listening socket will indicate that it can be read from when a new client connects. The internals will create a new [TCP::Talker](@ref ESocket::TCP::Talker) object to handle the new connection by calling the [TCP::Listener::createSocket()](@ref ESocket::TCP::Listener::createSocket) method. Once the talking object has been created, the dispatcher will then call the [TCP::Talker::onConnect()](@ref ESocket::TCP::Talker::onConnect) method to start communication with the new client. |
| Write | None | This event is not applicable to a listening socket. |
| Error | [TCP::Listener::onError](@ref ESocket::TCP::Listener::onError) | Indicates that an error has occurred on the socket while listening for new connections. |

**Example TCP Listener**
```cpp
class  Listener : public  ESocket::TCP::ListenerPrivate
{
public:
	Listener(TcpWorker  &thread) : ESocket::TCP::ListenerPrivate(thread) {}
	virtual  ~Listener() {}
	  
	ESocket::TCP::TalkerPrivate  *createSocket(ESocket::ThreadPrivate  &thread)
	{
		return ((TcpWorker &)thread).createTalker();
	}
	  
	Void  onClose()
	{
		std::cout << "listening socket closed"  <<  std::endl <<  std::flush;
	}
	
	Void  onError()
	{
		std::cout <<  "socket error "  <<  getError()
			<< " occurred on listening socket during select" << std::endl << std::flush;
	}
	
private:
	Listener();
};
```

**[TCP::Talker](@ref ESocket::TCP::Listener) Events**
| Event | Callback Method | Description |
| ------- | -------------------- | :----------- |
| Read | [TCP::Talker::onReceive](@ref ESocket::TCP::Talker::onReceive)<br>[TCP::Talker::onClose](@ref ESocket::TCP::Talker::onClose) | When a talking socket indicates that it can be read, the framework calls `recv()` to read any pending data. If zero bytes are read, the socket has been closed and the dispatcher will call the [TCP::Talker::onClose()](@ref ESocket::TCP::Talker::onClose) method. If more than zero bytes are read, the data is inserted it into an internal receive buffer and the dispatcher will call the [TCP::Talker::onReceive()](@ref ESocket::TCP::Talker::onReceive) method allowing the application to process the data that has been read. |
| Write | None | The framework processes the write event by attempting to send any unsent data to the peer. No application interaction is required to process this event. |
| Error | [TCP::Talker::onError](@ref ESocket::TCP::Talker::onError) | Indicates that an error has occurred on the socket.

**Example TCP Talker**
```cpp
class  Talker : public  ESocket::TCP::TalkerPrivate
{
public:
	Talker(TcpWorker  &thread) : ESocket::TCP::TalkerPrivate(thread) {}
	~Talker() {}
	  
	Void  onConnect()
	{
		ESocket::TCP::TalkerPrivate::onConnect();
		  
		if (((TcpWorker &)getThread()).getListen())
		{
			std::cout <<  "Talker::onConnect() - server connected"  <<  std::endl
					  <<  std::flush;
			  
			EString localIpAddr =  getLocalAddress();
			UShort localPort =  getLocalPort();
			EString remoteIpAddr =  getRemoteAddress();
			UShort remotePort =  getRemotePort();
			  
			std::cout.imbue(defaultLocale);
			std::cout << "socket connected"
					  << " localIp=" << localIpAddr << " localPort=" << localPort
					  << " remoteIp=" << remoteIpAddr << " remotePort=" << remotePort
					  << std::endl
					  << std::flush;
			std::cout.imbue(mylocale);
		}
		else
		{
			std::cout <<  "Talker::onConnect() - client connected"  <<  std::endl
					  <<  std::flush;
			  
			EString localIpAddr =  getLocalAddress();
			UShort localPort =  getLocalPort();
			EString remoteIpAddr =  getRemoteAddress();
			UShort remotePort =  getRemotePort();
			  
			std::cout.imbue(defaultLocale);
			std::cout << "socket connected"
					  << " localIp=" << localIpAddr << " localPort=" << localPort
					  << " remoteIp=" << remoteIpAddr << " remotePort=" << remotePort
					  << std::endl
					  << std::flush;
			std::cout.imbue(mylocale);
			  
			try
			{
				Int val =  1;
				write((pUChar)&val, sizeof(val));
			}
			catch (const  ESocket::TcpTalkerError_SendingPacket &e)
			{
				std::cerr << e.what() << '\n' std::flush;
				getThread().quit();
			}
		}
	}
	
	Void  onReceive()
	{
		UChar buffer[1024];
		Int *pval = (Int *)buffer;
		  
		try
		{
			while (true)
			{
				if (bytesPending() <  4  ||  read(buffer, 4) !=  4)
				break;
				  
				if (((TcpWorker &)getThread()).getListen())
				{
					if ((*pval) %  10000  ==  1)
						std::cout << "\r" << *pval -  1 << std::flush;
				}
				else
				{
					if ((*pval) %  10000  ==  0)
						std::cout << "\r" << *pval << std::flush;
				}

				if (*pval !=  -1)
				{
					*pval = (((TcpWorker &)getThread()).getCnt() >  0  &&  *pval >= ((TcpWorker &)getThread()).getCnt()) ?  -1  : (*pval +  1);
					write(buffer, 4);
				}
				  
				if (*pval ==  -1)
				{
					if (((TcpWorker &)getThread()).getListen())
						disconnect();
					break;
				}
			}
		}
		catch (const  ESocket::TcpTalkerError_SendingPacket &e)
		{
			std::cerr << e.what() << '\n' << std::flush;
			getThread().quit();
		}
		catch (const  std::exception &e)
		{
			std::cerr << e.what() << '\n' << std::flush;
			getThread().quit();
		}
	}
	
	Void  onClose()
	{
		std::cout << std::endl << "socket closed" <<  std::endl << std::flush;
		((TcpWorker &)getThread()).onClose();
	}  

private:
	Talker();
};
```

<a  name="feature-overview-socket-communications-udp"></a>
### UDP
UDP is a connectionless message based protocol.  Messages are sent to specific IP/ports and received from a specific IP/port.  A [UDP](@ref ESocket::UDP) socket must be bound to a local port and IP address to receive messages.

**Example UDP Thread**
```cpp
class  UdpWorker : public  ESocket::ThreadPrivate
{
public:
	// In this example, one message will be sent to the peer every second.
	UdpWorker()
	{
		m_localport =  0;
		m_remoteport =  0;
		m_cnt =  0;
		m_socket =  NULL;
	}
	  
	Void  onInit()
	{
		std::cout << "creating local UDP socket" << std::endl << std::flush;
		m_socket = new UdpSocket(*this);
		  
		std::cout.imbue(defaultLocale);
		std::cout << "binding to IP [" << getLocalIp()
			<< "] port [" << getLocalPort() << "]" << std::endl << std::flush;
		std::cout.imbue(mylocale);
		m_socket->bind( getLocalIp(), getLocalPort() );
		  
		ESocket::Address remote( getRemoteIp(), getRemotePort() );
		m_socket->setRemote( remote );
		m_socket->setCount( m_cnt );
		  
		std::cout << "starting the periodic timer" << std::endl << std::endl << std::flush;
		m_timer.setInterval(1000);
		m_timer.setOneShot(False);
		initTimer(m_timer);
		m_timer.start();
	}
	
	Void  onQuit()
	{
		delete m_socket;
	}
	
	Void  errorHandler(EError  &err, ESocket::BasePrivate  *psocket)
	{
		//std::cout << "Socket exception - " << err << std::endl << std::flush;
	}
	
	Void  setCount(Int  cnt) { m_cnt = cnt; }
	Int  getCount() { return m_cnt; }
	  
	Void  setLocalIp(cpStr  ip) { m_localip = ip; }
	EString  &getLocalIp() { return m_localip; }
	Void  setLocalPort(UShort  port) { m_localport = port; }
	UShort  getLocalPort() { return m_localport; }
	  
	Void  setRemoteIp(cpStr  ip) { m_remoteip = ip; }
	EString  &getRemoteIp() { return m_remoteip; }
	Void  setRemotePort(UShort  port) { m_remoteport = port; }
	UShort  getRemotePort() { return m_remoteport; }
	  
	Void  onTimer(EThreadEventTimer  *pTimer);
  
private:
	EString m_localip;
	UShort m_localport;
	EString m_remoteip;
	UShort m_remoteport;
	Int m_cnt;
	UdpSocket *m_socket;
	EThreadEventTimer m_timer;
};
```

**[UDP](@ref ESocket::UDP) Events**
| Event | Callback Method | Description |
| ------- | -------------------- | :----------- |
| Read | [UDP::onReceive](@ref ESocket::UDP::onReceive) | When a UDP socket indicates that it can be read, the framework calls `recvfrom()` to read any pending data. If more than zero bytes are read, the data is inserted it into an internal receive buffer and the dispatcher will call the [UDP::onReceive()](@ref ESocket::UDP::onReceive) method allowing the application to process the data that has been read. |
| Write | None | The framework processes the write event by sending any unsent messages to the destination. No application interaction is required to process this event. |
| Error | [UDP::onError](@ref ESocket::UDP::onError) | Indicates that an error has occurred on the socket. |

**Example UDP Socket**
```cpp
class  UdpSocket : public  ESocket::UdpPrivate
{
public:
	UdpSocket(UdpWorker  &thread) : ESocket::UdpPrivate(thread)
	{
		m_cnt =  0;
		m_sentcnt =  0;
	}
	  
	virtual  ~UdpSocket() {}
	  
	Void  onReceive(const ESocket::Address &from, pVoid msg, Int len)
	{
		std::cout.imbue(defaultLocale);
		std::cout << ETime::Now().Format("%Y-%m-%dT%H:%M:%S.%0",True)
			<< " Received [" << *(Int*)pData << "] length [" << length << "] from ["
			<< addr.getAddress() << ":" << addr.getPort() << "]" << std::endl << std::flush;
		std::cout.imbue(mylocale);
		  
		if (*(Int*)pData ==  -1)
		{
			if (m_sentcnt !=  -1)
			{
				m_sentcnt =  -1;
				sendpacket();
			}
			getThread().quit();
		}
	}
	
	Void  onError()
	{
		std::cout <<  "socket error "  <<  getError()
			<<  " occurred on UDP socket during select"  <<  std::endl <<  std::flush;
	}
		  
	Void  sendpacket()
	{
		if (m_sentcnt !=  -1)
		{
			if (m_sentcnt < m_cnt)
				m_sentcnt++;
			else
				m_sentcnt =  -1;
		}
		write( m_remote, &m_sentcnt, sizeof(m_sentcnt) );
	}
	  
	Void  setCount(Int  cnt) { m_cnt = cnt; }
	Int  getSentCount() { return m_sentcnt; }
	  
	Void  setRemote(const  ESocket::Address  addr) { m_remote = addr; }
  
private:
	Int m_cnt;
	Int m_sentcnt;
	ESocket::Address m_remote;
};
```

<a  name="feature-overview-logging"></a>
## Logging
Logging in ***EpcTools*** is performed by the [ELogger](@ref ELogger)  class and configured in the JSON configuration file or in code.  Multiple logs can be defined with each log identified by a unique integer identifier (e.g. 1, 2, 99).  Each log can have it's messages written to one or more destinations (sinks).  The log messages are written by a background thread.  The number of threads writing the log messages and the queue size for each thread are defined in the **Logger** configuration. 

<a  name="feature-overview-logging-configuration"></a>
### Configuration

<a  name="feature-overview-logging-configuration-json"></a>
#### JSON
The JSON logger configuration is defined in the **Logger** configuration section of the **EpcTools** configuration file.
**Example JSON Logger Configuration**
```json
"EpcTools": {
	"EnablePublicObjects": false,
	"Debug": false,
	"SynchronizationObjects": {
		"NumberSemaphores": 100,
		"NumberMutexes": 100
	},
	"Logger": {
		"ApplicationName": "epc_app",
		"QueueSize": 8192,
		"NumberThreads": 1,
		"SinkSets": [
			{
				"SinkID": 1,
				"Sinks": [
					{
						"SinkType": "syslog",
						"LogLevel": "startup",
						"Pattern": "[__APPNAME__] [%n] [%l] %v"
					},
					{
						"SinkType": "stdout",
						"LogLevel": "debug",
						"Pattern": "[%Y-%m-%dT%H:%M:%S.%e] [stdout] [%^__APPNAME__%$] [%n] [%^%l%$] %v"
					},
					{
						"SinkType": "stderr",
						"LogLevel": "minor",
						"Pattern": "[%Y-%m-%dT%H:%M:%S.%e] [stderr] [%^__APPNAME__%$] [%n] [%^%l%$] %v"
					},
					{
						"SinkType": "basic_file",
						"LogLevel": "debug",
						"Pattern": "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v",
						"FileName": "./logs/epctest.basic.log",
						"Truncate": false
					},
					{
						"SinkType": "rotating_file",
						"LogLevel": "debug",
						"Pattern": "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v",
						"FileName": "./logs/epctest.rotating.log",
						"MaxSizeMB": 1,
						"MaxNumberFiles": 2,
						"RotateOnOpen": false
					},
					{
						"SinkType": "daily_file",
						"LogLevel": "debug",
						"Pattern": "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v",
						"FileName": "./logs/epctest.daily.log",
						"Truncate": false,
						"RolloverHour": 14,
						"RolloverMinute": 55
					}
				]
			},
			{
				"SinkID": 2,
				"Sinks": [
					{
						"SinkType": "rotating_file",
						"LogLevel": "debug",
						"Pattern": "%v",
						"FileName": "./logs/epctest.stats.rotating.log",
						"MaxSizeMB": 1,
						"MaxNumberFiles": 2
					}
				]
			}
		],
		"Logs": [
			{
				"LogID": 1,
				"Category": "system",
				"SinkID": 1,
				"LogLevel": "debug"
			},
			{
				"LogID": 2,
				"Category": "test1",
				"SinkID": 1,
				"LogLevel": "debug"
			},
			{
				"LogID": 3,
				"Category": "stats",
				"SinkID": 2,
				"LogLevel": "debug"
			}
		]
	}
}
```

**JSON Configuration Elements**
| Event | Callback Method | Description |
| ------- | -------------------- | :----------- |
| Read | [UDP::onReceive](@ref ESocket::UDP::onReceive) | When a UDP socket indicates that it can be read, the framework calls `recvfrom()` to read any pending data. If more than zero bytes are read, the data is inserted it into an internal receive buffer and the dispatcher will call the [UDP::onReceive()](@ref ESocket::UDP::onReceive) method allowing the application to process the data that has been read. |
| Write | None | The framework processes the write event by sending any unsent messages to the destination. No application interaction is required to process this event. |
| Error | [UDP::onError](@ref ESocket::UDP::onError) | Indicates that an error has occurred on the socket. |

<a  name="feature-overview-logging-configuration-code"></a>
#### Code

The following code configures [ELogger](@ref ELogger) the same as the [JSON](#feature-overview-logging-configuration-json) configuration example.

```cpp
#define LOG_SINKSET_GENERAL 1
#define LOG_SINKSET_STATS 2

#define LOG_SYSTEM 1
#define LOG_TEST1 2
#define LOG_STATS 3
  
...

std::shared_ptr<ELoggerSink> sp1 = std::make_shared<ELoggerSinkSyslog>(
	ELogger::eStartup, "[__APPNAME__] [%n] [%l] %v" );
std::shared_ptr<ELoggerSink> sp2 = std::make_shared<ELoggerSinkStdout>(
	ELogger::eDebug, "[%Y-%m-%dT%H:%M:%S.%e] [stdout] [%^__APPNAME__%$] [%n] [%^%l%$] %v" );
std::shared_ptr<ELoggerSink> sp3 = std::make_shared<ELoggerSinkStderr>(
	ELogger::eMinor, "[%Y-%m-%dT%H:%M:%S.%e] [stderr] [%^__APPNAME__%$] [%n] [%^%l%$] %v" );
std::shared_ptr<ELoggerSink> sp4 = std::make_shared<ELoggerSinkBasicFile>(
	ELogger::eDebug, "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v",
	"./logs/epctest.basic.log", False );
std::shared_ptr<ELoggerSink> sp5 = std::make_shared<ELoggerSinkRotatingFile>(
	ELogger::eDebug, "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v",
	"./logs/epctest.rotating.log", 10, 2, false );
std::shared_ptr<ELoggerSink> sp6 = std::make_shared<ELoggerSinkDailyFile>(
	ELogger::eDebug, "[%Y-%m-%dT%H:%M:%S.%e] [%^__APPNAME__%$] [%n] [%^%l%$] %v",
	"./logs/epctest.daily.log", false, 14, 55 );
std::shared_ptr<ELoggerSink> sp7 = std::make_shared<ELoggerSinkRotatingFile>(
	ELogger::eDebug, "%v",
	"./logs/epctest.stats.rotating.log", 10, 2, false );

ELogger::createSinkSet( LOG_SINKSET_GENERAL );
ELogger::sinkSet(LOG_SINKSET_GENERAL).addSink( sp1 );
ELogger::sinkSet(LOG_SINKSET_GENERAL).addSink( sp2 );
ELogger::sinkSet(LOG_SINKSET_GENERAL).addSink( sp3 );
ELogger::sinkSet(LOG_SINKSET_GENERAL).addSink( sp4 );
ELogger::sinkSet(LOG_SINKSET_GENERAL).addSink( sp5 );
ELogger::sinkSet(LOG_SINKSET_GENERAL).addSink( sp6 );

ELogger::createSinkSet( LOG_SINKSET_STATS );
ELogger::sinkSet(LOG_SINKSET_STATS).addSink( sp7 );

ELogger::createLog( LOG_SYSTEM, "system", LOG_SINKSET_GENERAL );
ELogger::createLog( LOG_TEST1, "test1", LOG_SINKSET_GENERAL );
ELogger::createLog( LOG_STATS, "stats", LOG_SINKSET_STATS );

ELogger::log(LOG_SYSTEM).setLogLevel( ELogger::eDebug );
ELogger::log(LOG_TEST1).setLogLevel( ELogger::eDebug );
ELogger::log(LOG_STATS).setLogLevel( ELogger::eDebug );
```

<a  name="feature-overview-logging-configuration-sinksets"></a>
#### Sink Sets
A sink set defines a set of outputs or sinks.  Once defined, a sink set is then associated with a log.  If the same sink set is associated with more than one log, then each of the associated logs will have their log messages written to the same sinks.

**Sink Types**
| Class | Parameters | Description |
| ------- | ----- | -------------------- | :----------- |
| [ELoggerSinkSyslog](@ref ELoggerSinkSyslog) | loglevel<br>pattern | Message will be written to syslog. |
| [ELoggerSinkStdout](@ref ELoggerSinkStdout) | loglevel<br>pattern | Message will be written to the standard output (stdout) file handle. |
| [ELoggerSinkStderr](@ref ELoggerSinkStderr) | loglevel<br>pattern | Message will be written to the standard error (stderr) file handle. |
| [ELoggerSinkBasicFile](@ref ELoggerSinkBasicFile) | loglevel<br>pattern<br>filename<br>truncate | Message will be written to a file. If the **truncate** flag is true, then the file will be truncated at startup. |
| [ELoggerSinkRotatingFile](@ref ELoggerSinkRotatingFile) | loglevel<br>pattern<br>filename<br>max_size_mb<br>max_files<br>rotate_on_open | Message will be written to a series of files.  When a file reaches the maximum file size defined by **max_size_mb**, the next file in the series will be opened and written to. ELogger will keep a maximum of **max_files** previous log files (including the current file).|
| [ELoggerSinkDailyFile](@ref ELoggerSinkDailyFile) | loglevel<br>pattern<br>filename<br>truncate<br>rollover_hour<br>rollover_minute | Message will be written to a file.  When the hour and minute specified by **rollover_hour** and **rollover_minute** are reached, the current log file will be closed and a new log file will be opened.  The system date and time will be appended to the log file name. |

<a  name="feature-overview-logging-configuration-logs"></a>
#### Logs
A log definition includes a log **category**, a string that can be output with each log message, a **sink set** which defines the sinks that the log messages will be written to, a **log identifier** used to identify the log when writing a log message and the minimum **log level** at which log messages will be written to associated sink set sinks.

<a  name="feature-overview-logging-configuration-loglevels"></a>
#### Log Levels
When a log message is written a **log level** is included that represents the severity or importance of the log message.  Each **sink** and **log** also has a log level associated with it.  These log levels represent the minimum log level that will be output to the **sink** or **log**.

**Log Levels in Ascending Order of Importance**
| Log Level | Enumeration |
| --------- | ----------- |
| Debug | [ELogger::eDebug](@ref ELogger::eDebug) |
| Informational | [ELogger::eInfo](@ref ELogger::eInfo) |
| Startup | [ELogger::eStartup](@ref ELogger::eStartup) |
| Minor | [ELogger::eMinor](@ref ELogger::eMinor) |
| Major | [ELogger::eMajor](@ref ELogger::eMajor) |
| Critical | [ELogger::eCritical](@ref ELogger::eCritical) |
| Off (suppresses all messages) | [ELogger::eOff](@ref ELogger::eOff) |

<a  name="feature-overview-logging-logmessages"></a>
### Log Messages

**Examples**
```cpp

#define LOG_SYSTEM 1
#define LOG_TEST1 2
#define LOG_STATS 3

ELogger::log(LOG_SYSTEM).minor("Hello Wonderful World from the system log!!");
// ELoggerSinkSyslog
// Jan 23 18:12:54 dev epctest: [epctest] [system] [minor] Hello World from the system log!!
// ELoggerSinkStdout
// [2020-01-23T18:12:54.944] [stdout] [epctest] [system] [minor] Hello World from the system log!!
// ELoggerSinkStderr
// [2020-01-23T18:12:54.944] [stderr] [epctest] [system] [minor] Hello World from the system log!!
// ELoggerSinkBasicFile
// [2020-01-23T18:12:54.944] [epctest] [system] [minor] Hello World from the system log!!
// ELoggerSinkRotatingFile
// [2020-01-23T18:12:54.944] [epctest] [system] [minor] Hello World from the system log!!
// ELoggerSinkDailyFile
// [2020-01-23T18:12:54.944] [epctest] [system] [minor] Hello World from the system log!!

ELogger::log(LOG_TEST1).startup("Hello {} from the test1 log!!", "World");
// ELoggerSinkSyslog
// Jan 23 18:12:54 dev epctest: [epctest] [test1] [startup] Hello World from the test1 log!!
// ELoggerSinkStdout
// [2020-01-23T18:12:54.944] [stdout] [epctest] [test1] [startup] Hello World from the test1 log!!
// ELoggerSinkStderr
// [2020-01-23T18:12:54.944] [stderr] [epctest] [test1] [startup] Hello World from the test1 log!!
// ELoggerSinkBasicFile (epctest.basic.log)
// [2020-01-23T18:12:54.944] [epctest] [test1] [startup] Hello World from the test1 log!!
// ELoggerSinkRotatingFile (epctest.rotating.log)
// [2020-01-23T18:12:54.944] [epctest] [test1] [startup] Hello World from the test1 log!!
// ELoggerSinkDailyFile (epctest.daily_2020-01-23.log)
// [2020-01-23T18:12:54.944] [epctest] [test1] [startup] Hello World from the test1 log!!

ELogger::log(LOG_STATS).minor("Hello {} from the test2 log!!", "World");
// ELoggerSinkRotatingFile (epctest.stats.rotating.log)
// Hello World from the test2 log!!
```

<a  name="feature-overview-dns"></a>
## DNS

<a  name="feature-overview-dns-cache"></a>
### DNS Cache

<a  name="feature-overview-dns-epc-node-discovery"></a>
### EPC Node Discovery

<a  name="feature-overview-dns-diameter-s-naptr"></a>
### Diameter S-NAPTR

<a  name="feature-overview-rest-server"></a>
## REST Server

<a  name="feature-overview-freediameter"></a>
## freeDiameter

<a  name="feature-overview-statistics"></a>
## Interface Statistics

<a  name="feature-overview-timer-pool"></a>
## Timer Pool

<a  name="feature-overview-miscellaneous"></a>
## Miscellaneous

<a  name="feature-overview-miscellaneous-string"></a>
### String

<a  name="feature-overview-miscellaneous-utilities"></a>
### Utilities

<a  name="feature-overview-miscellaneous-time"></a>
### Time

<a  name="feature-overview-miscellaneous-timer"></a>
### Timer

<a  name="feature-overview-miscellaneous-path"></a>
### Path

<a  name="feature-overview-miscellaneous-directory"></a>
### Directory

<a  name="feature-overview-miscellaneous-error"></a>
### Error

<a  name="feature-overview-miscellaneous-hash"></a>
### Hash

<a  name="feature-overview-miscellaneous-bzip2"></a>
### BZip2

<a  name="feature-overview-miscellaneous-circular-buffer"></a>
### Circular Buffer