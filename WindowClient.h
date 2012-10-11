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

struct WindowClient : public CefClient
                    , public CefLifeSpanHandler
                    , public CefDisplayHandler
                    , public CefRequestHandler
{
    WindowClient();
    ~WindowClient();

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

    IMPLEMENT_REFCOUNTING(WindowClient);
};

// aieeeee
extern int s_result;
