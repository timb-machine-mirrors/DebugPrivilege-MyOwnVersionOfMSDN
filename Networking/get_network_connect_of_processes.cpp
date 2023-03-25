#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <Ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")

typedef struct TcpConnection {
    DWORD processId;
    std::string processName;
    std::string localAddress;
    USHORT localPort;
    std::string remoteAddress;
    USHORT remotePort;
    std::string state;
} TcpConnection;

const char* tcpStateToString(DWORD state)
{
    switch (state)
    {
    case MIB_TCP_STATE_CLOSED:
        return "CLOSED";
    case MIB_TCP_STATE_LISTEN:
        return "LISTEN";
    case MIB_TCP_STATE_SYN_SENT:
        return "SYN_SENT";
    case MIB_TCP_STATE_SYN_RCVD:
        return "SYN_RCVD";
    case MIB_TCP_STATE_ESTAB:
        return "ESTABLISHED";
    case MIB_TCP_STATE_FIN_WAIT1:
        return "FIN_WAIT1";
    case MIB_TCP_STATE_FIN_WAIT2:
        return "FIN_WAIT2";
    case MIB_TCP_STATE_CLOSE_WAIT:
        return "CLOSE_WAIT";
    case MIB_TCP_STATE_CLOSING:
        return "CLOSING";
    case MIB_TCP_STATE_LAST_ACK:
        return "LAST_ACK";
    case MIB_TCP_STATE_TIME_WAIT:
        return "TIME_WAIT";
    case MIB_TCP_STATE_DELETE_TCB:
        return "DELETE_TCB";
    default:
        return "UNKNOWN";
    }
}


void clearConsole()
{
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

int main()
{
    // Initialize WinSock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    while (true)
    {

        // Enumerate all TCP connections
        MIB_TCPTABLE_OWNER_PID* tcpTable = NULL;
        ULONG tcpTableSize = 0;
        result = GetExtendedTcpTable(NULL, &tcpTableSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        if (result == ERROR_INSUFFICIENT_BUFFER)
        {
            tcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(tcpTableSize);
            result = GetExtendedTcpTable(tcpTable, &tcpTableSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        }

        if (result != NO_ERROR)
        {
            printf("GetExtendedTcpTable failed: %d\n", result);
            WSACleanup();
            return 1;
        }

        std::vector<TcpConnection> connections;

        // Collect TCP connections data
        for (DWORD i = 0; i < tcpTable->dwNumEntries; i++)
        {
            MIB_TCPROW_OWNER_PID tcpRow = tcpTable->table[i];
            TcpConnection connection;
            connection.processId = tcpRow.dwOwningPid;

            char localAddr[INET_ADDRSTRLEN] = { 0 };
            snprintf(localAddr, sizeof(localAddr), "%d.%d.%d.%d",
                (tcpRow.dwLocalAddr >> 0) & 0xFF,
                (tcpRow.dwLocalAddr >> 8) & 0xFF,
                (tcpRow.dwLocalAddr >> 16) & 0xFF,
                (tcpRow.dwLocalAddr >> 24) & 0xFF);
            connection.localAddress = localAddr;
            connection.localPort = ntohs((u_short)tcpRow.dwLocalPort);

            char remoteAddr[INET_ADDRSTRLEN] = { 0 };
            snprintf(remoteAddr, sizeof(remoteAddr), "%d.%d.%d.%d",
                (tcpRow.dwRemoteAddr >> 0) & 0xFF,
                (tcpRow.dwRemoteAddr >> 8) & 0xFF,
                (tcpRow.dwRemoteAddr >> 16) & 0xFF,
                (tcpRow.dwRemoteAddr >> 24) & 0xFF);
            connection.remoteAddress = remoteAddr;
            connection.remotePort = ntohs((u_short)tcpRow.dwRemotePort);
            connection.state = tcpStateToString(tcpRow.dwState);

            // Get the process name
            HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, connection.processId);
            if (processHandle != NULL)
            {
                char processName[MAX_PATH] = { 0 };
                DWORD processNameLength = MAX_PATH;
                if (QueryFullProcessImageNameA(processHandle, 0, processName, &processNameLength))
                {
                    // Strip the path and keep only the filename
                    char* fileName = strrchr(processName, '\\');
                    if (fileName != NULL)
                    {
                        fileName++;
                    }
                    else
                    {
                        fileName = processName;
                    }
                    connection.processName = fileName;
                }
                else
                {
                    connection.processName = "<unknown>";
                }

                CloseHandle(processHandle);
            }
            else
            {
                connection.processName = "<unknown>";
            }

            // Add the connection only if the process name is not <unknown>
            if (connection.processName != "<unknown>")
            {
                connections.push_back(connection);
            }
        }

        // Print the local and remote addresses and ports for each TCP connection in a table format
        clearConsole();
        printf("========================================================================================================================\n");
        printf("%-6s | %-30s | %-15s | %-10s | %-15s | %-10s | %-12s\n", "PID", "Process name", "Local address", "Local port", "Remote address", "Remote port", "State");
        printf("========================================================================================================================\n");
        for (const TcpConnection& connection : connections)
        {
            printf("%-6d | %-30s | %-15s | %-10d | %-15s | %-10d | %-12s\n",
                connection.processId,
                connection.processName.c_str(),
                connection.localAddress.c_str(),
                connection.localPort,
                connection.remoteAddress.c_str(),
                connection.remotePort,
                connection.state.c_str());
        }
        printf("========================================================================================================================\n");

        free(tcpTable);
        tcpTable = NULL;

        // Sleep for a while before the next iteration
        Sleep(10000); // Sleep for 10000 milliseconds (10 seconds)
    }

    // Clean up WinSock
    WSACleanup();

    return 0;
}
