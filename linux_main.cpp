#include <locale.h>

#include "Application.h"
#include "WindowClient.h"

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");
    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefCommandLine> commandLine(CefCommandLine::CreateCommandLine());
    commandLine->InitFromArgv(main_args.argc, main_args.argv);

    CefRefPtr<CefApp> app(new Application());

    // Execute the secondary process, if any.
    int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
    if (exit_code >= 0) {
        return exit_code;
    }
    printf("CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());

    if (argc < 2) {
        printf("Syntax: %s filename.js [args for js]\n", argv[0]);
        return 1;
    }

    CefSettings appSettings;
    appSettings.remote_debugging_port = 24042;
    CefInitialize(main_args, appSettings, app, NULL);

    CefRunMessageLoop();

    CefShutdown();

    return s_result;
}
