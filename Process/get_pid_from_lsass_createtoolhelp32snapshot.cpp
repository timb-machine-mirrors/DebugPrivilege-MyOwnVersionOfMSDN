#include <iostream>
#include <windows.h>
#include <TlHelp32.h>

struct ProcessInfo
{
    DWORD processPID = 0;
    std::wstring processName;
};

ProcessInfo getProcessInfo(const std::string& processName)
{
    HANDLE snap_handler = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap_handler == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        std::cerr << "Error creating process snapshot: " << error << std::endl;
        exit(1);
    }

    PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
    std::wstring processNameWide(processName.begin(), processName.end());
    const wchar_t* szName = processNameWide.c_str();

    ProcessInfo info;
    if (Process32First(snap_handler, &processEntry))
    {
        do
        {
            if (_wcsicmp(processEntry.szExeFile, szName) == 0)
            {
                info.processPID = processEntry.th32ProcessID;
                info.processName = processEntry.szExeFile;
                break;
            }
        } while (Process32Next(snap_handler, &processEntry));
    }

    // Check for errors
    if (info.processPID == 0)
    {
        DWORD error = GetLastError();
        if (error != ERROR_SUCCESS)
        {
            std::cerr << "Error getting process information: " << error << std::endl;
            exit(1);
        }
    }

    CloseHandle(snap_handler);
    processNameWide.clear();

    return info;
}

int main(int argc, char** argv)
{
    const std::string processName = "lsass.exe";
    ProcessInfo info = getProcessInfo(processName);

    std::cout << "PID: " << info.processPID << std::endl;
    std::wcout << "ProcessName: " << info.processName << std::endl;
}
