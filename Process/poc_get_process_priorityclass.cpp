#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <locale>
#include <codecvt>
#include <string>

typedef struct ProcessInfo {
    DWORD processPID = 0;
    std::wstring processName;
    DWORD priority = 0;
} ProcessInfo;

std::wstring getPriorityClassString(DWORD priority) {
    switch (priority) {
    case IDLE_PRIORITY_CLASS: return L"IDLE_PRIORITY_CLASS";
    case BELOW_NORMAL_PRIORITY_CLASS: return L"BELOW_NORMAL_PRIORITY_CLASS";
    case NORMAL_PRIORITY_CLASS: return L"NORMAL_PRIORITY_CLASS";
    case ABOVE_NORMAL_PRIORITY_CLASS: return L"ABOVE_NORMAL_PRIORITY_CLASS";
    case HIGH_PRIORITY_CLASS: return L"HIGH_PRIORITY_CLASS";
    case REALTIME_PRIORITY_CLASS: return L"REALTIME_PRIORITY_CLASS";
    default: return L"UNKNOWN_PRIORITY_CLASS";
    }
}

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
                DWORD priority = GetPriorityClass(processHandle);
                {
                    ProcessInfo info;
                    info.processPID = processEntry.th32ProcessID;
                    info.processName = processEntry.szExeFile;
                    info.priority = priority;
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
    std::wcout << L"+-------+---------------------+-----------------------+" << std::endl;
    std::wcout << L"| PID   | Process Name        | Priority              |" << std::endl;
    std::wcout << L"+-------+---------------------+-----------------------+" << std::endl;

    // Convert wide strings to narrow
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    // Print process information in a table
    for (const auto& info : processInfos)
    {
        std::string processName = converter.to_bytes(info.processName);
        std::wstring priorityClassString = getPriorityClassString(info.priority);

        std::printf("| %5d | %-20s | %-21ls |\n", info.processPID, processName.c_str(), priorityClassString.c_str());
    }

    // Print footer
    std::wcout << L"+-------+---------------------+-----------------------+" << std::endl;

    return 0;
}
