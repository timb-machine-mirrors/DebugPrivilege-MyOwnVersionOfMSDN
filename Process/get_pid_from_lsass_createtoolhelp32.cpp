#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <stdexcept>
#include <memory>

// A class that holds information about a process
class ProcessInfo
{
public:
    DWORD processID = 0; // The ID of the process
    std::wstring processName; // The name of the process
};

// A class that represents a Windows error
class WindowsError : public std::runtime_error
{
public:
    // Constructor that initializes the error message and error code
    WindowsError(DWORD errorCode) : std::runtime_error(getErrorMessage(errorCode)), errorCode_(errorCode) {}
    // Getter for the error code
    DWORD getErrorCode() const { return errorCode_; }

private:
    // Helper function that converts a Windows error code to an error message string
    static std::string getErrorMessage(DWORD errorCode)
    {
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);

        //Free the buffer.
        LocalFree(messageBuffer);

        return message;
    }
    DWORD errorCode_;
};

// A function that returns information about a process with the specified name
std::unique_ptr<ProcessInfo> getProcessInfo(const std::string& processName)
{
    // Check if the processName is empty
    if (processName.empty())
        throw std::invalid_argument("processName cannot be empty");

    // Create a snapshot of all processes
    HANDLE snap_handler = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap_handler == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        throw WindowsError(error);
    }

    // Create a unique pointer to store the process information
    std::unique_ptr<ProcessInfo> info(new ProcessInfo);

    // Convert processName to wide string
    std::wstring processNameWide(processName.begin(), processName.end());
    const wchar_t* szName = processNameWide.c_str();

    // Initialize the process entry
    PROCESSENTRY32 processEntry = { sizeof(PROCESSENTRY32) };
    if (Process32First(snap_handler, &processEntry))
    {
        while (Process32Next(snap_handler, &processEntry))
        {
            // Check if the process name matches the specified name
            if (_wcsicmp(processEntry.szExeFile, szName) == 0)
            {
                // If the process name matches, store the process ID and name in the ProcessInfo object
                info->processID = processEntry.th32ProcessID;
                info->processName = processEntry.szExeFile;
                break;
            }
        }
    }

    // If the process ID is still 0, it means that the process is not found
    if (info->processID == 0)
    {
        DWORD error = GetLastError();
        throw WindowsError(error);
    }

    // Close the snapshot handle
    CloseHandle(snap_handler);
    return info;
}

int main(int argc, char** argv)
{
    try
    {
        // Get the process information for "lsass.exe"
        std::unique_ptr<ProcessInfo> info = getProcessInfo("lsass.exe");
        // Print the process ID
        std::cout << "PID: " << info->processID << std::endl;
        // Print the process name
        std::wcout << "ProcessName: " << info->processName << std::endl;
    }
    catch (const WindowsError& e)
    {
        // Catch WindowsError exception and print error message and error code
        std::cerr << "Error: " << e.what() << " (Error code: " << e.getErrorCode() << ")" << std::endl;
        return 1;
    }
    catch (const std::exception& e)
    {
        // Catch other exceptions and print error message
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
