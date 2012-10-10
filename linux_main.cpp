
#include <gtk/gtk.h>
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
        printf("CefCommandLine %S\n", commandLine->GetCommandLineString().ToWString().c_str());

        gtk_init(&argc, &argv);


        if (argc < 2) {
            printf("Syntax: %s filename.js [args for js]\n", argv[0]);
            return 1;
        }

        CefRefPtr<ChromeWindowClient> client(new ChromeWindowClient());

        CefMainArgs args(argc, argv);
        CefSettings appSettings;
        appSettings.remote_debugging_port = 24042;
        //appSettings.log_severity = LOGSEVERITY_VERBOSE;
        CefInitialize(main_args, appSettings, app);

        //GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        //gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

        //g_signal_connect(G_OBJECT(window), "destroy",
        //                 G_CALLBACK(gtk_widget_destroyed), &window);
        //g_signal_connect(G_OBJECT(window), "destroy",
        //                 G_CALLBACK(CefQuitMessageLoop), NULL);

        CefWindowInfo info;
        //info.SetAsChild(window);

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
