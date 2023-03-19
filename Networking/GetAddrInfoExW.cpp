# Just a simple example on calling GetAddrInfoExW to resolve a hostname to one or more IP addresses and retrieve related address information

#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

// Completion routine for GetAddrInfoExW
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
        std::cout << "Address information retrieved successfully." << std::endl;

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

            std::wcout << L"Canonical Name: " << addr->ai_canonname << std::endl;
            std::cout << "IP Address: " << ip << std::endl;
            std::cout << "Socket Type: " << addr->ai_socktype << std::endl;
            std::cout << "Protocol: " << addr->ai_protocol << std::endl;
            std::cout << "----------------------" << std::endl;
        }

        // Free the address information
        FreeAddrInfoExW(result);
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
        L"portal.azure.com", // Hostname to resolve, as example. We use the Azure Portal
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

    // Clean up Winsock
    WSACleanup();

    return 0;
}
