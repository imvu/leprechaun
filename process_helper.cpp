
#include "include/cef_app.h"
#include "Application.h"

int main(int argc, char* argv[]) {
    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefApp> app(new Application);
    return CefExecuteProcess(main_args, app);
}
