var startTime = null;
var commandIndex = 0;
var consoleEnabledCommands = [];
var webSocketConnections = [];
if (leprechaun.args.length !== 2) {
    leprechaun.echo('Usage: run-jstest.js URL');
    leprechaun.echo('got: ' + leprechaun.args.toString());
    leprechaun.exit(1);
}
function onNewBrowser(url) {
    if (startTime != null) {
        return;
    }

    var req = new XMLHttpRequest();
    req.open('GET', 'http://localhost:24042/json', true);
    req.onerror = function(err) {leprechaun.echo(err);}; 
    req.onabort = function(err) {leprechaun.echo(err);};
    req.onload = function() {
        var response = JSON.parse(req.responseText);
        for(i =0; i < response.length; i++) {
            if (response[i].webSocketDebuggerUrl == '') {
                continue;
            } else if (response[i].url == window.location) {
                continue;
            }
            var websocket = new WebSocket(response[i].webSocketDebuggerUrl);
            webSocketConnections.push(websocket);
            websocket.onerror = function(e) {
                leprechaun.echo('got websocket error ' + e.toString());
            };
            websocket.onclose = function() {
                leprechaun.echo('got websocket close');
            };
            websocket.onopen = function(evt) {
                consoleEnabledCommands.push(commandIndex);
                websocket.send(JSON.stringify({id:commandIndex++, method:'Console.enable'}));
            };
            websocket.onmessage = function(event) {
                var data = JSON.parse(event.data);
                if (data.method === 'Console.messageAdded') {
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
                    window.setTimeout(function () {
                        leprechaun.echo('Your unit test took longer than 30 seconds to complete.' +
                                        ' It\'s possible that this test either is absurdly slow, '+
                                        'attempted to access the network, or was unable to load a dependency.');
                        leprechaun.exit(1);
                    }, 30000);
                    return;
                }
                leprechaun.echo('Unknown event:' + event.data);
            };
        }
    };
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
};

leprechaun.onError = function onError(msg, trace) {
    var i, k, fn, file;
    leprechaun.echo('Got error: ' + msg);
    for (i = 0; i < trace.length; ++i) {
        fn = trace[i].functionName || '<anonymous>';
        file = trace[i].url;
        leprechaun.echo("    at " + fn + " (" + file + ":" + trace[i].lineNumber + ")");
    }
    leprechaun.exit(1);
};

window.setTimeout(function () {
    leprechaun.echo('Leprechaun took longer than 45s to start.  There is probably a problem in run_jstest.js' +
                    ' or your system configuration.');
    leprechaun.exit(1);
}, 45000);
