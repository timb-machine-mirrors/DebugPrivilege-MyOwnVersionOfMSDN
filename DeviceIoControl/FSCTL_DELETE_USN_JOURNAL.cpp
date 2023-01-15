#include <Windows.h>
#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
    // Check if user passed in a volume name as an argument
    if (argc < 2)
    {
        _tprintf(_T("Usage: %s <volume>\n"), argv[0]);
        return 1;
    }

    // Create the volume string in the format of "\\.\VolumeName"
    _TCHAR volume[MAX_PATH];
    _sntprintf_s(volume, _countof(volume), _T("\\\\.\\%s"), argv[1]);

    // Open a handle to the specified volume
    HANDLE hVolume = CreateFile(volume, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hVolume == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("[-] Error: Failed to open handle to volume %s with error code %d\n"), argv[1], GetLastError());
        return 1;
    }

    // Retrieve the USN journal data for the volume
    USN_JOURNAL_DATA journalData;
    DWORD bytesReturned;
    if (!DeviceIoControl(hVolume, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &bytesReturned, NULL))
    {
        _tprintf(_T("[-] Error: Failed to retrieve USN journal data for volume %s with error code %d\n"), argv[1], GetLastError());
        CloseHandle(hVolume);
        return 1;
    }

    // Prepare the data needed to delete the USN journal
    DELETE_USN_JOURNAL_DATA deleteData;
    deleteData.UsnJournalID = journalData.UsnJournalID;
    deleteData.DeleteFlags = USN_DELETE_FLAG_DELETE;

    // Delete the USN journal
    if (!DeviceIoControl(hVolume, FSCTL_DELETE_USN_JOURNAL, &deleteData, sizeof(deleteData), NULL, 0, &bytesReturned, NULL))
    {
        _tprintf(_T("[-] Error: Failed to delete USN journal for volume %s with error code %d\n"), argv[1], GetLastError());
        CloseHandle(hVolume);
        return 1;
    }

    _tprintf(_T("[+] USN journal for volume %s deleted successfully!\n"), argv[1]);
    CloseHandle(hVolume);
    return 0;
}
