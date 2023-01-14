#include <Windows.h>
#include <string>

int main()
{
    std::wstring username = L"Admin";
    std::wstring domain = L"contoso";
    std::wstring password = L"Passw0rd!";
    std::wstring cmd = L"cmd.exe";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessWithLogonW(username.c_str(), domain.c_str(), password.c_str(), LOGON_WITH_PROFILE, cmd.c_str(), NULL, 0, NULL, NULL, &si, &pi))
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
