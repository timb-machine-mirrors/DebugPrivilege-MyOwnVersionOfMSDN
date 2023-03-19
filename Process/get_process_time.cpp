#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iomanip>
#include <codecvt>
#include <locale>
#include <string>

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

    // Set the locale to UTF-8 to support wide characters
    std::locale::global(std::locale(""));

    // Print header
    std::wcout << L"+-------+----------------------+---------------------+---------------------+---------------------+" << std::endl;
    std::wcout << L"| PID   | Process Name         | Creation Time       | User Time (seconds) | Kernel Time (seconds)|" << std::endl;
    std::wcout << L"+-------+----------------------+---------------------+---------------------+---------------------+" << std::endl;

    for (const auto& info : processInfos)
    {
        SYSTEMTIME stUTC, stLocal;
        FileTimeToSystemTime(&info.lpCreationTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        std::wstring creationTimeStr = std::to_wstring(stLocal.wYear) + L"-" +
            std::wstring(1, L'0') + std::to_wstring(stLocal.wMonth) + L"-" +
            std::wstring(1, L'0') + std::to_wstring(stLocal.wDay) + L" " +
            std::wstring(1, L'0') + std::to_wstring(stLocal.wHour) + L":" +
            std::wstring(1, L'0') + std::to_wstring(stLocal.wMinute);

        ULARGE_INTEGER userTime;
        userTime.LowPart = info.lpUserTime.dwLowDateTime;
        userTime.HighPart = info.lpUserTime.dwHighDateTime;
        std::wstring userTimeStr = std::to_wstring(userTime.QuadPart / 10000000);

        LARGE_INTEGER kernelTime;
        kernelTime.HighPart = info.lpKernelTime.dwHighDateTime;
        kernelTime.LowPart = info.lpKernelTime.dwLowDateTime;
        std::wstring kernelTimeStr = std::to_wstring(kernelTime.QuadPart / 10000000);

        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string processName = converter.to_bytes(info.processName);

        std::wprintf(L"| %5d | %-20S | %-19s | %-19s | %-19s |\n",
            info.processPID, processName.c_str(),
            creationTimeStr.c_str(), userTimeStr.c_str(), kernelTimeStr.c_str());
    }

    // Print footer
    std::wcout << L"+-------+----------------------+---------------------+---------------------+---------------------+" << std::endl;

    return 0;
}
