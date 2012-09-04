
#include <stdio.h>
#include "include/cef_base.h"
#include "include/cef_client.h"


#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
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

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() {
        return this;
    }

    // CefV8Handler

    virtual void OnContextCreated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    ) {
        printf("oncontextcreated\n");
        CefRefPtr<CefV8Value> f = CefV8Value::CreateFunction("call", this);

        CefRefPtr<CefV8Value> leprechaun = CefV8Value::CreateObject(0);
        leprechaun->SetValue("call", f, V8_PROPERTY_ATTRIBUTE_READONLY);

        context->GetGlobal()->SetValue("leprechaun", leprechaun, V8_PROPERTY_ATTRIBUTE_READONLY);
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
        if (name == "call") {
            /* ... */
            return true;
        }

        return false;
    }
};

int main(int argc, char** argv) {
    CefRefPtr<CefApp> app(new ChromeWindowApp);

    CefMainArgs args(argc, argv);
    CefSettings appSettings;
    appSettings.log_severity = LOGSEVERITY_VERBOSE;
    CefInitialize(args, appSettings, app);

    CefWindowInfo info;
    CefRefPtr<ChromeWindowClient> client = new ChromeWindowClient;

    CefBrowserSettings settings;

    CefBrowser* browser = CefBrowserHost::CreateBrowserSync(
        info, client.get(),
        "about:blank",
        settings
    );

    printf("CreateBrowser result: 0x08X\n", browser);

    CefRunMessageLoop();
    CefShutdown();

    return 0;
}
