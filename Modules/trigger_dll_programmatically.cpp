// Trigger a DLL programmatically instead of calling rundll32.exe with the export.

#include <Windows.h>
#include <iostream>

int main() {
    // Load the DLL into the process
    HMODULE hModule = LoadLibrary(L"C:\\Users\\Admin\\source\\repos\\Dll1\\x64\\Release\\Dll1.dll");
    if (hModule == NULL) {
        std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
        return 1;
    }

    // Get a pointer to the function
    typedef void (*MyExecuteProcessPtr)(void);
    MyExecuteProcessPtr myExecuteProcess = (MyExecuteProcessPtr)GetProcAddress(hModule, "MyExecuteProcess");
    if (myExecuteProcess == NULL) {
        std::cerr << "Failed to get function pointer: " << GetLastError() << std::endl;
        FreeLibrary(hModule);
        return 1;
    }

    // Call the function
    myExecuteProcess(); // Export of Dll1.dll

    // Free the DLL from the process
    FreeLibrary(hModule);

    return 0;
}
