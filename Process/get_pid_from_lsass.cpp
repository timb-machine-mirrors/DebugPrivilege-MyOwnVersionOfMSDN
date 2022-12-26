#include <iostream>
#include <windows.h>
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <sstream>

using namespace std;

DWORD getProcessPid(DWORD& processPID)
{
    HANDLE snap_handler = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    string lsass_processname = "lsass.exe";
    std::wstring processname(lsass_processname.begin(), lsass_processname.end());
    const wchar_t* szName = processname.c_str();

    if (Process32First(snap_handler, &processEntry))
    {
        do
        {
            if (_wcsicmp(processEntry.szExeFile, szName) == 0)
            {
                processPID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snap_handler, &processEntry));
    }

    // Check for errors
    if (processPID == 0)
    {
        DWORD error = GetLastError();
        if (error != ERROR_SUCCESS)
        {
            std::cerr << "Error getting process PID: " << error << std::endl;
        }
    }

    CloseHandle(snap_handler);
    processname.clear();
}

int main(int argc, char** argv)
{
    // Obtains LSASS Process PID
    DWORD processPID = 0;
    getProcessPid(processPID);
    // Print the PID to the console
    cout << "PID of lsass.exe: " << processPID << endl;
}
