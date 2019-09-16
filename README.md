# EpcTools - Enhanced Packet Core Development Tools

EpcTools is an event based, multi-threaded C++ development framework designed for any Linux implementation that supports g++ and pthreads.

  - Type some Markdown on the left
  - See HTML in the right
  - Magic

# Dependencies

* [RapidJSON](http://rapidjson.org) - a JSON parser and generator for C++

# Installation

Clone the project, install the dependencies (via configure), build the static library and install.

```sh
$ git clone https://github.com/brianwaters3/FoundationTools.git epctools
$ cd epctools
$ ./configure
$ make
$ sudo make install
```

# Features
EpcTools includes classes for:

  - Threading - Basic, Event, Event thread with TCP socket capabilities
  - Synchronization - Mutex using native IPC (pthread) or custom mutex implementation and an optimized Semaphore implementation
  - Custom queue implementation with message encoding and decoding
  - Shared Memory configuration and access
  - Ability to enable or disable public variants of threads, mutexes, semaphores and queues that utilize shared memory
  - Various utility classes including circular buffer, directory processing, bzip2, time, elapsed timer, string, exception and logging

## Threading

### Basic Thread

To create a basic thread, an object must be derived from `EThreadBasic` overriding the `EThreadBasic::threadProc()` method which represents the code that will be executed in a separate thread.  To initialize and start the thread simply call the `Void init(pVoid arg, Dword stackSize = 0)` method.  Call `Void join()` to wait for the thread to exit.  Other useful `EThreadBasic` methods include `static Void sleep(Int milliseconds)` and `static Void yield()`.

```cpp
class EThreadBasicTest : public EThreadBasic
{
public:
   EThreadBasicTest() : m_timetoquit(false) {}

   Dword threadProc(Void *arg) {
      while (!m_timetoquit) {
         cout << "Inside the thread [" << (cpStr)arg << "]" << endl;
         sleep(1000);
      }
      cout << "Exiting EThreadTest::threadProc()" << endl;
      return 0;
   }

   Void setTimeToQuit() {
      m_timetoquit = true;
   }

private:
   bool m_timetoquit;
};

Void EThreadBasic_test() {
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
### Event Thread

### Socket Thread (TCP)

## Synchronization

### Mutex

### Semaphore

## Queue

### Configuration

### Encode/Decode

## Shared Memory

## Configuration

## Exceptions

## Logger



### Tech

Dillinger uses a number of open source projects to work properly:

* [AngularJS] - HTML enhanced for web apps!
* [Ace Editor] - awesome web-based text editor
* [markdown-it] - Markdown parser done right. Fast and easy to extend.
* [Twitter Bootstrap] - great UI boilerplate for modern web apps
* [node.js] - evented I/O for the backend
* [Express] - fast node.js network app framework [@tjholowaychuk]
* [Gulp] - the streaming build system
* [Breakdance](http://breakdance.io) - HTML to Markdown converter
* [jQuery] - duh

And of course Dillinger itself is open source with a [public repository][dill]
 on GitHub.

### Installation

Dillinger requires [Node.js](https://nodejs.org/) v4+ to run.

Install the dependencies and devDependencies and start the server.

```sh
$ git clone https://github.com/brianwaters3/FoundationTools.git epctools
$ cd epctools
$ ./configure
```

For production environments...

```sh
$ npm install --production
$ NODE_ENV=production node app
```

### Plugins

Dillinger is currently extended with the following plugins. Instructions on how to use them in your own application are linked below.

| Plugin | README |
| ------ | ------ |
| Dropbox | [plugins/dropbox/README.md][PlDb] |
| Github | [plugins/github/README.md][PlGh] |
| Google Drive | [plugins/googledrive/README.md][PlGd] |
| OneDrive | [plugins/onedrive/README.md][PlOd] |
| Medium | [plugins/medium/README.md][PlMe] |
| Google Analytics | [plugins/googleanalytics/README.md][PlGa] |


### Development

Want to contribute? Great!

Dillinger uses Gulp + Webpack for fast developing.
Make a change in your file and instantanously see your updates!

Open your favorite Terminal and run these commands.

First Tab:
```sh
$ node app
```

Second Tab:
```sh
$ gulp watch
```

(optional) Third:
```sh
$ karma test
```
#### Building for source
For production release:
```sh
$ gulp build --prod
```
Generating pre-built zip archives for distribution:
```sh
$ gulp build dist --prod
```
### Docker
Dillinger is very easy to install and deploy in a Docker container.

By default, the Docker will expose port 8080, so change this within the Dockerfile if necessary. When ready, simply use the Dockerfile to build the image.

```sh
cd dillinger
docker build -t joemccann/dillinger:${package.json.version} .
```
This will create the dillinger image and pull in the necessary dependencies. Be sure to swap out `${package.json.version}` with the actual version of Dillinger.

Once done, run the Docker image and map the port to whatever you wish on your host. In this example, we simply map port 8000 of the host to port 8080 of the Docker (or whatever port was exposed in the Dockerfile):

```sh
docker run -d -p 8000:8080 --restart="always" <youruser>/dillinger:${package.json.version}
```

Verify the deployment by navigating to your server address in your preferred browser.

```sh
127.0.0.1:8000
```

#### Kubernetes + Google Cloud

See [KUBERNETES.md](https://github.com/joemccann/dillinger/blob/master/KUBERNETES.md)


### Todos

 - Write MORE Tests
 - Add Night Mode

License
----

MIT


**Free Software, Hell Yeah!**

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)


   [dill]: <https://github.com/joemccann/dillinger>
   [git-repo-url]: <https://github.com/joemccann/dillinger.git>
   [john gruber]: <http://daringfireball.net>
   [df1]: <http://daringfireball.net/projects/markdown/>
   [markdown-it]: <https://github.com/markdown-it/markdown-it>
   [Ace Editor]: <http://ace.ajax.org>
   [node.js]: <http://nodejs.org>
   [Twitter Bootstrap]: <http://twitter.github.com/bootstrap/>
   [jQuery]: <http://jquery.com>
   [@tjholowaychuk]: <http://twitter.com/tjholowaychuk>
   [express]: <http://expressjs.com>
   [AngularJS]: <http://angularjs.org>
   [Gulp]: <http://gulpjs.com>

   [PlDb]: <https://github.com/joemccann/dillinger/tree/master/plugins/dropbox/README.md>
   [PlGh]: <https://github.com/joemccann/dillinger/tree/master/plugins/github/README.md>
   [PlGd]: <https://github.com/joemccann/dillinger/tree/master/plugins/googledrive/README.md>
   [PlOd]: <https://github.com/joemccann/dillinger/tree/master/plugins/onedrive/README.md>
   [PlMe]: <https://github.com/joemccann/dillinger/tree/master/plugins/medium/README.md>
   [PlGa]: <https://github.com/RahulHP/dillinger/blob/master/plugins/googleanalytics/README.md>
