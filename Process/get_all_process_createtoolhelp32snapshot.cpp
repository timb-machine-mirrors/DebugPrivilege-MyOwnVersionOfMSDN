#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>

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
    for (const auto& info : processInfos)
    {
        std::wcout << "PID: " << info.processPID << std::endl;
        std::wcout << "ProcessName: " << info.processName << std::endl;
    }
}
