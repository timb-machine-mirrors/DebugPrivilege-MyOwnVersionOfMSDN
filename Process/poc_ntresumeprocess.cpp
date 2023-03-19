#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);

void resumeProcess(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle)
    {
        std::cout << "OpenProcess failed. Error code: " << GetLastError() << std::endl;
        return;
    }

    NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(
        GetModuleHandle(L"ntdll"), "NtResumeProcess");

    pfnNtResumeProcess(processHandle);
    CloseHandle(processHandle);
}

int main()
{
    DWORD processId = 0;
    std::wstring processName;

    // Get the process name from the user
    std::wcout << L"Enter the name of the process: ";
    std::getline(std::wcin, processName);

    // Get the process id of the specified process
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cout << "CreateToolhelp32Snapshot failed. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe))
    {
        do
        {
            if (!_wcsicmp(pe.szExeFile, processName.c_str()))
            {
                processId = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);

    if (!processId)
    {
        std::wcout << processName << L" not found." << std::endl;
        return 1;
    }

    resumeProcess(processId);
    std::cout << "The specified process has been resumed." << std::endl;

    return 0;
}
