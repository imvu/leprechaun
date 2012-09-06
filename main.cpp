
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
        printf("OnConsoleMessage %ls\n", message.c_str());
        return false;
    }

    CefRefPtr<CefBrowser> browser;

    IMPLEMENT_REFCOUNTING(ChromeWindowClient);
};

struct ChromeWindowApp : public CefApp
                       , public CefRenderProcessHandler
                       , public CefV8Handler
{
    IMPLEMENT_REFCOUNTING(ChromeWindowApp);

private:
    std::vector<std::string> argv;

public:
    ChromeWindowApp(const std::vector<std::string>& argv)
        : argv(argv)
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
        char* contents;
        {
            FILE* file = fopen(argv.at(1).c_str(), "rb");
            if (!file) {
                printf("Unable to read %s\n", argv[1].c_str());
                return;
            }

            fseek(file, 0, SEEK_END);
            int len = ftell(file);
            fseek(file, 0, SEEK_SET);

            contents = new char[len + 1];
            fread(contents, 1, len, file);
            contents[len] = 0;

            fclose(file);
        }

        CefString sourceCode(contents);
        delete[] contents;
        contents = 0;

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
            printf("Eval error: %ls\n", bootstrapException->GetMessage().c_str());

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

    if (argc > 1 && std::string(argv[1]) == "--type=zygote") {
        // ok
    } else if (argc == 2) {
        // ok
    } else {
        printf("Syntax: %s filename.js\n", argv[0]);
        return 1;
    }

    CefRefPtr<CefApp> app(new ChromeWindowApp(std::vector<std::string>(argv, argv + argc)));
    CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient);

    CefMainArgs args(argc, argv);
    CefSettings appSettings;
    appSettings.log_severity = LOGSEVERITY_VERBOSE;
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

    printf("CreateBrowser result: 0x08X\n", browser);
    gtk_widget_show_all(GTK_WIDGET(window));

    CefRunMessageLoop();
    CefShutdown();

    return 0;
}
