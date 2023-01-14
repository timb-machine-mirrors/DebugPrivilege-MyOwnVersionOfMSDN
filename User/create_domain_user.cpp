#include <Windows.h>
#include <lm.h>
#include <string>
#include <iostream>
#pragma comment(lib, "netapi32.lib")

void printError(DWORD errorCode) {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    std::cout << "Error code: " << errorCode << std::endl;
    std::cout << "Error message: " << (char*)lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
}

int main()
{
    DWORD dwError = 0;
    LPUSER_INFO_1 pBuf = NULL;
    std::wstring username = L"newuser";
    std::wstring password = L"Passw0rd!";
    std::wstring domain = L"contoso"; // Domain name
    std::wstring fullUsername = domain + L"\\" + username;
    LPCWSTR server = L"DC02"; // Server name of a Domain Controller

    // Allocate memory for the user information structure
    pBuf = (LPUSER_INFO_1)GlobalAlloc(GPTR, sizeof(USER_INFO_1));
    if (pBuf == NULL)
    {
        std::cout << "Failed to allocate memory for the user information structure" << std::endl;
        return 1;
    }

    // Set the user information
    pBuf->usri1_name = const_cast<LPWSTR>(username.c_str());
    pBuf->usri1_password = const_cast<LPWSTR>(password.c_str());
    pBuf->usri1_priv = USER_PRIV_USER;
    pBuf->usri1_flags = UF_SCRIPT;

    // Create the user account
    dwError = NetUserAdd(server, 1, (LPBYTE)pBuf, NULL);
    if (dwError == NERR_Success)
    {
        std::cout << "[+] Account created successfully" << std::endl;
    }
    else
    {
        printError(dwError);
    }

    // Free the allocated memory
    GlobalFree(pBuf);

    return 0;
}
