Leprechaun is a small application for driving a headless Chromium browser.  Basically, the way it works is that you point Leprechaun at a JavaScript source file and it will evaluate it in a browser frame.

Its commandline syntax is simply this:

    leprechaun path_to_script.js

Internally, we use Leprechaun to run our JavaScript unit tests.  The way this works is that we run a small JavaScript script which opens a new browser frame, attaches some hooks to listen for error conditions and console messages, and points it at the test.

# Disclaimers
This is very young software.  It has a lot of rough edges just now.  We think it works, but consider yourself warned.

Secondly, this software intentionally duplicates what PhantomJS offers.  It's entirely likely that we will discontinue it if PhantomJS catches up.

# FAQ
### How is Leprechaun different from PhantomJS?
[PhantomJS](http://phantomjs.org/) is built atop QtWebKit, which is about a year and a half behind what you get in Chrome today.  It therefore lacks a bunch of useful things, like Float64Array.

It is also a lot less mature than PhantomJS.  If stability is more important than the latest browser features, PhantomJS is terrific.

Leprechaun (as of this writing) embeds Chrome 23.0.1271.18 via the thoroughly excellent [CEF](http://code.google.com/p/chromiumembedded/).  Leprechaun supports everything that modern Chrome supports.

---

# Compiling
Leprechaun has been tested to build on Linux and OSX 10.8.  No other configurations have been tested.

First, [Compile CEF](http://code.google.com/p/chromiumembedded/wiki/BranchesAndBuilding).  We are using the 1180 CEF3 branch.

At the moment, the build scripts assume that you put the chrome source code at /home/cit/src/chromium

Install SCons

    sudo apt-get install scons
    
Run SCons in the leprechaun source directory to compile.

    scons CEFDIR=path/to/chromium

More up-to-date compiling hints can be found in [COMPILING](COMPILING.md)

# How it Works

Leprechaun is a relatively simple application that uses the [Chromium Embedded Framework (CEF)](http://code.google.com/p/chromiumembedded/) to script a browser.

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
* Windows support
* Performance isn't as good as we'd like.  PhantomJS is much faster.
  * Is the debugger API killing us?
