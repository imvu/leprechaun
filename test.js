startTime = new Date();
var w = window.open("http://localhost.imvu.com/jstest/?script=common/imq/imq.test.js&headless=1");

w.onload = function() {
    console.log("frame loaded");
};

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

//window.location = "http://localhost.imvu.com/jstest/?script=common/imq/imq.test.js&headless=1";
//setTimeout(function() { leprechaun.exit(); }, 50);
