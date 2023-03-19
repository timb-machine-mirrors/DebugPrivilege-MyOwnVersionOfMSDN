# Example on establishing a network connection by using the WinSock APIs. This code sends an HTTP GET request using the send function. 
# It then enters a loop to receive the server's response using the recv function.

#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cout << "WSAStartup failed. Error: " << result << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

    if (sock == INVALID_SOCKET) {
        std::cout << "WSASocketW failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Set up the sockaddr_in structure for the remote server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80); // Port number (e.g., 80 for HTTP)

    // Convert the IPv4 address string to binary format
    result = inet_pton(AF_INET, "93.184.216.34", &serverAddr.sin_addr);

    if (result <= 0) {
        std::cout << "inet_pton failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Connect to the remote server
    result = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    if (result == SOCKET_ERROR) {
        std::cout << "Connection failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the remote server successfully." << std::endl;

    // Send an HTTP GET request
    const char* httpRequest = "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";
    result = send(sock, httpRequest, strlen(httpRequest), 0);

    if (result == SOCKET_ERROR) {
        std::cout << "Failed to send the request. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Request sent successfully." << std::endl;

    // Receive the response
    char buffer[4096];
    int bytesReceived;

    while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << buffer;
    }

    if (bytesReceived < 0) {
        std::cout << "Failed to receive the response. Error: " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "Response received successfully." << std::endl;
    }

    // Close the socket
    closesocket(sock);

    // Clean up Winsock
    WSACleanup();

    return 0;
}
