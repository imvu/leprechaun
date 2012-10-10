
#include "include/cef_app.h"
#include "main.h"

int main(int argc, char* argv[]) {
    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefApp> app(new ChromeWindowApp);
    return CefExecuteProcess(main_args, app);
}
