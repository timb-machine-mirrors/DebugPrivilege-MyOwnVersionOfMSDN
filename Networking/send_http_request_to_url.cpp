// Example: SendHTTP.exe portal.azure.com

#include <Windows.h>
#include <WinINet.h>
#include <iostream>
#include <string>

#pragma comment(lib, "wininet.lib")

typedef struct {
    std::string url;
} HTTP_REQUEST_PARAMS;

BOOL parseCommandLine(int argc, char* argv[], HTTP_REQUEST_PARAMS* pParams)
{
    if (argc < 2)
        return FALSE;

    pParams->url = argv[1];

    return TRUE;
}

int main(int argc, char* argv[])
{
    HTTP_REQUEST_PARAMS params = { "" };

    if (!parseCommandLine(argc, argv, &params)) {
        std::cout << "Usage: " << argv[0] << " <url>" << std::endl;
        return 1;
    }

    // Open an HTTP session
    HINTERNET hSession = InternetOpenA("HTTPGET", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hSession == NULL) {
        std::cout << "Failed to open HTTP session" << std::endl;
        return 1;
    }

    // Open a connection to the server
    HINTERNET hConnect = InternetConnectA(hSession, params.url.c_str(), INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
    if (hConnect == NULL) {
        std::cout << "Failed to connect to server" << std::endl;
        InternetCloseHandle(hSession);
        return 1;
    }

    // Create an HTTP request
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", "/", NULL, NULL, NULL, INTERNET_FLAG_RELOAD, 1);
    if (hRequest == NULL) {
        std::cout << "Failed to create HTTP request" << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return 1;
    }

    // Send the HTTP request
    BOOL bRequestSent = HttpSendRequestA(hRequest, NULL, 0, NULL, 0);
    if (!bRequestSent) {
        std::cout << "Failed to send HTTP request" << std::endl;
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return 1;
    }

    // Read the HTTP response
    char buffer[1024];
    DWORD dwRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &dwRead) && dwRead) {
        buffer[dwRead] = '\0';
        std::cout << buffer;
        dwRead = 0;
    }

    // Close the HTTP request, connection, and session handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return 0;
}
