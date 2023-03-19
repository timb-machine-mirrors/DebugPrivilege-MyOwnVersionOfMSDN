// Just a simple example on calling GetAddrInfoExW to resolve a hostname to one or more IP addresses and retrieve related address information

#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <string>
#include <vector>
#include <iomanip>

#pragma comment(lib, "Ws2_32.lib")

typedef struct AddressInfo {
    std::wstring canonicalName;
    std::string ipAddress;
    int socketType;
    int protocol;
} AddressInfo;

std::vector<AddressInfo> addressInfos;

void CALLBACK GetAddrInfoExCompletion(
    DWORD Error,
    DWORD Bytes,
    LPWSAOVERLAPPED Overlapped
) {
    PADDRINFOEXW result = (PADDRINFOEXW)Overlapped->hEvent;

    if (Error != 0) {
        std::cout << "GetAddrInfoExW failed. Error: " << Error << std::endl;
    }
    else {
        std::wcout << L"Address information retrieved successfully." << std::endl;

        // Print table header
        std::wcout << std::left << std::setw(40) << L"Canonical Name"
            << std::setw(40) << L"IP Address"
            << std::setw(15) << L"Socket Type"
            << std::setw(10) << L"Protocol"
            << std::endl;
        std::wcout << std::wstring(105, L'-') << std::endl;

        // Iterate through the results and print the information
        for (PADDRINFOEXW addr = result; addr != nullptr; addr = addr->ai_next) {
            char ip[INET6_ADDRSTRLEN] = { 0 };

            if (addr->ai_family == AF_INET) {
                sockaddr_in* ipv4 = (sockaddr_in*)addr->ai_addr;
                inet_ntop(addr->ai_family, &ipv4->sin_addr, ip, sizeof(ip));
            }
            else if (addr->ai_family == AF_INET6) {
                sockaddr_in6* ipv6 = (sockaddr_in6*)addr->ai_addr;
                inet_ntop(addr->ai_family, &ipv6->sin6_addr, ip, sizeof(ip));
            }

            std::wstring wip = std::wstring(ip, ip + strlen(ip));

            std::wcout << std::left << std::setw(40) << addr->ai_canonname
                << std::setw(40) << wip
                << std::setw(15) << addr->ai_socktype
                << std::setw(10) << addr->ai_protocol
                << std::endl;
        }

        // Free the address information
        FreeAddrInfoExW(result);
    }
}

void printAddressInfos() {
    std::cout << std::left << std::setw(20) << "Canonical Name"
        << std::setw(40) << "IP Address"
        << std::setw(12) << "Socket Type"
        << std::setw(10) << "Protocol" << std::endl;

    std::cout << std::string(82, '-') << std::endl;

    for (const auto& info : addressInfos) {
        std::wcout << std::left << std::setw(20) << info.canonicalName;
        std::cout << std::setw(40) << info.ipAddress
            << std::setw(12) << info.socketType
            << std::setw(10) << info.protocol << std::endl;
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cout << "WSAStartup failed. Error: " << result << std::endl;
        return 1;
    }

    ADDRINFOEXW hints = { 0 };
    hints.ai_family = AF_UNSPEC; // Both IPv4 and IPv6 addresses
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME; // Request canonical name

    // Overlapped structure for GetAddrInfoExW
    WSAOVERLAPPED overlapped = { 0 };

    // Query address information using GetAddrInfoExW
    HANDLE hLookup;
    result = GetAddrInfoExW(
        L"portal.azure.com", // Hostname to resolve
        nullptr,
        NS_ALL,
        nullptr,
        &hints,
        (PADDRINFOEXW*)&overlapped.hEvent,
        nullptr,
        &overlapped,
        GetAddrInfoExCompletion, // Completion routine
        &hLookup
    );

    if (result == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();

        if (errorCode != WSA_IO_PENDING) {
            std::cout << "GetAddrInfoExW failed. Error: " << errorCode << std::endl;
            WSACleanup();
            return 1;
        }
    }

    // Wait for the completion routine to be called
    SleepEx(INFINITE, TRUE);

    // Print the address information in a table format
    printAddressInfos();

    // Clean up Winsock
    WSACleanup();

    return 0;
}
