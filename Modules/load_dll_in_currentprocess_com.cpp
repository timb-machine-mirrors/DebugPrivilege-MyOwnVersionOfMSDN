// Loading a DLL into the current process via COM

#include <iostream>
#include <Windows.h>
#include <comdef.h>

typedef struct {
    const wchar_t* dllPath;
    HINSTANCE hInst;
} MyDllData;

int main() {
    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM: " << _com_error(hr).ErrorMessage() << std::endl;
        return 1;
    }

    // Specify the path to your DLL
    MyDllData myDll = { L"C:\\Users\\Admin\\source\\repos\\Dll1\\x64\\Release\\Dll1.dll", NULL };

    // Load the DLL using CoLoadLibrary
    myDll.hInst = CoLoadLibrary(const_cast<LPOLESTR>(myDll.dllPath), TRUE);
    if (!myDll.hInst) {
        std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
        CoUninitialize();
        return 1;
    }

    std::cout << "DLL loaded successfully." << std::endl;

    // Perform tasks with the loaded DLL

    // Unload the DLL using CoFreeLibrary
    CoFreeLibrary(myDll.hInst);

    // Uninitialize COM
    CoUninitialize();

    return 0;
}
