#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
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
        // No error message output
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

void printModuleTableHeader() {
    std::wcout << L"+-------+---------------------+--------------------+" << std::endl;
    std::wcout << L"| PID   | Process Name        | Module Name        |" << std::endl;
    std::wcout << L"+-------+---------------------+--------------------+" << std::endl;
}

void printModuleTableFooter() {
    std::wcout << L"+-------+---------------------+--------------------+" << std::endl;
}

std::wstring toLower(const std::wstring& str) {
    std::wstring lowerStr(str);
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
    return lowerStr;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <process_name>" << std::endl;
        return 1;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring targetProcessName = converter.from_bytes(argv[1]);

    std::vector<ProcessInfo> processInfos = getAllProcessInfo();

    // Set the locale to UTF-8 to support wide characters
    std::locale::global(std::locale(""));

    bool foundProcess = false;
    std::wstring lowerTargetProcessName = toLower(targetProcessName);

    for (const auto& info : processInfos)
    {
        std::wstring lowerProcessName = toLower(info.processName);

        if (lowerProcessName == lowerTargetProcessName) {
            foundProcess = true;
            printModuleTableHeader();

            for (const auto& module : info.modules)
            {
                std::string processName = converter.to_bytes(info.processName);
                std::string moduleName = converter.to_bytes(module.moduleName);

                std::printf("| %5d | %-20s | %-20s |\n", info.processPID, processName.c_str(), moduleName.c_str());
            }

            printModuleTableFooter();
            break;
        }
    }

    if (!foundProcess) {
        std::cerr << "Process not found: " << argv[1] << std::endl;
    }

    return 0;
}
