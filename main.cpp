
#include <stdio.h>
#include <gtk/gtk.h>
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
{
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

    CefRefPtr<CefBrowser> browser;

    IMPLEMENT_REFCOUNTING(ChromeWindowClient);
};

struct ChromeWindowApp : public CefApp
                       , public CefRenderProcessHandler
                       , public CefV8Handler
{
    IMPLEMENT_REFCOUNTING(ChromeWindowApp);

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
        CefRefPtr<CefV8Value> leprechaun = CefV8Value::CreateObject(0);
        CefRefPtr<CefV8Value> exit = CefV8Value::CreateFunction("exit", this);
        leprechaun->SetValue("exit", exit, V8_PROPERTY_ATTRIBUTE_READONLY);

        CefRefPtr<CefV8Value> global = context->GetGlobal();
        global->SetValue("leprechaun", leprechaun, V8_PROPERTY_ATTRIBUTE_READONLY);
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

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 80, 60);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gtk_widget_destroyed), &window);
    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(destroy), NULL);

    CefRefPtr<CefApp> app(new ChromeWindowApp);
    CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient);

    CefMainArgs args(argc, argv);
    CefSettings appSettings;
    appSettings.log_severity = LOGSEVERITY_VERBOSE;
    appSettings.multi_threaded_message_loop = false;
    appSettings.single_process = true;
    CefInitialize(args, appSettings, app);

    CefWindowInfo info;
    info.SetAsChild(window);

    CefBrowserSettings settings;

    CefBrowser* browser = CefBrowserHost::CreateBrowserSync(
        info, client.get(),
        "file:///home/cit/src/leprechaun/test.html",
        settings
    );

    printf("CreateBrowser result: 0x08X\n", browser);
    gtk_widget_show_all(GTK_WIDGET(window));

    CefRunMessageLoop();
    CefShutdown();

    return 0;
}
