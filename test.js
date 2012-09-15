var startTime = null;
var commandIndex = 0;
var debuggerEnabledCommands = [];
var debuggerSetPauseableCommands = [];
var consoleEnabledCommands = [];
if (leprechaun.args.length !== 2) {
    console.log('Usage: run-jstest.js URL');
    console.log('got: ' + leprechaun.args.toString());
    leprechaun.exit(1);
}
function onNewBrowser(url) {
            if (startTime != null) {
                return;
            }
//console.log('building request using url ' + url);
var req = new XMLHttpRequest();
req.open('GET', 'http://localhost:24042/json', true);
req.onerror = function(err) {console.log(err);}; 
req.onabort = function(err) {console.log(err);};
req.onload = function() {
    //console.log('got response' + req.responseText);
    var response = JSON.parse(req.responseText);
    //console.log('this window:' + window.location);
    for(i =0; i < response.length; i++) {
    //console.log('opening websocket on ' + response[i].webSocketDebuggerUrl);
    //console.log('for url:' + response[i].url);
        if (response[i].webSocketDebuggerUrl == '') {
            //console.log("skipping debugger connection: null websocket");
            continue;
        } else if (response[i].url == window.location) {
            //console.log("Skipping debugger connection for test runner");
            continue;
        }
    var websocket = new WebSocket(response[i].webSocketDebuggerUrl);
    websocket.onerror = function(e) {console.log('got websocket error ' + e.toString());};
    websocket.onclose = function() {console.log('got websocket close');};
    websocket.onopen = function(evt) {
        //console.log('WebSocket open worked');
        //debuggerEnabledCommands.push(commandIndex);
        //websocket.send(JSON.stringify({id:commandIndex++, method:'Debugger.enable'}));
        consoleEnabledCommands.push(commandIndex);
        websocket.send(JSON.stringify({id:commandIndex++, method:'Console.enable'}));
        //console.log('sent debugger enable');
    };
    websocket.onmessage = function(event) {
        //console.log('got message ' + event.data);
        var data = JSON.parse(event.data);
        //var event = JSON.parse(evt);
        if (data.method === 'Debugger.paused') {
            //console.log('Got a debugger event!!!!');
            leprechaun.onError(event.params.reason, event.params.callFrames);
            return;
        } else if (data.method === 'Console.messageAdded') {
            //console.log("got console message: " + data.params.message.text);
            //console.log("checking console message: " + data.params.message.text.substr(0, 8));
            if (data.params.message.text.substr(0, 8) === 'Uncaught') {
                leprechaun.onError(data.params.message.text, data.params.message.stackTrace);
            } else {
                leprechaun.onConsoleMessage(data.params.message.text);
            }
            return;
        } else if (data.method === 'Console.messagesCleared') {
            return;
        } else if (data.method === 'Debugger.scriptParsed') {
            return;
        } else if (-1 !== consoleEnabledCommands.indexOf(data.id)) {
            w.location= 'http://localhost.imvu.com/jstest/?headless=1&script=' + window.encodeURIComponent(leprechaun.args[1]);
            startTime = new Date();
            return;
        /*
        } else if (-1 !== debuggerEnabledCommands.indexOf(data.id)) {
            //console.log('Setting debugger to pause on all exceptions');
            w.location="http://localhost.imvu.com/test.html";
            //debuggerSetPauseableCommands.push(commandIndex);
            //websocket.send(JSON.stringify({id:commandIndex++, method:'Debugger.setPauseOnExceptions', params:{state:'all'}}));
            return;
        } else if (-1 !== debuggerSetPauseableCommands.indexOf(data.id)) {
            console.log('response:   ' + event.data);
            leprechaun.echo('Debugger should pause after excpetions now');
            startTime = new Date();
            //var w = window.open("http://localhost.imvu.com/jstest/?script=common/imq/imq.test.js&headless=1");
            //w.location="http://localhost.imvu.com/test.html";

            return;
        */
        }
        console.log('Unknown event:' + event.data);
    }
    }
}
//console.log('sending json request');
req.send();
}
var w = window.open('about:blank');

leprechaun.onConsoleMessage = function onConsoleMessage(msg) {
    var passed, total, failed, endTime, diffTime;

    if (msg.indexOf('!!DONE!! ') === 0) {
        var r = JSON.parse(msg.substr(9));
        total = r.passes + r.failures;

        endTime = new Date();
        diffTime = endTime.getTime() - startTime.getTime();
        leprechaun.echo("Test Run took " + diffTime + " ms");

        leprechaun.echo('PhantomJS test runner: ' + r.passes + ' tests of ' + total + ' passed, ' + r.failures + ' failed');
        leprechaun.exit((r.failures > 0) ? 1 : 0);
    } else if (msg.indexOf('jstest ') === 0) {
        leprechaun.echo(msg.substr(7));
    } else {
        leprechaun.echo('>>> ' + msg);
    }
}

leprechaun.onError = function onError(msg, trace) {
    var i, k, fn, file;
    leprechaun.echo('Got error: ' + msg);
    for (i = 0; i < trace.length; ++i) {
        fn = trace[i].functionName || '<anonymous>';
        file = trace[i].url;
        leprechaun.echo("    at " + fn + " (" + file + ":" + trace[i].lineNumber + ")");
    }
    leprechaun.exit(1);
}

window.setTimeout(function () {
    console.log('Your unit test took longer than 15 seconds to complete. It\'s possible that this test either is absurdly slow, attempted to access the network, or was unable to load a dependency.');
    leprechaun.exit(1);
}, 15000);
