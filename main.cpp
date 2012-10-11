
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <fstream>
#include <sstream>

#include "main.h"
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

    frame->ExecuteJavaScript(sourceCode, "", 0);
}

ChromeWindowClient::ChromeWindowClient() {
}

ChromeWindowClient::~ChromeWindowClient() {
}

CefRefPtr<CefDisplayHandler> ChromeWindowClient::GetDisplayHandler() {
    return this;
}

CefRefPtr<CefLifeSpanHandler> ChromeWindowClient::GetLifeSpanHandler() {
    return this;
}

void ChromeWindowClient::OnAfterCreated(CefRefPtr<CefBrowser> aBrowser) {
    if (browser.get()) {
        return;
    }

    browser = aBrowser;
}

bool ChromeWindowClient::OnProcessMessageReceived(
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

ChromeWindowApp::ChromeWindowApp() {
}

CefRefPtr<CefRenderProcessHandler> ChromeWindowApp::GetRenderProcessHandler() {
    return this;
}

void ChromeWindowApp::OnContextCreated(
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

void ChromeWindowApp::OnContextReleased(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context
) {
}

void ChromeWindowApp::OnWebKitInitialized() {
}

bool ChromeWindowApp::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, 
    CefProcessId source_process, 
    CefRefPtr<CefProcessMessage> message
) {
    if (message->GetName() == "arguments") {
        printf("Handling arguments\n");
        CefRefPtr<CefListValue> arguments = message.get()->GetArgumentList();
        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();
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

bool ChromeWindowApp::Execute(
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
        return true;
    } else if (name == "echo") {
        std::wstringstream ss;
        for (size_t i = 0; i < arguments.size(); ++i) {
            ss << arguments[i]->GetStringValue().ToWString();
        }
        this->outBuffer += ss.str().c_str();
        this->outBuffer += L"\n";
        return true;
    } else if (name == "onerror") {
        return true;
    }

    return false;
}

void noOpenWindow() {
    printf("Test tried to window.open!  This is bad!\n");
    exit(1);
}
