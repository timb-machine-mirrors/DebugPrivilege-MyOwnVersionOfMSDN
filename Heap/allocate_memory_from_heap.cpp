// Example: HeapAlloc function to allocate a buffer for the command line arguments and the HeapFree function is used to free the buffer

#include <windows.h>
#include <iostream>

int main()
{
    // Declare variables for the process information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Get the handle to the process heap
    HANDLE hHeap = GetProcessHeap();

    // Allocate a buffer for the command line arguments
    const wchar_t* originalCmdLine = L"cmd.exe /C whoami /all";
    size_t cmdLineSize = (wcslen(originalCmdLine) + 1) * sizeof(wchar_t);
    wchar_t* commandLine = static_cast<wchar_t*>(HeapAlloc(hHeap, 0, cmdLineSize));

    // Copy the command line arguments into the buffer
    memcpy(commandLine, originalCmdLine, cmdLineSize);

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
        HeapFree(hHeap, 0, commandLine);
        return 1;
    }

    // Wait for the process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Free the buffer for the command line arguments
    HeapFree(hHeap, 0, commandLine);

    // Close the handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
