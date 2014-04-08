#include "WindowClient.h"

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

void WindowClient::OnAfterCreated(CefRefPtr<CefBrowser> aBrowser) {
    if (browser.get()) {
        return;
    }

    browser = aBrowser;
}

bool WindowClient::OnProcessMessageReceived(
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
        for(unsigned int i = 0; i < arguments.size(); ++i) {
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

