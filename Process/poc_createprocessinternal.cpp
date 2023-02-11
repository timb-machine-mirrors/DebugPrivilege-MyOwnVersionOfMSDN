#include <windows.h>
#include <tchar.h>
#include <iostream>

typedef BOOL(WINAPI* CreateProcessInternalFunction)(
    HANDLE hToken,
    LPCTSTR lpApplicationName,
    LPTSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCTSTR lpCurrentDirectory,
    LPSTARTUPINFO lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    PHANDLE hNewToken
    );

int main()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Load the KernelBase.dll library.
    HMODULE hKernelBase = LoadLibrary(_T("KernelBase.dll"));
    if (hKernelBase == NULL)
    {
        std::cout << "LoadLibrary failed. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Get the address of the CreateProcessInternal function.
    CreateProcessInternalFunction CreateProcessInternal = (CreateProcessInternalFunction)GetProcAddress(hKernelBase, "CreateProcessInternalW");
    if (CreateProcessInternal == NULL)
    {
        std::cout << "GetProcAddress failed. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // The command line argument for the process to be created.
    LPCTSTR lpApplicationName = _T("C:\\Windows\\System32\\notepad.exe");

    // Flag that specifies the creation of a new console for the process.
    DWORD dwCreationFlags = CREATE_NEW_CONSOLE;

    HANDLE hNewToken = NULL;

    // Call the CreateProcessInternal function to create the process.
    if (!CreateProcessInternal(NULL,
        (LPWSTR)lpApplicationName,
        NULL,
        NULL,
        NULL,
        NULL,
        dwCreationFlags,
        NULL,
        NULL,
        &si,
        &pi,
        &hNewToken))
    {
        std::cout << "CreateProcessInternal failed. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Wait for the process to complete.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close the process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Free the KernelBase.dll library.
    FreeLibrary(hKernelBase);

    return 0;
}
