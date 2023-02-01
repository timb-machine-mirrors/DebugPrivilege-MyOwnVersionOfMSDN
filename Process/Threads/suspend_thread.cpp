#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>

struct ThreadID {
    DWORD threadID;
};

struct ThreadState {
    std::wstring state;
};

struct ProcessInfo {
    DWORD processPID;
    std::wstring processName;
    std::vector<ThreadID> threadIDs;
    std::vector<ThreadState> threadStates;
};

ProcessInfo getProcessInfo()
{
    HANDLE snap_handler = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap_handler == INVALID_HANDLE_VALUE) {
        return {};
    }
    PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
    std::string processName = "notepad.exe"; // Specify processname
    std::wstring wprocessName(processName.begin(), processName.end());
    ProcessInfo info = {};
    while (Process32Next(snap_handler, &processEntry)) {
        if (_wcsicmp(processEntry.szExeFile, wprocessName.c_str()) == 0) {
            info.processPID = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
            HANDLE snap_handler_threads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (snap_handler_threads == INVALID_HANDLE_VALUE) {
                CloseHandle(snap_handler);
                return {};
            }

            THREADENTRY32 threadEntry = { sizeof(THREADENTRY32) };
            while (Thread32Next(snap_handler_threads, &threadEntry)) {
                if (threadEntry.th32OwnerProcessID == info.processPID) {
                    HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
                    if (threadHandle == INVALID_HANDLE_VALUE) {
                        CloseHandle(snap_handler_threads);
                        CloseHandle(snap_handler);
                        return {};
                    }

                    DWORD threadState = SuspendThread(threadHandle);
                    ThreadID threadID = { threadEntry.th32ThreadID };
                    info.threadIDs.push_back(threadID);
                    if (threadState == 0) {
                        ThreadState threadS = { L"Suspended" };
                        info.threadStates.push_back(threadS);
                    }
                    else {
                        ThreadState threadS = { L"The Thread was running, but is now suspended" };
                        info.threadStates.push_back(threadS);
                    }
                    CloseHandle(threadHandle);
                }
            }
            CloseHandle(snap_handler_threads);
            break;
        }
    }
    CloseHandle(snap_handler);
    return info;
}

int main() {
    ProcessInfo info = getProcessInfo();
    std::cout << "PID: " << info.processPID << std::endl;
    std::wcout << "ProcessName: " << info.processName << std::endl;
    for (int i = 0; i < info.threadIDs.size(); i++) {
        std::cout << "Thread ID: " << info.threadIDs[i].threadID << std::endl;
        std::wcout << "Thread State: " << info.threadStates[i].state << std::endl;
        HANDLE threadHandle = NULL;
        threadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, info.threadIDs[i].threadID);
        if (threadHandle == NULL) {
            std::cout << "Failed to open thread" << std::endl;
            continue;
        }
        if (SuspendThread(threadHandle) == -1) {
            std::cout << "Failed to suspend thread" << std::endl;
            continue;
        }
        CloseHandle(threadHandle);
    }
    return 0;
}
