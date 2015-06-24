Leprechaun is a small application for driving a headless Chromium browser.  Basically, the way it works is that you point Leprechaun at a JavaScript source file and it will evaluate it in a browser frame.

Its commandline syntax is simply this:

    leprechaun http://somehost/path/to/script.js

Internally, we use Leprechaun to run our JavaScript unit tests.  The way this works is that we run a small JavaScript script which opens a new browser frame, attaches some hooks to listen for error conditions and console messages, and points it at the test.

# Disclaimers
This is very young software.  It has a lot of rough edges just now.  We think it works, but consider yourself warned.

# FAQ
### How is Leprechaun different from PhantomJS?
[PhantomJS](http://phantomjs.org/) is built atop QtWebKit, which is about a year and a half behind what you get in Chrome today.  It therefore lacks a bunch of useful things, like Float64Array.

It is also a lot less mature than PhantomJS.  If stability is more important than the latest browser features, PhantomJS is terrific.

Leprechaun (as of this writing) embeds Chrome 43.0.2357.81 via the thoroughly excellent [CEF](https://bitbucket.org/chromiumembedded/cef).  Leprechaun supports everything that modern Chrome supports.

---

# Compiling
Leprechaun has been tested to build on Linux Ubuntu Precise (12.04), Windows 7, and OSX 10.9.  No other configurations have been tested.

First, download the appropriate [compiled CEF binary](https://cefbuilds.com/) or [build CEF from source](https://bitbucket.org/chromiumembedded/cef/wiki/BranchesAndBuilding) using the automated build process. We are using Branch 2357. Building Leprechaun on Linux Ubuntu Precise cannot use the pre-built CEF binaries because of an incompatibility with the default libstdc++ package.
* Linux 64bit CEF 3.2357.1276
* Mac 64bit CEF 3.2357.1276
* Windows 32bit CEF 3.2357.1280

If building CEF from source (requires ~24GB of space):

    wget https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py
    python automate-git.py --download-dir=<some_temp_location> --branch=2357 --no-debug-build
    cp -r <some_temp_location>/chromium/src/cef/binary_distrib/cef_binary_3.2357.1280.<some_hash>.linux64/ <somewhere>

Otherwise, unpack the pre-built CEF binary:

    cd <somewhere>
    p7zip -d /path/to/cef/download.7z

Install cmake 3.x

    wget http://www.cmake.org/files/v3.2/cmake-3.2.3.tar.gz
    tar -xf cmake-3.2.3.tar.gz
    cd cmake-3.2.3/
    ./bootstrap
    make
    sudo make install

Install depot\_tools

    sudo apt-get install depot_tools

Build the pre-built CEF binary (yes, the pre-built CEF binaries need to be built)

    cd <somewhere>/cef_binary_3.2357.1280_<some_hash>/
    mkdir build
    cd build
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
    ninja cefclient cefsimple

Install SCons

    sudo apt-get install scons

Get Leprechaun

    cd <somewhere>
    git clone git@github.com:imvu/leprechaun.git
    cd leprechaun

Run SCons in the leprechaun source directory to compile.

    scons CEFDIR=/path/to/extracted/cef

More up-to-date compiling hints can be found in [COMPILING](COMPILING.md)

# How it Works

Leprechaun is a relatively simple application that uses the [Chromium Embedded Framework (CEF)](https://bitbucket.org/chromiumembedded/cef) to script a browser.

On Linux, we point it at an Xvfb server so that you don't see the windows it creates.

## Major Moving Parts
### WindowClient
Implements CefClient, which provides callback handlers for various browser events.  We only really use this to sense console messages.

### Application
Application implements the CefApp interface, which basically amounts to handlers for system-wide things that can occur.  Like resource loading and custom URL schemes.

Most notably, the Application is notified whenever a V8 JavaScript context is created.  The OnContextCreated method runs before any JavaScript in the frame, so it is where we add in the leprechaun-specific APIs that we need.

### test.js
Most of Leprechaun's inner workings actually happen in JavaScript.  When the initial page is loaded, ChromeWindowApp applies the onNewBrowser() function defined in test.js

test.js works by listening to the Chromium debugger by opening a websocket to localhost.  Through this condiut, we listen for errors, console messages, and eventually actually loads the test in question in a new browser frame.

# Future improvements
* Update the "Major Moving Parts" section to make it reflect the current state of the project
* Make interface compatible with PhantomJS
* Make chromium operate in headless mode to remove runtime dependence on Xvfb or X
