#include "WindowClient.h"

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

WindowClient::WindowClient() {
}

WindowClient::~WindowClient() {
}

CefRefPtr<CefDisplayHandler> WindowClient::GetDisplayHandler() {
    return this;
}

CefRefPtr<CefLifeSpanHandler> WindowClient::GetLifeSpanHandler() {
    return this;
}

void WindowClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    browsers_.push_back(browser);
}

void WindowClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    for (BrowserList::iterator i = browsers_.begin(); i != browsers_.end(); ++i) {
        if ((*i)->IsSame(browser)) {
            browsers_.erase(i);
            break;
        }
    }

    if (browsers_.empty()) {
        CefQuitMessageLoop();
    }
}

bool WindowClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
    const CefString& message,
    const CefString& source,
    int line
) {
    return true;
}

bool WindowClient::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message
) {
    if (message->GetName() == "quit") {
        s_result = message->GetArgumentList()->GetInt(0);
        printf("Quitting with value %d\n", s_result);
        printf("Log:\n%S", message->GetArgumentList()->GetString(1).ToWString().c_str());

        CloseAllBrowsers();
    } else {
        printf("Unknown message %s\n", message->GetName().ToString().c_str());
    }
    return false;
}

void WindowClient::CloseAllBrowsers() {
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI,
            base::Bind(&WindowClient::CloseAllBrowsers, this));
        return;
    }

    if (browsers_.empty()) {
        CefQuitMessageLoop();
    }

    for (BrowserList::const_iterator i = browsers_.begin(); i != browsers_.end(); ++i) {
        (*i)->GetHost()->CloseBrowser(true);
    }
}
