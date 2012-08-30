
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
                          , public CefV8Handler
{
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() {
        return this;
    }

    /*virtual CefRefPtr<CefV8ContextHandler> GetV8ContextHandler() {
        return this;
        }*/

    // CefLifeSpanHandler

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> aBrowser) {
        if (browser.get()) {
            return;
        }

        browser = aBrowser;
    }

    CefRefPtr<CefBrowser> browser;

    // CefV8ContextHandler

    virtual void OnContextCreated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    ) {
        /*CefRefPtr<CefV8Value> f = CefV8Value::CreateFunction("call", this);

        CefRefPtr<CefV8Value> imvu = CefV8Value::CreateObject(0, 0);
        imvu->SetValue("call", f, V8_PROPERTY_ATTRIBUTE_READONLY);

        context->GetGlobal()->SetValue("imvu", imvu, V8_PROPERTY_ATTRIBUTE_READONLY);*/
    }

    virtual void OnContextReleased(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    ) {
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


    IMPLEMENT_REFCOUNTING(ChromeWindowClient);
};

struct ChromeWindowApp : public CefApp {
    IMPLEMENT_REFCOUNTING(ChromeWindowApp);
};

//ChromeWindow::ChromeWindow(INativeWindowPtr window, const std::wstring& url)
//    : app(new ChromeWindowApp)
//    , window(window)
//{
    //NativeWindowPtr wnd = boost::dynamic_pointer_cast<NativeWindow>(window);
    //NSView* contentView = wnd->getView();
int main(int argc, char** argv) {
    printf("Hello, world!\n");


    CefMainArgs main_args(argc, argv);

    CefSettings appSettings;
    appSettings.log_severity = LOGSEVERITY_VERBOSE;
    //appSettings.pack_loading_disabled = true;
    //appSettings.pack_file_path = L".";
    //appSettings.locales_dir_path = L".";

    CefRefPtr<CefApp> app(new ChromeWindowApp);

    CefInitialize(main_args, appSettings, app);

    //auto size = wnd->getSize();

    const int WIDTH = 640;
    const int HEIGHT = 480;

    CefWindowInfo info;
    //info.SetAsChild(NULL, 0, 0, WIDTH, HEIGHT);

    CefRefPtr<ChromeWindowClient> client = new ChromeWindowClient;

    CefBrowserSettings settings;

    CefBrowser* browser = CefBrowserHost::CreateBrowserSync(
        info, client.get(),
        "http://www.google.com",
        settings
    );

    printf("CreateBrowser result: 0x08X\n", browser);

    //frameListener = new ChromeFrameListener();
    CefRunMessageLoop();
    CefShutdown();

    return 0;
}
