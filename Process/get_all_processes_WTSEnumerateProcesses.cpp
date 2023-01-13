#include <Windows.h>
#include <tchar.h>
#include <WtsApi32.h>
#include <iostream>

#pragma comment(lib, "WtsApi32.lib")

/* This function enumerates all processes running on the current server and
   prints the process name and process id of each process */
void EnumerateAllProcesses() {
    PWTS_PROCESS_INFO info = nullptr; // pointer to an array of process information
    DWORD cnt = 0;  // variable to store the number of processes
    BOOL Result;

    Result = WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &info, &cnt);

    if (!Result) {
        std::cout << "WTSEnumerateProcesses() failed : " << GetLastError() << std::endl;
        return;
    }

    for (DWORD i = 0; i < cnt; i++) {
        std::wcout << "Process name: " << info[i].pProcessName << ", PID: " << info[i].ProcessId << std::endl;
    }
    // Frees the memory allocated to the process information array
    WTSFreeMemory(info);
}

int main() {
    EnumerateAllProcesses();
    return 0;
}
