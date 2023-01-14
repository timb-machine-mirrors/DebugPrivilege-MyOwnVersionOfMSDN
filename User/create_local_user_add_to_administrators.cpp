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

    std::cout << "[-] Error code: " << errorCode << std::endl;
    std::cout << "[-] Error message: " << (char*)lpMsgBuf << std::endl;
    LocalFree(lpMsgBuf);
}

int main()
{
    USER_INFO_1 user_info = {};
    DWORD dwError = 0;
    LOCALGROUP_MEMBERS_INFO_3 localgroup_members_info = {};

    std::wstring username = L"Admin";
    std::wstring password = L"Passw0rd!";
    std::wstring groupname = L"Administrators";

    // Set the user information
    user_info.usri1_name = const_cast<LPWSTR>(username.c_str());
    user_info.usri1_password = const_cast<LPWSTR>(password.c_str());
    user_info.usri1_priv = USER_PRIV_USER;
    user_info.usri1_flags = UF_SCRIPT;

    // Create the user account
    dwError = NetUserAdd(NULL, 1, (LPBYTE)&user_info, NULL);
    if (dwError == NERR_Success)
    {
        std::cout << "[+] Account created successfully" << std::endl;
        localgroup_members_info.lgrmi3_domainandname = const_cast<LPWSTR>(username.c_str());

        // add user to Administrators group
        dwError = NetLocalGroupAddMembers(NULL, groupname.c_str(), 3, (LPBYTE)&localgroup_members_info, 1);
        if (dwError == NERR_Success)
        {
            std::cout << "[+] User added to Administrators group successfully" << std::endl;
        }
        else
        {
            printError(dwError);
        }
    }
    else
    {
        printError(dwError);
    }
    return 0;
}
