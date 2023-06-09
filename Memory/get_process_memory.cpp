#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <psapi.h>

struct ProcessInfo {
    DWORD processPID = 0;
    std::wstring processName;
    SIZE_T workingSetSize = 0;
    SIZE_T peakWorkingSetSize = 0;
    SIZE_T pagefileUsage = 0;
    SIZE_T peakPagefileUsage = 0;
    SIZE_T quotaPagedPoolUsage = 0;
    SIZE_T quotaPeakPagedPoolUsage = 0;
    SIZE_T quotaNonPagedPoolUsage = 0;
    SIZE_T quotaPeakNonPagedPoolUsage = 0;
    DWORD pageFaultCount = 0;
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
                PROCESS_MEMORY_COUNTERS_EX memoryCounters{};
                if (GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*)&memoryCounters, sizeof(memoryCounters))) {
                    ProcessInfo info;
                    info.processPID = processEntry.th32ProcessID;
                    info.processName = processEntry.szExeFile;
                    info.workingSetSize = memoryCounters.WorkingSetSize;
                    info.peakWorkingSetSize = memoryCounters.PeakWorkingSetSize;
                    info.pagefileUsage = memoryCounters.PagefileUsage;
                    info.peakPagefileUsage = memoryCounters.PeakPagefileUsage;
                    info.quotaPagedPoolUsage = memoryCounters.QuotaPagedPoolUsage;
                    info.quotaPeakPagedPoolUsage = memoryCounters.QuotaPeakPagedPoolUsage;
                    info.quotaNonPagedPoolUsage = memoryCounters.QuotaNonPagedPoolUsage;
                    info.quotaPeakNonPagedPoolUsage = memoryCounters.QuotaPeakNonPagedPoolUsage;
                    info.pageFaultCount = memoryCounters.PageFaultCount;
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
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.processPID);
        if (hProcess != NULL)
        {
            PROCESS_MEMORY_COUNTERS_EX pmc{};
            if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
            {
                std::wcout << "PID: " << info.processPID << std::endl;
                std::wcout << "ProcessName: " << info.processName << std::endl;
                std::wcout << "\tWorkingSetSize: " << info.workingSetSize / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tPeakWorkingSetSize: " << pmc.PeakWorkingSetSize / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tPagefileUsage: " << info.pagefileUsage / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tPeakPagefileUsage: " << info.peakPagefileUsage / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tQuotaPagedPoolUsage: " << pmc.QuotaPagedPoolUsage / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tQuotaPeakPagedPoolUsage: " << pmc.QuotaPeakPagedPoolUsage / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tQuotaNonPagedPoolUsage: " << pmc.QuotaNonPagedPoolUsage / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tQuotaPeakNonPagedPoolUsage: " << pmc.QuotaPeakNonPagedPoolUsage / (1024 * 1024) << " MB" << std::endl;
                std::wcout << "\tPageFaultCount: " << pmc.PageFaultCount << std::endl;
            }
            CloseHandle(hProcess);
        }
    }
    return 0;
}
