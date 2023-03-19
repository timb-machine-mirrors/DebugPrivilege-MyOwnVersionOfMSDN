#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iomanip>

class ProcessInfo {
public:
    // member variables
    DWORD processPID = 0;           // process ID
    std::wstring processName;       // process name
    FILETIME lpCreationTime;        // process creation time
    FILETIME lpUserTime;            // amount of time spent in user mode by the process
    FILETIME lpKernelTime;          // amount of time spent in kernel mode by the process

    // constructors
    ProcessInfo() = default;        // default constructor
    ProcessInfo(DWORD pid, std::wstring name, FILETIME creationTime, FILETIME userTime, FILETIME kernelTime)
        : processPID(pid), processName(name), lpCreationTime(creationTime), lpUserTime(userTime), lpKernelTime(kernelTime)
    {}                              // parameterized constructor
};


std::vector<ProcessInfo> getAllProcessInfo()
{
    std::vector<ProcessInfo> processInfos;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating snapshot: " << GetLastError() << std::endl;
        return processInfos;
    }

    PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
    if (Process32First(snapshot, &processEntry)) {
        while (Process32Next(snapshot, &processEntry)) {
            HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processEntry.th32ProcessID);
            if (processHandle != NULL) {
                FILETIME lpCreationTime, lpExitTime, lpUserTime, lpKernelTime;
                if (GetProcessTimes(processHandle, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime)) {
                    ProcessInfo info(processEntry.th32ProcessID, processEntry.szExeFile, lpCreationTime, lpUserTime, lpKernelTime);
                    processInfos.push_back(info);
                }
                CloseHandle(processHandle);
            }
        }
    }
    else {
        std::cerr << "Error getting process information: " << GetLastError() << std::endl;
    }

    CloseHandle(snapshot);
    return processInfos;
}

int main(int argc, char** argv)
{
    std::vector<ProcessInfo> processInfos = getAllProcessInfo();
    for (const auto& info : processInfos)
    {
        std::wcout << "PID: " << info.processPID << std::endl;
        std::wcout << "ProcessName: " << info.processName << std::endl;

        SYSTEMTIME stUTC, stLocal;
        FileTimeToSystemTime(&info.lpCreationTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        std::wcout << "CreationTime: " << std::setfill(L'0') << std::setw(4) << stLocal.wYear << L"-"
            << std::setfill(L'0') << std::setw(2) << stLocal.wMonth << L"-"
            << std::setfill(L'0') << std::setw(2) << stLocal.wDay << " "
            << std::setfill(L'0') << std::setw(2) << stLocal.wHour << ":"
            << std::setfill(L'0') << std::setw(2) << stLocal.wMinute << std::endl;

        ULARGE_INTEGER userTime;
        userTime.LowPart = info.lpUserTime.dwLowDateTime;
        userTime.HighPart = info.lpUserTime.dwHighDateTime;
        std::wcout << "UserTime: " << (userTime.QuadPart / 10000000) << " seconds" << std::endl;

        LARGE_INTEGER kernelTime;
        kernelTime.HighPart = info.lpKernelTime.dwHighDateTime;
        kernelTime.LowPart = info.lpKernelTime.dwLowDateTime;
        std::wcout << "KernelTime: " << (kernelTime.QuadPart / 10000000) << " seconds" << std::endl;

        std::wcout << std::endl;
    }

    return 0;
}
