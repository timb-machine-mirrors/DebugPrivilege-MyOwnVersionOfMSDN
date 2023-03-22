// Compile this sample as a .DLL
// It contains two exports as example that have the same functionality. MyExecuteProcess() and MyOtherFunction().
// To automatically execute the MyExecuteProcess function when the DLL is injected into a specified process, 
// we need to call it from the DllMain function and specify the export at DLL_PROCESS_ATTACH

#include <windows.h>
#include <iostream>

// Define the exported function with a custom name
extern "C" __declspec(dllexport) void MyExecuteProcess()
{
    // Declare variables for the process information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Command line arguments for the process
    wchar_t commandLine[] = L"cmd.exe /k whoami /all";

    // Create the process
    if (!CreateProcess(
        L"C:\\Windows\\System32\\cmd.exe",  // Path to the executable
        commandLine,                        // Command-line arguments
        NULL,                               // Process handle not inheritable
        NULL,                               // Thread handle not inheritable
        FALSE,                              // Set handle inheritance to FALSE
        0,                                  // No creation flags
        NULL,                               // Use parent's environment block
        NULL,                               // Use parent's starting directory 
        &si,                                // Pointer to STARTUPINFO structure
        &pi)                                // Pointer to PROCESS_INFORMATION structure
        ) {
        std::cout << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return;
    }

    // Wait for the process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close the handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Define the second exported function with a custom name
extern "C" __declspec(dllexport) void MyOtherFunction()
{
    // Declare variables for the process information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Command line arguments for the process
    wchar_t commandLine[] = L"cmd.exe /k whoami /all";

    // Create the process
    if (!CreateProcess(
        L"C:\\Windows\\System32\\cmd.exe",  // Path to the executable
        commandLine,                        // Command-line arguments
        NULL,                               // Process handle not inheritable
        NULL,                               // Thread handle not inheritable
        FALSE,                              // Set handle inheritance to FALSE
        0,                                  // No creation flags
        NULL,                               // Use parent's environment block
        NULL,                               // Use parent's starting directory 
        &si,                                // Pointer to STARTUPINFO structure
        &pi)                                // Pointer to PROCESS_INFORMATION structure
        ) {
        std::cout << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return;
    }

    // Wait for the process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close the handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

// Define the entry point function for the DLL
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Call the exported function when the DLL is loaded into the process
        MyExecuteProcess();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
