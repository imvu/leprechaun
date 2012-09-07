
#include <stdio.h>
#include <gtk/gtk.h>
#include <cassert>
#include <iostream>
#include "include/cef_base.h"
#include "include/cef_client.h"


#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_runnable.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#include "include/cef_app.h"

struct ChromeWindowClient : public CefClient
                          , public CefLifeSpanHandler
                          , public CefDisplayHandler
{
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() {
        return this;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() {
        return this;
    }

    // CefLifeSpanHandler

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> aBrowser) {
        if (browser.get()) {
            return;
        }

        browser = aBrowser;
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

    CefRefPtr<CefBrowser> browser;

    IMPLEMENT_REFCOUNTING(ChromeWindowClient);
};

std::string readFile(const std::string& fileName) {
    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file) {
        printf("Unable to read %s\n", fileName.c_str());
        return std::string();
    }

    fseek(file, 0, SEEK_END);
    const size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* const contents = new char[len + 1];
    const size_t readLen = fread(contents, 1, len, file);
    if (len != readLen) {
        printf("Failed to read %s\n", fileName.c_str());
        delete[] contents;
        return std::string();
    }

    fclose(file);

    contents[len] = 0;

    std::string s(contents);
    delete[] contents;
    return s;
}

struct ChromeWindowApp : public CefApp
                       , public CefRenderProcessHandler
                       , public CefV8Handler
{
    IMPLEMENT_REFCOUNTING(ChromeWindowApp);

private:
    CefRefPtr<CefCommandLine> commandLine;

public:
    ChromeWindowApp(CefRefPtr<CefCommandLine>& commandLine)
        : commandLine(commandLine)
    {
    }

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
        if (!commandLine->HasArguments()) {
            return;
        }

        CefCommandLine::ArgumentList arguments;
        commandLine->GetArguments(arguments);

        std::string sourceCode = readFile(arguments.at(0));
        if (sourceCode.empty()) {
            return;
        }

        //

        CefRefPtr<CefV8Value> leprechaun = CefV8Value::CreateObject(0);
        CefRefPtr<CefV8Value> exit = CefV8Value::CreateFunction("exit", this);
        leprechaun->SetValue("exit", exit, V8_PROPERTY_ATTRIBUTE_READONLY);

        CefRefPtr<CefV8Value> global = context->GetGlobal();
        global->SetValue("leprechaun", leprechaun, V8_PROPERTY_ATTRIBUTE_READONLY);

        CefRefPtr<CefV8Value> bootstrapFunction;
        CefRefPtr<CefV8Exception> bootstrapException;
        bool evalResult = context->Eval(
            CefString(
                "(function leprechaun_bootstrap$(sourceCode) {"
                "    var s = document.createElement(\"script\");"
                "    var t = document.createTextNode(sourceCode + \"\\n//@sourceURL=thingie\");"
                "    s.appendChild(t);"
                "    document.body.appendChild(s);"
                "})"
            ), bootstrapFunction, bootstrapException
        );

        if (!evalResult) {
            printf("Eval error: %S\n", bootstrapException->GetMessage().ToWString().c_str());

            assert(evalResult);
        }

        CefV8ValueList args;
        args.push_back(CefRefPtr<CefV8Value>(CefV8Value::CreateString(sourceCode)));
        CefRefPtr<CefV8Value> callResult = bootstrapFunction->ExecuteFunction(0, args);
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
            // FIXME: This breaks if we run chromium in multiprocess mode.
            // I can't figure out how to get the IPC right. -- andy 5 September 2012
            bool result = CefPostTask(
                TID_UI,
                NewCefRunnableFunction(&CefQuitMessageLoop)
            );

            retval = CefV8Value::CreateString(result ? L"OK" : L"NOT OK");

            return true;
        }

        return false;
    }
};

void destroy() {
    CefQuitMessageLoop();
}

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);

    CefRefPtr<CefCommandLine> commandLine(CefCommandLine::CreateCommandLine());
    commandLine->InitFromArgv(argc, argv);

    printf("Commandline %S\n", commandLine->GetCommandLineString().ToWString().c_str());

    // FIXME: Chromium launches a zygote, even though we're in single-process mode.
    // It's not clear to me what this accomplishes, but for now, we just roll with it.
    // -- andy 7 September 2012
    if (!commandLine->HasSwitch("type") && argc != 2) {
        printf("Syntax: %s filename.js\n", argv[0]);
        return 1;
    }

    CefRefPtr<CefApp> app(new ChromeWindowApp(commandLine));
    CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient);

    CefMainArgs args(argc, argv);
    CefSettings appSettings;
    //appSettings.log_severity = LOGSEVERITY_VERBOSE;
    appSettings.multi_threaded_message_loop = false;
    appSettings.single_process = true;
    CefInitialize(args, appSettings, app);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_widget_destroyed), &window);
    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(destroy), NULL);

    CefWindowInfo info;
    info.SetAsChild(window);

    CefBrowserSettings settings;

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
