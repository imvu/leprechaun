
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <cassert>
#include <fstream>
#include <sstream>
#include "include/cef_base.h"
#include "include/cef_client.h"


#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_runnable.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#include "include/cef_app.h"

std::string readFile(const std::string& fileName) {
    return std::string(
        std::istreambuf_iterator<char>(std::ifstream(fileName.c_str()).rdbuf()),
        std::istreambuf_iterator<char>());
}

void bootstrap(CefRefPtr<CefFrame>& frame, const std::string& fileName) {
    std::string sourceCode = readFile(fileName);
    if (sourceCode.empty()) {
        return;
    }

    frame->ExecuteJavaScript(sourceCode, fileName, 0);
}

struct ChromeWindowClient : public CefClient
                          , public CefLifeSpanHandler
                          , public CefDisplayHandler
{
    IMPLEMENT_REFCOUNTING(ChromeWindowClient);

private:
    CefRefPtr<CefCommandLine> commandLine;
public:
    ChromeWindowClient(CefRefPtr<CefCommandLine>& commandLine)
        : commandLine(commandLine)
        { }

    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() {
        return this;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() {
        return this;
    }

    // CefLifeSpanHandler

    CefRefPtr<CefBrowser> browser;

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> aBrowser) {
        if (browser.get()) {
            return;
        }

        browser = aBrowser;

        printf("onAfterCreated\n");
        if (!commandLine->HasArguments()) {
            return;
        }

        CefCommandLine::ArgumentList arguments;
        commandLine->GetArguments(arguments);

        //
    }

    // CefDisplayHandler

    virtual bool OnConsoleMessage(
        CefRefPtr<CefBrowser> browser,
        const CefString& message,
        const CefString& source,
        int line
    ) {
        printf("OnConsoleMessage %S\n", message.ToWString().c_str());

        return false;
    }
};

struct ChromeWindowApp : public CefApp
                       , public CefRenderProcessHandler
                       , public CefV8Handler
{
    IMPLEMENT_REFCOUNTING(ChromeWindowApp);
    CefRefPtr<CefBrowser> firstBrowser;
    CefRefPtr<CefV8Value> leprechaunObj;

private:
    CefRefPtr<CefCommandLine> commandLine;
public:
    ChromeWindowApp(CefRefPtr<CefCommandLine>& commandLine)
        : commandLine(commandLine)
    { }

    // CefApp

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() {
        return this;
    }

    // CefRenderProcessHandler

    virtual void OnContextCreated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    ) {
        printf("OnContextCreated '%S'\n", frame->GetURL().ToWString().c_str());

        CefRefPtr<CefV8Value> global = context->GetGlobal();

        if (this->firstBrowser != NULL) {
            //if (this->firstBrowser.get() != browser.get()) {
            //    printf("Skipping reconnecting to initial browser\n");
            //    return;
            //}
            printf("Second context; creating log function to allow redirect of logs\n");
            CefRefPtr<CefV8Value> console = CefV8Value::CreateObject(0);
            CefRefPtr<CefV8Value> log = CefV8Value::CreateFunction("log", this);
            console->SetValue("log", log, V8_PROPERTY_ATTRIBUTE_READONLY);
            global->SetValue("console", console, V8_PROPERTY_ATTRIBUTE_READONLY);

            // have first window debugger connect to the new browser
            this->firstBrowser->GetMainFrame()->ExecuteJavaScript("onNewBrowser();", "NewBrowser.js", 0);
            return;
        }
        printf("Primary context; using leprechaun object\n");
        this->firstBrowser = browser;

        CefCommandLine::ArgumentList arguments;
        commandLine->GetArguments(arguments);

        this->leprechaunObj = CefV8Value::CreateObject(0);
        CefRefPtr<CefV8Value> exit = CefV8Value::CreateFunction("exit", this);
        this->leprechaunObj->SetValue("exit", exit, V8_PROPERTY_ATTRIBUTE_READONLY);
        CefRefPtr<CefV8Value> echo = CefV8Value::CreateFunction("echo", this);
        this->leprechaunObj->SetValue("echo", echo, V8_PROPERTY_ATTRIBUTE_READONLY);
        CefRefPtr<CefV8Value> open = CefV8Value::CreateFunction("open", this);
        this->leprechaunObj->SetValue("open", open, V8_PROPERTY_ATTRIBUTE_READONLY);
        CefRefPtr<CefV8Value> args = CefV8Value::CreateArray(arguments.size());
        for (int i; i < arguments.size(); ++i) {
            args->SetValue(i, CefV8Value::CreateString(arguments.at(i)));
        }
        this->leprechaunObj->SetValue("args", args, V8_PROPERTY_ATTRIBUTE_READONLY);

        CefRefPtr<CefV8Value> onConsoleMessage = CefV8Value::CreateFunction("onConsoleMessage", this);
        this->leprechaunObj->SetValue("onConsoleMessage", onConsoleMessage, V8_PROPERTY_ATTRIBUTE_NONE);
        CefRefPtr<CefV8Value> onError = CefV8Value::CreateFunction("onError", this);
        this->leprechaunObj->SetValue("onError", onError, V8_PROPERTY_ATTRIBUTE_NONE);
        CefRefPtr<CefV8Value> log = CefV8Value::CreateFunction("log", this);
        this->leprechaunObj->SetValue("log", log, V8_PROPERTY_ATTRIBUTE_READONLY);

        global->SetValue("leprechaun", this->leprechaunObj, V8_PROPERTY_ATTRIBUTE_READONLY);

        // Launch a websocket that will connect to the remote debugger to transfer back stack traces
        //std::string exceptionJs = readFile("../error.js");
        //frame->ExecuteJavaScript(exceptionJs.c_str(), "EnableStackTraces.js", 0);

        bootstrap(frame, arguments.at(0));
    }

    virtual void OnContextReleased(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    ) {
    }

    virtual void OnWebKitInitialized() {
    }

    // CefV8Handler

    virtual bool Execute(
        const CefString& name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval,
        CefString& exception
    ) {
        if (name == "exit") {
            printf("Exiting with value %d!\n", arguments[0]->GetIntValue());
            this->firstBrowser = NULL;
            // see http://www.magpcss.org/ceforum/viewtopic.php?f=6&t=891 for the 'right' way to do this
            bool result = CefPostTask(
                TID_UI,
                NewCefRunnableFunction(&exit, arguments[0]->GetIntValue())
            );
            return true;

        } else if (name == "open") {

        } else if (name == "log") {
            if (object->IsSame(this->leprechaunObj)) {
                // Figure out if we need this some day...
                return true;
            }
            std::wstringstream ss;
            for (size_t i = 0; i < arguments.size(); ++i) {
                ss << arguments[i]->GetStringValue().ToWString();
            }
            CefRefPtr<CefV8Value> onConsoleMessageFunc = this->leprechaunObj->GetValue("onConsoleMessage");
            // Right now we're passing the original arguments; should we be passing the single string instead?
            onConsoleMessageFunc->ExecuteFunction(this->leprechaunObj, arguments);
            return true;
        } else if (name == "echo") {
            std::wstringstream ss;
            for (size_t i = 0; i < arguments.size(); ++i) {
                ss << arguments[i]->GetStringValue().ToWString();
            }
            printf("%S\n", ss.str().c_str());
            return true;
        } else if (name == "onerror") {
            //CefRefPtr<CefV8Value> onErrorFunc = this->leprechaunObj->GetValue("onError");
            //onErrorFunc->ExecuteFunction();
            return true;
        }

        return false;
    }
};

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");

    gtk_init(&argc, &argv);

    CefRefPtr<CefCommandLine> commandLine(CefCommandLine::CreateCommandLine());
    commandLine->InitFromArgv(argc, argv);

    printf("Commandline %S\n", commandLine->GetCommandLineString().ToWString().c_str());

    // FIXME: Chromium launches a zygote, even though we're in single-process mode.
    // It's not clear to me what this accomplishes, but for now, we just roll with it.
    // -- andy 7 September 2012
    if (!commandLine->HasSwitch("type") && argc < 2) {
        printf("Syntax: %s filename.js\n", argv[0]);
        return 1;
    }

    CefRefPtr<CefApp> app(new ChromeWindowApp(commandLine));
    CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient(commandLine));

    CefMainArgs args(argc, argv);
    CefSettings appSettings;
    appSettings.remote_debugging_port = 24042;
    //appSettings.log_severity = LOGSEVERITY_VERBOSE;
    appSettings.multi_threaded_message_loop = false;
    appSettings.single_process = true;
    CefInitialize(args, appSettings, app);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_widget_destroyed), &window);
    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(CefQuitMessageLoop), NULL);

    CefWindowInfo info;
    info.SetAsChild(window);

    CefBrowserSettings settings;
    settings.web_security_disabled = true;

    CefBrowser* browser = CefBrowserHost::CreateBrowserSync(
        info, client.get(),
        "data:text/html,<!DOCTYPE html><html><head></head><body></body><script>void 0;</script></html>",
        settings
    );

    gtk_widget_show_all(GTK_WIDGET(window));

    CefRunMessageLoop();
    CefShutdown();

    return 0;
}
