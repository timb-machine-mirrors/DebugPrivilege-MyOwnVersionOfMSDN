# Simple code snippet that calls CreateProcessWithLogonW to run under the security context of the user Jason to spawn cmd.exe 
# CreateProcessWithLogonW leverages the Secondary Logon Service

#include <Windows.h>
#include <string>

int main()
{
    std::wstring username = L"Jason";
    std::wstring password = L"Passw0rd!";
    std::wstring cmd = L"cmd.exe";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessWithLogonW(username.c_str(), NULL, password.c_str(), LOGON_WITH_PROFILE, cmd.c_str(), NULL, 0, NULL, NULL, &si, &pi))
    {
        DWORD errorCode = GetLastError();
        LPWSTR errorMessage = NULL;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMessage, 0, NULL);
        wprintf(L"CreateProcessWithLogonW failed with error code %d: %s\n", errorCode, errorMessage);
        LocalFree(errorMessage);
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
