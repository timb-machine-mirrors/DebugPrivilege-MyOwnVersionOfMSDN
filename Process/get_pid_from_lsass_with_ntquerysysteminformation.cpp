// Credits to: https://www.mdsec.co.uk/2022/08/fourteen-ways-to-read-the-pid-for-the-local-security-authority-subsystem-service-lsass/
// I've tweaked the code here and there, but credits goes to the folks at MDSec
// Title: Enumerate the PID of lsass.exe via NtQuerySystemInformation

#include <Windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <iostream>
#include <string.h>
#include <vector>
#include <memory>

// NtQuerySystemInformation function prototype
typedef NTSTATUS(WINAPI* PNtQuerySystemInformation)(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
    );

// Use NtQuerySystemInformation(SystemProcessInformation) to get the process ID of lsass.exe
DWORD GetLsaPidFromName() {
	NTSTATUS status;
	DWORD processId = 0, length = 1024;
	std::unique_ptr<BYTE[]> processList;

    // Get the address of NtQuerySystemInformation
    PNtQuerySystemInformation NtQuerySystemInformation = (PNtQuerySystemInformation)GetProcAddress(
        GetModuleHandle(L"ntdll.dll"),
        "NtQuerySystemInformation"
    );

    if (!NtQuerySystemInformation) {
        std::cout << "GetProcAddress(NtQuerySystemInformation) failed: " << GetLastError() << std::endl;
        return 0;
    }

    // Allocate memory for the process list
    processList = std::make_unique<BYTE[]>(length);

    // Retrieve the list of processes
    status = NtQuerySystemInformation(
        SystemProcessInformation,
        processList.get(),
        length,
        &length
    );

    if (status == STATUS_INFO_LENGTH_MISMATCH) {
        // The buffer was too small, so allocate a new one with the correct size
        processList = std::make_unique<BYTE[]>(length);

        // Retrieve the list of processes again
        status = NtQuerySystemInformation(
            SystemProcessInformation,
            processList.get(),
            length,
            &length
        );
    }

    if (status != STATUS_SUCCESS) {
        std::cout << "NtQuerySystemInformation failed: " << status << std::endl;
        return 0;
    }

    DWORD offset = 0;
    PSYSTEM_PROCESS_INFORMATION processEntry = NULL;

    // Iterate through the list of processes
    do {
        processEntry = (PSYSTEM_PROCESS_INFORMATION)(processList.get() + offset);

        // Convert the process name to a wide string
        std::wstring processName(processEntry->ImageName.Buffer, processEntry->ImageName.Length / sizeof(wchar_t));

        // Check if the process name is "lsass.exe"
        if (_wcsicmp(processName.c_str(), L"lsass.exe") == 0) {
            // If the process name matches, return the process ID
            DWORD processId = HandleToUlong(processEntry->UniqueProcessId);
            if (processId == 0) {
                DWORD error = GetLastError();
                std::cout << "HandleToUlong failed: " << error << std::endl;
            }
            return processId;
        }

        offset += processEntry->NextEntryOffset;
    } while (processEntry->NextEntryOffset);

    return processId;
}

int main() {
    // Get the process ID of lsass.exe
    DWORD processId = GetLsaPidFromName();
    std::cout << "Process ID of lsass.exe: " << processId << std::endl;
    return 0;
}
