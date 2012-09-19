Leprechaun is a small application for driving a headless web browser.  Basically, the way it works is that you point Leprechaun at a JavaScript source file and it will evaluate it in a browser frame.

Its commandline syntax is simply this:

    leprechaun path_to_script.js

Internally, we use Leprechaun to run our JavaScript unit tests.  The way this works is that we run a small JavaScript script which opens a new browser frame, attaches some hooks to listen for error conditions and console messages, and points it at the test.
# Compiling
Do all of this on Linux.  Leprechaun has had absolutely zero testing on Windows or OSX.

First, [Compile CEF](http://code.google.com/p/chromiumembedded/wiki/BranchesAndBuilding).  We are using the 1180 CEF3 branch.  Be sure to pay close attention to the Chromium build instructions, as there is an important step there to exclude certain test cases which will cause your build to take several times as long.

* You will have to change line 986 of net/third_party/nss/ssl/ssl3ecc.c for Chromium to build on our Ubuntu setup

  Comment out the line that reads
  
      case SEC_OID_PKCS1_SHA224_WITH_RSA_ENCRYPTION:

At the moment, the build scripts assume that you put the chrome source code at /home/cit/src/chromium

Install SCons.

    sudo apt-get install scons
    
Run SCons in the leprechaun source directory to compile.

# How it Works

Leprechaun is a relatively simple GTK+ application that uses the [Chromium Embedded Framework (CEF)](http://code.google.com/p/chromiumembedded/) to script a browser.  We point it at an Xvfb server so that you don't see the windows it creates.

## Major Moving Parts
### ChromeWindowClient
Implements CefClient, which provides callback handlers for various browser events.  We only really use this to sense console messages.

### ChromeWindowApp
ChromeWindowApp implements the CefApp interface, which basically amounts to handlers for system-wide things that can occur.  Like 
resource loading and custom URL schemes.

Most notably, the ChromeWindowApp is notified whenever a V8 JavaScript context is created.  The OnContextCreated method runs before any JavaScript in the frame, so it is where we add in the leprechaun-specific APIs that we need.

### test.js
Most of Leprechaun's inner workings actually happen in JavaScript.  When the initial page is loaded, ChromeWindowApp applies the onNewBrowser() function defined in test.js

test.js works by listening to the Chromium debugger by opening a websocket to localhost.  Through this condiut, we listen for errors, console messages, and eventually actually loads the test in question in a new browser frame.

# Future improvements
* Performance isn't as good as we'd like.  PhantomJS is much faster.
  * Is the debugger API killing us?
