#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <sstream>

struct ProcessInfo {
	DWORD processPID = 0;
	std::wstring processName;
};

ProcessInfo getProcessInfo()
{
	HANDLE snap_handler = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processEntry = {};
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	std::string lsass_processname = "lsass.exe";
	std::wstring processname(lsass_processname.begin(), lsass_processname.end());
	const wchar_t* szName = processname.c_str();
	ProcessInfo info;
    if (Process32First(snap_handler, &processEntry))
    {
        while (_wcsicmp(processEntry.szExeFile, szName) != 0)
        {
            if (!Process32Next(snap_handler, &processEntry))
            {
                break;
            }
        }
        if (_wcsicmp(processEntry.szExeFile, szName) == 0)
        {
            info.processPID = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
        }
    }
    // Check for errors if (info.processPID == 0)
    {
        DWORD error = GetLastError();
        if (error != ERROR_SUCCESS)
        {
            std::cerr << "Error getting process PID: " << error << std::endl;
        }
    }
    CloseHandle(snap_handler);
    processname.clear();
    return info;
}
int main(int argc, char** argv)
{
    // Obtains LSASS Process info
    ProcessInfo info = getProcessInfo();
    std::cout << "PID: " << info.processPID << std::endl;
    // Print the PID and name to the console
    std::wcout << "ProcessName: " << info.processName << std::endl;
}
