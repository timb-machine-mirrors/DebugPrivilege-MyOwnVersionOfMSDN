#include <iostream>
#include <Windows.h>
#include <Urlmon.h>
#include <string>

#pragma comment(lib, "urlmon.lib")

typedef struct {
    std::string url;
    std::string save_path;
} DownloadInfo;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <url> <save_path>\n";
        std::cerr << "Example: Download.exe https://github.com/0xlane/BypassUAC/blob/master/x64/Release/BypassUAC.exe?raw=true C:\\Temp\\BypassUAC.exe\n";
        return 1;
    }

    DownloadInfo download_info;
    download_info.url = argv[1];
    download_info.save_path = argv[2];

    std::wstring url_w(download_info.url.begin(), download_info.url.end());
    std::wstring save_path_w(download_info.save_path.begin(), download_info.save_path.end());

    HRESULT result = URLDownloadToFile(
        nullptr,                       // A pointer to the controlling IUnknown interface
        url_w.c_str(),                 // The URL of the file to be downloaded
        save_path_w.c_str(),           // The local file path where the file will be saved
        0,                             // Reserved and should be set to 0
        nullptr                        // A pointer to the IBindStatusCallback interface
    );

    if (result == S_OK)
    {
        std::cout << "File downloaded successfully.\n";
    }
    else
    {
        std::cout << "Error downloading file. Error code: " << std::hex << result << "\n";
    }

    return 0;
}
