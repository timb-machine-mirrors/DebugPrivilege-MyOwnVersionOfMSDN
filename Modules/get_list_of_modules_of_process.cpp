#include <iostream>
#include <locale>
#include <codecvt>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>

struct ModuleInfo {
    DWORD modulePID = 0;
    std::wstring moduleName;
};

struct ProcessInfo {
    DWORD processPID = 0;
    std::wstring processName;
    std::vector<ModuleInfo> modules;
};

std::vector<ModuleInfo> getProcessModules(DWORD processID)
{
    std::vector<ModuleInfo> moduleInfos;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);

    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Error creating snapshot: " << GetLastError() << std::endl;
        return moduleInfos;
    }

    MODULEENTRY32 moduleEntry = { sizeof(MODULEENTRY32) };
    if (Module32First(snapshot, &moduleEntry)) {
        while (Module32Next(snapshot, &moduleEntry)) {
            ModuleInfo info;
            info.modulePID = moduleEntry.th32ModuleID;
            // Extract only the .dll name from the full path
            std::wstring moduleName(moduleEntry.szModule);
            size_t lastBackslash = moduleName.find_last_of(L"\\");
            if (lastBackslash != std::wstring::npos) {
                moduleName = moduleName.substr(lastBackslash + 1);
            }
            info.moduleName = moduleName;
            moduleInfos.push_back(info);
        }
    }
    else {
        std::cerr << "Error getting module information: " << GetLastError() << std::endl;
    }

    CloseHandle(snapshot);
    return moduleInfos;
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
            ProcessInfo info;
            info.processPID = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
            info.modules = getProcessModules(processEntry.th32ProcessID);
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
    std::wcout << L"+-------+---------------------+--------------------+" << std::endl;
    std::wcout << L"| PID   | Process Name        | Module Name        |" << std::endl;
    std::wcout << L"+-------+---------------------+--------------------+" << std::endl;

    // Convert wide strings to narrow
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;

    // Print process and module information in a table
    for (const auto& info : processInfos)
    {
        for (const auto& module : info.modules)
        {
            std::string processName = converter.to_bytes(info.processName);
            std::string moduleName = converter.to_bytes(module.moduleName);

            std::printf("| %5d | %-20s | %-20s |\n", info.processPID, processName.c_str(), moduleName.c_str());
        }
    }

    // Print footer
    std::wcout << L"+-------+---------------------+--------------------+" << std::endl;

    return 0;
}
