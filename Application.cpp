
#include "Application.h"

#include <errno.h>

#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "WindowClient.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

int s_result = 0;

std::string escape(const std::string & value, std::string::value_type escape_char, std::string::value_type replace_char) {
    std::ostringstream escaped;
    for (std::string::const_iterator i = value.begin(); i != value.end(); ++i) {
        const std::string::value_type c = (*i);
        if ((c == escape_char) || (c == replace_char)) {
            escaped << escape_char << c;
        } else {
            escaped << c;
        }
    }
    return escaped.str();
}
std::string escapeQuotes(const std::string & value) {
    return escape(value, '\\', '\"');
}

void noOpenWindow() {
    printf("Test tried to window.open!  This is bad!\n");
    exit(1);
}

Application::Application() {
}

CefRefPtr<CefBrowserProcessHandler> Application::GetBrowserProcessHandler() {
    return this;
}

CefRefPtr<CefRenderProcessHandler> Application::GetRenderProcessHandler() {
    return this;
}

void Application::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) {
    this->firstBrowser = NULL;
}

void Application::OnContextCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context
) {
    CefRefPtr<CefV8Value> global = context->GetGlobal();

    CefRefPtr<CefV8Value> notification = CefV8Value::CreateFunction("Notification", this);
    global->SetValue("Notification", notification, V8_PROPERTY_ATTRIBUTE_READONLY);

    if (this->firstBrowser != NULL) {
        // Disallow window.open

        CefRefPtr<CefV8Value> openWindow = CefV8Value::CreateFunction("openWindow", this);
        global->SetValue("open", openWindow, V8_PROPERTY_ATTRIBUTE_READONLY);

        // have first window debugger connect to the new browser
        firstBrowser->GetMainFrame()->ExecuteJavaScript("onNewBrowser();", "NewBrowser.js", 0);
        return;
    }
    this->firstBrowser = browser;

    this->leprechaunObj = CefV8Value::CreateObject(0, 0);
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

    CefRefPtr<CefV8Value> args = CefV8Value::CreateArray(0);
    this->leprechaunObj->SetValue("args", args, V8_PROPERTY_ATTRIBUTE_NONE);

    global->SetValue("leprechaun", this->leprechaunObj, V8_PROPERTY_ATTRIBUTE_READONLY);
}

void Application::OnContextInitialized() {
    CefRefPtr<WindowClient> client(new WindowClient());

    CefWindowInfo info;
    //info.windowless_rendering_enabled = true;

    CefBrowserSettings settings;
    settings.web_security = STATE_DISABLED;

    CefCommandLine::ArgumentList arguments;
    CefCommandLine::GetGlobalCommandLine()->GetArguments(arguments);

    std::ostringstream content;
    content << "data:text/html,<!DOCTYPE html><html><head><title>Leprechauns are oh so magically delicious</title></head><body>\n";
    content << "<script>leprechaun.args = [";
    for (unsigned int i = 0; i < arguments.size(); ++i) {
        content << ((i > 0) ? "," : "") << '\"' << escapeQuotes(arguments[i]) << '\"';
    }
    content << "];</script>\n";
    content << "<script src=\"" << escapeQuotes(arguments[0]) << "\"></script>\n";
    content << "</body><html>";


    // The script to be run is read from disk and executed in the browser frame in bootstrap() above.
    CefBrowserHost::CreateBrowser(
        info,
        client.get(),
        content.str(),
        settings,
        NULL
    );
}

void Application::OnContextReleased(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context
) {
}

void Application::OnWebKitInitialized() {
}

bool Application::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, 
    CefProcessId source_process, 
    CefRefPtr<CefProcessMessage> message
) {
    printf("Unknown render message %s\n", message->GetName().ToString().c_str());
    return false;
}

// CefV8Handler

bool Application::Execute(
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
        CefPostTask(
            TID_UI,
            CefCreateClosureTask(base::Bind(&noOpenWindow))
            );
        return true;

    } else if (name == "nop") {
        return true;

    } else if (name == "log") {
        /*if (object->IsSame(this->leprechaunObj)) {
            // Figure out if we need this some day...
            return false;
        }*/
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

