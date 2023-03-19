#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iomanip>
#include <codecvt>
#include <locale>

struct ProcessInfo {
    DWORD processPID = 0;
    std::wstring processName;
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
            ProcessInfo info;
            info.processPID = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
            processInfos.push_back(info);
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
    std::wcout << L"+-------+---------------------+-------------+" << std::endl;
    std::wcout << L"| PID   | Process Name        | HandleCount |" << std::endl;
    std::wcout << L"+-------+---------------------+-------------+" << std::endl;

    for (const auto& info : processInfos)
    {
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.processPID);
        if (processHandle != NULL) {
            DWORD handleCount;
            if (GetProcessHandleCount(processHandle, &handleCount)) {
                std::wcout << L"| " << std::setw(5) << std::left << info.processPID
                    << L"| " << std::setw(20) << std::left << info.processName
                    << L"| " << std::setw(11) << std::left << handleCount << L"|" << std::endl;
            }
            else {
                std::cerr << "Error getting handle count: " << GetLastError() << std::endl;
            }
            CloseHandle(processHandle);
        }
        else {
            DWORD error = GetLastError();
            if (error != ERROR_ACCESS_DENIED) {
                std::cerr << "Error opening process: " << error << std::endl;
            }
        }
    }

    // Print footer
    std::wcout << L"+-------+---------------------+-------------+" << std::endl;

    return 0;
}
