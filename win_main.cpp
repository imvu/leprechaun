
#include "Application.h"
#include "WindowClient.h"

int main(int argc, char ** argv) {
    setlocale(LC_ALL, "");
    CefMainArgs main_args(GetModuleHandle(NULL));
    CefRefPtr<CefCommandLine> commandLine(CefCommandLine::CreateCommandLine());
    commandLine->InitFromString(GetCommandLineW());

    CefRefPtr<CefApp> app(new Application());

    // Execute the secondary process, if any.
    int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
    if (exit_code >= 0) {
        return exit_code;
    }

    if (FILE* f = fopen("out.txt", "a")) {
        fprintf(f, "CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());
        fclose(f);
    }
    printf("CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());

    if (argc < 2) {
        printf("Syntax: %s filename.js [args for js]\n", argv[0]);
        return 1;
    }

    CefRefPtr<WindowClient> client(new WindowClient());

    CefSettings appSettings;
    appSettings.remote_debugging_port = 24042;
    appSettings.log_severity = LOGSEVERITY_DISABLE;
    CefInitialize(main_args, appSettings, app, NULL);

    CefRunMessageLoop();

    CefShutdown();

    return s_result;
}
