#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")

typedef struct _ICMP_HEADER {
    BYTE type;
    BYTE code;
    USHORT checksum;
    USHORT id;
    USHORT seq;
} ICMP_HEADER;

#define ICMP_ECHO_REQUEST 8

int main(int argc, char* argv[])
{
    // Check command-line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <IP address>" << std::endl;
        return 1;
    }

    // Initialize WinSock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // Resolve IP address
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = 0;
    wchar_t wide_ip[INET_ADDRSTRLEN];
    if (MultiByteToWideChar(CP_ACP, 0, argv[1], -1, wide_ip, INET_ADDRSTRLEN) == 0 ||
        InetPton(AF_INET, wide_ip, &target.sin_addr) != 1) {
        std::cerr << "Invalid IP address: " << argv[1] << std::endl;
        return 1;
    }

    // Create raw socket for ICMP protocol
    SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Build ICMP echo request packet
    const int packet_size = sizeof(ICMP_HEADER);
    char packet[packet_size];
    memset(packet, 0, packet_size);
    ICMP_HEADER* header = (ICMP_HEADER*)packet;
    header->type = ICMP_ECHO_REQUEST;
    header->code = 0;
    header->checksum = 0;
    header->id = (USHORT)GetCurrentProcessId();
    header->seq = 0;
    const int data_size = packet_size - sizeof(ICMP_HEADER);
    char* data = packet + sizeof(ICMP_HEADER);
    memset(data, 'A', data_size);

    // Calculate checksum
    USHORT* buf = (USHORT*)packet;
    ULONG sum = 0;
    int count = packet_size / 2;
    while (count--) {
        sum += *buf++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    header->checksum = (USHORT)~sum;

    // Send ICMP echo request packet
    int num_sent_packets = 0;
    int send_size = sendto(sock, packet, packet_size, 0, (struct sockaddr*)&target, sizeof(target));
    if (send_size == SOCKET_ERROR) {
        std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    num_sent_packets++;
    std::cout << "ICMP echo request sent to " << argv[1] << std::endl;

    // Receive ICMP echo reply packet
    const int reply_packet_size = 512;
    char reply_packet[reply_packet_size];
    struct sockaddr_in from;
    int fromlen = sizeof(from);
    int num_recv_packets = 0;
    int recv_size;

    while (num_sent_packets > num_recv_packets) {
        recv_size = recvfrom(sock, reply_packet, reply_packet_size, 0, (struct sockaddr*)&from, &fromlen);
        if (recv_size == SOCKET_ERROR) {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        // Convert IP address from binary to string format
        char from_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &from.sin_addr, from_ip, INET_ADDRSTRLEN);

        std::cout << "ICMP echo reply received from " << from_ip << std::endl;
        num_recv_packets++;
    }

    // Display number of packets sent and received
    std::cout << "Sent " << num_sent_packets << " packets, received " << num_recv_packets << " packets." << std::endl;

    // Cleanup WinSock
    closesocket(sock);
    WSACleanup();

    return 0;
}
