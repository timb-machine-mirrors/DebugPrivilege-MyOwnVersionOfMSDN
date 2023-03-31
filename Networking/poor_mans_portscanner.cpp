// A basic port scanner

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

// Define a struct to hold the IP address and port information
typedef struct Target {
    const char* ipAddress;
    int port;
} Target;

// Function to check if a port is open for a given IP address
bool checkPortOpen(Target target) {
    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set up the target address structure
    sockaddr_in targetAddr = { 0 };
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(target.port);
    inet_pton(AF_INET, target.ipAddress, &targetAddr.sin_addr);

    // Attempt to connect to the target
    if (connect(sock, (sockaddr*)&targetAddr, sizeof(targetAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        return false;
    }

    // If the connection is successful, close the socket and return true
    closesocket(sock);
    return true;
}

int main(int argc, char* argv[]) {
    // Check for correct number of command-line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <IP address>" << std::endl;
        return 1;
    }

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Set the target IP address from the command line
    const char* targetIP = argv[1];

    // Define targets with their corresponding ports
    Target targets[] = {
        {targetIP, 3389}, // RDP
        {targetIP, 445},  // SMB
        {targetIP, 135},  // RPC
        {targetIP, 22},   // SSH
        {targetIP, 443},  // HTTPS
        {targetIP, 80},   // HTTP
        {targetIP, 53},   // DNS
        {targetIP, 5985},  // WinRM
        {targetIP, 21},  // FTP
        {targetIP, 3306}  // MYSQL
    };

    // Define service names for output
    const char* serviceNames[] = {
        "RDP",
        "SMB",
        "RPC",
        "SSH",
        "HTTPS",
        "HTTP",
        "DNS",
        "WinRM",
        "FTP",
        "MYSQL"
    };

    // Print the table header
    std::cout << std::setw(10) << std::left << "Service"
        << std::setw(10) << std::left << "Port"
        << std::setw(10) << std::left << "Status" << std::endl;
    std::cout << std::string(30, '-') << std::endl;

    // Iterate through targets and check if ports are open, then print the results in a table format
    for (int i = 0; i < sizeof(targets) / sizeof(targets[0]); ++i) {
        std::cout << std::setw(10) << std::left << serviceNames[i]
            << std::setw(10) << std::left << targets[i].port
            << std::setw(10) << std::left << (checkPortOpen(targets[i]) ? "Open" : "Closed") << std::endl;
    }

    WSACleanup();
    return 0;
}
