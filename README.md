Leprechaun is a small application for driving a headless Chromium browser.  Basically, the way it works is that you point Leprechaun at a JavaScript source file and it will evaluate it in a browser frame.

Its commandline syntax is simply this:

    leprechaun http://somehost/path/to/script.js

Internally, we use Leprechaun to run our JavaScript unit tests.  The way this works is that we run a small JavaScript script which opens a new browser frame, attaches some hooks to listen for error conditions and console messages, and points it at the test.

# Disclaimers
Leprechaun has a lot of rough edges right now.  We've been using it for years, and it works, but consider yourself warned.

# FAQ
### How is Leprechaun different from PhantomJS?

The long and the short of it is that we need WebGL, which the PhantomJS team are not interested in implementing.

If you don't need WebGL and you'd rather use something that is under active development, give [PhantomJS](http://phantomjs.org/) a try.  It's pretty great.

Leprechaun (as of this writing) embeds Chrome 56.0.2924.76 via the thoroughly excellent [CEF](https://bitbucket.org/chromiumembedded/cef).  Leprechaun supports everything that modern Chrome supports.

---

# Compiling

Instructions in [COMPILING](COMPILING.md)

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
* Make Chromium operate in headless mode to remove runtime dependence on Xvfb or X
