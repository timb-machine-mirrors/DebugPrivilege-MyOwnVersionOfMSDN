#include <iostream>
#include <fstream>
#include <Windows.h>
#include <WinInet.h>
#include <string>

#pragma comment(lib, "wininet.lib")

typedef struct {
    std::wstring url;
    std::wstring save_path;
} DownloadInfo;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <url> <save_path>\n";
        std::cerr << "Example: Download.exe https://github.com/0xlane/BypassUAC/raw/master/x64/Release/BypassUAC.exe C:\\Temp\\BypassUAC.exe\n";
        return 1;
    }

    DownloadInfo download_info;
    download_info.url = std::wstring(argv[1], argv[1] + strlen(argv[1]));
    download_info.save_path = std::wstring(argv[2], argv[2] + strlen(argv[2]));

    HINTERNET hInternet = InternetOpen(L"", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) {
        std::cerr << "InternetOpen failed. Error: " << GetLastError() << std::endl;
        return 1;
    }

    HINTERNET hUrl = InternetOpenUrl(hInternet, download_info.url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        std::cerr << "InternetOpenUrl failed. Error: " << GetLastError() << std::endl;
        InternetCloseHandle(hInternet);
        return 1;
    }

    std::ofstream output_file(std::string(download_info.save_path.begin(), download_info.save_path.end()), std::ios::binary);
    if (!output_file) {
        std::cerr << "Failed to open output file." << std::endl;
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return 1;
    }

    constexpr DWORD buffer_size = 4096;
    char buffer[buffer_size];
    DWORD bytes_read;

    while (InternetReadFile(hUrl, buffer, buffer_size, &bytes_read) && bytes_read > 0) {
        output_file.write(buffer, bytes_read);
    }

    output_file.close();
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    std::cout << "File downloaded successfully.\n";
    return 0;
}
