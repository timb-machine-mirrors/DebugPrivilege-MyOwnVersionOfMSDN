// Get integrity level of a process: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/81d92bba-d22b-4a8c-908a-554ab29148ab

#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <locale>
#include <codecvt>
#include <string>
#include <sddl.h>


typedef struct ProcessInfo {
    DWORD processPID = 0;
    std::wstring processName;
    std::wstring integrityLevel;
} ProcessInfo;

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
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processEntry.th32ProcessID);
            if (hProcess == NULL) {
                continue;
            }
            HANDLE hToken = NULL;
            if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken) == FALSE) {
                continue;
            }
            DWORD cbSize = 0;
            if (GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &cbSize) == FALSE && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                continue;
            }
            PTOKEN_MANDATORY_LABEL pTIL = (PTOKEN_MANDATORY_LABEL)new BYTE[cbSize];
            if (pTIL == NULL) {
                continue;
            }
            if (GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, cbSize, &cbSize) == FALSE) {
                delete[] pTIL;
                continue;
            }
            PSID integrityLevel = pTIL->Label.Sid;
            wchar_t* szIntegrityLevel = NULL;
            if (ConvertSidToStringSid(integrityLevel, &szIntegrityLevel) == FALSE) {
                delete[] pTIL;
                continue;
            }
            info.processPID = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
            info.integrityLevel = szIntegrityLevel;
            processInfos.push_back(info);
            delete[] pTIL;
            LocalFree(szIntegrityLevel);
            CloseHandle(hToken);
            CloseHandle(hProcess);
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
    std::wcout << L"+-------+---------------------+---------------------+" << std::endl;
    std::wcout << L"| PID   | Process Name        | Integrity Level     |" << std::endl;
    std::wcout << L"+-------+---------------------+---------------------+" << std::endl;

    // Print process and integrity level information in a table
    for (const auto& info : processInfos)
    {
        std::string processName = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(info.processName);
        std::string integrityLevel = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(info.integrityLevel);

        std::printf("| %5d | %-20s | %-20s |\n", info.processPID, processName.c_str(), integrityLevel.c_str());
    }

    // Print footer
    std::wcout << L"+-------+---------------------+---------------------+" << std::endl;

    return 0;
}
