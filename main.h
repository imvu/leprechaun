#pragma once

#include "include/cef_base.h"
#include "include/cef_client.h"

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_runnable.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#include "include/cef_app.h"

struct ChromeWindowApp : public CefApp
                       , public CefRenderProcessHandler
                       , public CefV8Handler
{
    ChromeWindowApp();

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler();

    virtual void OnContextCreated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    );

    virtual void OnContextReleased(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context
    );

    virtual void OnWebKitInitialized();

    virtual bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser, 
        CefProcessId source_process, 
        CefRefPtr<CefProcessMessage> message
    );

    virtual bool Execute(
        const CefString& name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval,
        CefString& exception
    );

private:
    CefRefPtr<CefBrowser> firstBrowser;
    CefRefPtr<CefV8Value> leprechaunObj;
    std::wstring outBuffer;

    IMPLEMENT_REFCOUNTING(ChromeWindowApp);
};

struct ChromeWindowClient : public CefClient
                          , public CefLifeSpanHandler
                          , public CefDisplayHandler
                          , public CefRequestHandler
{
    ChromeWindowClient();
    ~ChromeWindowClient();

    // CefClient
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler();
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler();

    // CefLifeSpanHandler
    CefRefPtr<CefBrowser> browser;

    virtual void OnAfterCreated(CefRefPtr<CefBrowser> aBrowser);

    // CefRequestHandler
    virtual bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message
    );

    IMPLEMENT_REFCOUNTING(ChromeWindowClient);
};

// aieeeee
extern int s_result;
