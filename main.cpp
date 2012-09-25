
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

int s_result = 0;

std::string readFile(const std::string& fileName) {
    return std::string(
        std::istreambuf_iterator<char>(std::ifstream(fileName.c_str()).rdbuf()),
        std::istreambuf_iterator<char>());
}

void bootstrap(const CefRefPtr<CefFrame>& frame, const std::string& fileName) {
    std::string sourceCode = readFile(fileName);
    if (sourceCode.empty()) {
        return;
    }

    frame->ExecuteJavaScript(sourceCode, fileName, 0);
}

struct ChromeWindowClient : public CefClient
                          , public CefLifeSpanHandler
                          , public CefDisplayHandler
                          , public CefRequestHandler
{
    IMPLEMENT_REFCOUNTING(ChromeWindowClient);

public:
    ChromeWindowClient()
        { }

    ~ChromeWindowClient() {printf("Shutting down client\n");}

    // CefClient

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
    }

    // CefRequestHandler
    virtual bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message
    ) {
        if (message->GetName() == "getArguments") {
            CefRefPtr<CefProcessMessage> outMessage = CefProcessMessage::Create("arguments");
            CefCommandLine::ArgumentList arguments;
            CefCommandLine::GetGlobalCommandLine()->GetArguments(arguments);
            CefRefPtr<CefListValue> argList = outMessage->GetArgumentList();
            argList->SetSize(arguments.size());
            for(int i = 0; i < arguments.size(); ++i) {
                argList->SetString(i, arguments.at(i));
            }
            browser->SendProcessMessage(PID_RENDERER, outMessage);
            return true;
        } else if (message->GetName() == "quit") {
            s_result = message->GetArgumentList()->GetInt(0);
            printf("Quitting with value %d\n", s_result);
            printf("Log:\n%S", message->GetArgumentList()->GetString(1).ToWString().c_str());
            CefPostTask(
                TID_UI,
                NewCefRunnableFunction(&CefQuitMessageLoop)
            );
        } else {
            printf("Unknown message %s\n", message->GetName().ToString().c_str());
        }
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
    std::wstring outBuffer;

public:
    ChromeWindowApp()
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
        CefRefPtr<CefV8Value> global = context->GetGlobal();

        if (this->firstBrowser != NULL) {
            // Disallow window.open

            CefRefPtr<CefV8Value> openWindow = CefV8Value::CreateFunction("openWindow", this);
            global->SetValue("open", openWindow, V8_PROPERTY_ATTRIBUTE_READONLY);

            // have first window debugger connect to the new browser
            firstBrowser->GetMainFrame()->ExecuteJavaScript("onNewBrowser();", "NewBrowser.js", 0);
            return;
        }
        this->firstBrowser = browser;

        this->leprechaunObj = CefV8Value::CreateObject(0);
        CefRefPtr<CefV8Value> exit = CefV8Value::CreateFunction("exit", this);
        this->leprechaunObj->SetValue("exit", exit, V8_PROPERTY_ATTRIBUTE_READONLY);
        CefRefPtr<CefV8Value> echo = CefV8Value::CreateFunction("echo", this);
        this->leprechaunObj->SetValue("echo", echo, V8_PROPERTY_ATTRIBUTE_READONLY);
        CefRefPtr<CefV8Value> open = CefV8Value::CreateFunction("open", this);
        this->leprechaunObj->SetValue("open", open, V8_PROPERTY_ATTRIBUTE_READONLY);

        CefRefPtr<CefV8Value> onConsoleMessage = CefV8Value::CreateFunction("onConsoleMessage", this);
        this->leprechaunObj->SetValue("onConsoleMessage", onConsoleMessage, V8_PROPERTY_ATTRIBUTE_NONE);
        CefRefPtr<CefV8Value> onError = CefV8Value::CreateFunction("onError", this);
        this->leprechaunObj->SetValue("onError", onError, V8_PROPERTY_ATTRIBUTE_NONE);
        CefRefPtr<CefV8Value> log = CefV8Value::CreateFunction("log", this);
        this->leprechaunObj->SetValue("log", log, V8_PROPERTY_ATTRIBUTE_READONLY);

        global->SetValue("leprechaun", this->leprechaunObj, V8_PROPERTY_ATTRIBUTE_READONLY);
        printf("Asking browser proc for arguments\n");
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("getArguments");
        browser->SendProcessMessage(PID_BROWSER, message);
    }

    virtual void OnContextReleased(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    ) {
    }

    virtual void OnWebKitInitialized() {
    }

    virtual bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser, 
        CefProcessId source_process, 
        CefRefPtr<CefProcessMessage> message
    ) {
        if (message->GetName() == "arguments") {
            printf("Handling arguments\n");
            CefRefPtr<CefListValue> arguments = message.get()->GetArgumentList();
            CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
            context->Enter();
            CefRefPtr<CefV8Value> args1 = CefV8Value::CreateArray(0);
            CefRefPtr<CefV8Value> args = CefV8Value::CreateArray(arguments->GetSize());
            for (int i=0; i < arguments->GetSize(); ++i) {
                args->SetValue(i, CefV8Value::CreateString(arguments->GetString(i)));
            }
            this->leprechaunObj->SetValue("args", args, V8_PROPERTY_ATTRIBUTE_READONLY);
            bootstrap(browser->GetMainFrame(), message.get()->GetArgumentList()->GetString(0));
            context->Exit();
            return true;
        } else {
            printf("Unknown render message %s\n", message->GetName().ToString().c_str());
        }
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
            CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("quit");
            message->GetArgumentList()->SetInt(0, arguments[0]->GetIntValue());
            message->GetArgumentList()->SetString(1, this->outBuffer);
            //printf("Sending message to quit, log:\n%S\n", this->outBuffer.c_str());
            if(CefV8Context::GetEnteredContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, message)) {
                printf("Got true from quit send process\n");
            } else { 
                printf("Got false from quit send process message\n");
            }
            return true;

        } else if (name == "open") {

        } else if (name == "openWindow") {
            void noOpenWindow();
            CefPostTask(
                TID_UI,
                NewCefRunnableFunction(&noOpenWindow)
            );
            return true;

        } else if (name == "nop") {
            return true;

        } else if (name == "log") {
            if (object->IsSame(this->leprechaunObj)) {
                // Figure out if we need this some day...
                return false;
            }
            std::wstringstream ss;
            for (size_t i = 0; i < arguments.size(); ++i) {
                ss << arguments[i]->GetStringValue().ToWString();
            }
            printf("Got log: %S\n", ss.str().c_str());
            //CefRefPtr<CefV8Value> onConsoleMessageFunc = this->leprechaunObj->GetValue("onConsoleMessage");
            // Right now we're passing the original arguments; should we be passing the single string instead?
            //onConsoleMessageFunc->ExecuteFunction(this->leprechaunObj, arguments);
            return true;
        } else if (name == "echo") {
            std::wstringstream ss;
            for (size_t i = 0; i < arguments.size(); ++i) {
                ss << arguments[i]->GetStringValue().ToWString();
            }
            //printf(">>>%S\n", ss.str().c_str());
            this->outBuffer += ss.str().c_str();
            this->outBuffer += L"\n";
            return true;
        } else if (name == "onerror") {
            //CefRefPtr<CefV8Value> onErrorFunc = this->leprechaunObj->GetValue("onError");
            //onErrorFunc->ExecuteFunction();
            return true;
        }

        return false;
    }
};

void noOpenWindow() {
    printf("Test tried to window.open!  This is bad!\n");
    exit(1);
}

int main(int argc, char** argv) {
    {
        setlocale(LC_ALL, "");
        CefMainArgs main_args(argc, argv);
        CefRefPtr<CefCommandLine> commandLine(CefCommandLine::CreateCommandLine());
        commandLine->InitFromArgv(main_args.argc, main_args.argv);

        CefRefPtr<CefApp> app(new ChromeWindowApp());

        // Execute the secondary process, if any.
        int exit_code = CefExecuteProcess(main_args, app.get());
        if (exit_code >= 0) {
            return exit_code;
        }
        printf("CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());

        gtk_init(&argc, &argv);


        if (argc < 2) {
            printf("Syntax: %s filename.js [args for js]\n", argv[0]);
            return 1;
        }

        CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient());

        CefMainArgs args(argc, argv);
        CefSettings appSettings;
        appSettings.remote_debugging_port = 24042;
        //appSettings.log_severity = LOGSEVERITY_VERBOSE;
        CefInitialize(main_args, appSettings, app);

        //GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        //gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

        //g_signal_connect(G_OBJECT(window), "destroy",
        //                 G_CALLBACK(gtk_widget_destroyed), &window);
        //g_signal_connect(G_OBJECT(window), "destroy",
        //                 G_CALLBACK(CefQuitMessageLoop), NULL);

        CefWindowInfo info;
        //info.SetAsChild(window);

        CefBrowserSettings settings;
        settings.web_security_disabled = true;

        CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
            info, client.get(),
            "data:text/html,<!DOCTYPE html><html><head></head><body></body><script>void 0;</script></html>",
            settings
        );

        //gtk_widget_show_all(GTK_WIDGET(window));

        CefRunMessageLoop();
    }
    CefShutdown();


    return s_result;
}
