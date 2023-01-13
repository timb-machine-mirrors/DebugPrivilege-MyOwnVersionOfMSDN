#include <Windows.h>
#include <tchar.h>
#include <WtsApi32.h>
#include <iostream>

#pragma comment(lib, "WtsApi32.lib")

DWORD GetLsaPidFromWTS() {
    DWORD ProcessId = 0;
    PWTS_PROCESS_INFO process_info = NULL;
    DWORD process_count = 0;
    BOOL result;

    result = WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, NULL, 1, &process_info, &process_count);

    if (!result) {
        std::cout << "WTSEnumerateProcesses() failed : " << GetLastError() << std::endl;
        return ProcessId;
    }

    for (DWORD i = 0; i < process_count; i++) {
        if (!lstrcmpiW(process_info[i].pProcessName, L"lsass.exe")) {
            ProcessId = process_info[i].ProcessId;
            break;
        }
    }

    WTSFreeMemory(process_info);
    return ProcessId;
}

int main() {
    DWORD process_id = GetLsaPidFromWTS();
    std::cout << "Process ID of lsass.exe: " << process_id << std::endl;
    return 0;
}
