
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <cassert>

#include "include/cef_app.h"
#include "include/cef_base.h"


#include "main.h"

int main(int argc, char** argv) {
    {
        setlocale(LC_ALL, "");
        CefMainArgs main_args(argc, argv);
        CefRefPtr<CefCommandLine> commandLine(CefCommandLine::CreateCommandLine());
        commandLine->InitFromArgv(main_args.argc, main_args.argv);

        CefRefPtr<CefApp> app(new ChromeWindowApp());

        // Execute the secondary process, if any.
        int exit_code = CefExecuteProcess(main_args, app.get());
        if (exit_code >= 0) {
            return exit_code;
        }

        if (FILE* f = fopen("out.txt", "a")) {
            fprintf(f, "CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());
            fclose(f);
        }
        printf("CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());

        //gtk_init(&argc, &argv);

        if (argc < 2) {
            printf("Syntax: %s filename.js [args for js]\n", argv[0]);
            return 1;
        }

        CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient());

        CefMainArgs args(argc, argv);
        CefSettings appSettings;
        appSettings.pack_loading_disabled = true;
        appSettings.remote_debugging_port = 24042;
        //appSettings.log_severity = LOGSEVERITY_VERBOSE;
        CefInitialize(main_args, appSettings, app);

        CefWindowInfo info;

        CefBrowserSettings settings;
        settings.web_security_disabled = true;

        CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(
            info, client.get(),
            "data:text/html,<!DOCTYPE html><html><head></head><body></body><script>void 0;</script></html>",
            settings
        );

        //gtk_widget_show_all(GTK_WIDGET(window));

        CefRunMessageLoop();
    }
    CefShutdown();


    return s_result;
}
