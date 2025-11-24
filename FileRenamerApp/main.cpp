// Entry point utama untuk aplikasi Windows
#include <windows.h>
#include "UI.h"
// Entry point aplikasi Windows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    return RunApp(hInstance, nCmdShow);
}
