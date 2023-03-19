#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);

void SuspendProcess(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (processHandle == NULL)
    {
        std::cout << "OpenProcess failed. Error code: " << GetLastError() << std::endl;
        return;
    }

    NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
        GetModuleHandle(L"ntdll"), "NtSuspendProcess");
    if (pfnNtSuspendProcess == NULL)
    {
        std::cout << "GetProcAddress failed. Error code: " << GetLastError() << std::endl;
        CloseHandle(processHandle);
        return;
    }

    LONG status = pfnNtSuspendProcess(processHandle);
    if (status != 0)
    {
        std::cout << "NtSuspendProcess failed. Error code: " << status << std::endl;
    }
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

    // Suspend the specified process
    SuspendProcess(processId);

    std::cout << "The specified process has been suspended." << std::endl;
    return 0;
}
