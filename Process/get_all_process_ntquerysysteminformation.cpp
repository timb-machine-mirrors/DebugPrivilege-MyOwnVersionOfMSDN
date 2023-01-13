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

void EnumerateProcesses() {
    NTSTATUS status;
    DWORD length = 1024;
    std::unique_ptr<BYTE[]> processList;

    // Get the address of NtQuerySystemInformation
    PNtQuerySystemInformation NtQuerySystemInformation = (PNtQuerySystemInformation)GetProcAddress(
        GetModuleHandle(L"ntdll.dll"),
        "NtQuerySystemInformation"
    );

    if (!NtQuerySystemInformation) {
        std::cout << "GetProcAddress(NtQuerySystemInformation) failed: " << GetLastError() << std::endl;
        return;
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
        return;
    }

    DWORD offset = 0;
    PSYSTEM_PROCESS_INFORMATION processEntry = NULL;

    // Iterate through the list of processes
    do {
        processEntry = (PSYSTEM_PROCESS_INFORMATION)(processList.get() + offset);

        // Convert the process name to a wide string
        std::wstring processName(processEntry->ImageName.Buffer, processEntry->ImageName.Length / sizeof(wchar_t));
        DWORD processId = HandleToUlong(processEntry->UniqueProcessId);
        if (processId == 0) {
            DWORD error = GetLastError();
            std::cout << "HandleToUlong failed: " << error << std::endl;
        }
        // Print the process name and process ID
        std::wcout << "Process Name: " << processName.c_str() << " PID: " << processId << std::endl;

        offset += processEntry->NextEntryOffset;
    } while (processEntry->NextEntryOffset);
}

int main() {
    EnumerateProcesses();
    return 0;
}
