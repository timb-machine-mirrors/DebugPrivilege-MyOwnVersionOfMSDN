#include <Windows.h>
#include <tchar.h>

// Function to print the error message corresponding to the error code
void printErrorMessage(DWORD errorCode) {
    switch (errorCode) {
    case ERROR_FILE_NOT_FOUND:
        _tprintf(_T("The specified file was not found.\n"));
        break;
    case ERROR_ACCESS_DENIED:
        _tprintf(_T("Access is denied.\n"));
        break;
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc < 2)
    {
        _tprintf(_T("Usage: %s <volume>\n"), argv[0]);
        return 1;
    }

    _TCHAR volume[MAX_PATH];
    _sntprintf_s(volume, _countof(volume), _T("\\\\.\\%s"), argv[1]);

    // Open a handle to the specified volume
    HANDLE hVolume = CreateFile(volume, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hVolume == INVALID_HANDLE_VALUE)
    {
        printErrorMessage(GetLastError());
        return 1;
    }

    USN_JOURNAL_DATA journalData;
    DWORD bytesReturned;

    // Try to open existing journal
    if (!DeviceIoControl(hVolume, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &bytesReturned, NULL))
    {
        if (GetLastError() == ERROR_JOURNAL_NOT_ACTIVE)
        {
            _tprintf(_T("Creating new USN journal\n"));

            CREATE_USN_JOURNAL_DATA createData = { 0, 0 };

            if (!DeviceIoControl(hVolume, FSCTL_CREATE_USN_JOURNAL, &createData, sizeof(createData), NULL, 0, &bytesReturned, NULL))
            {
                printErrorMessage(GetLastError());
                CloseHandle(hVolume);
                return 1;
            }

            // Try to get the information again
            if (!DeviceIoControl(hVolume, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &bytesReturned, NULL))
            {
                printErrorMessage(GetLastError());
                CloseHandle(hVolume);
                return 1;
            }
        }
        else
        {
            printErrorMessage(GetLastError());
            CloseHandle(hVolume);
            return 1;
        }
    }

    _tprintf(_T("USN Journal ID: %llu\n"), journalData.UsnJournalID);
    _tprintf(_T("First USN: %lld\n"), journalData.FirstUsn);
    _tprintf(_T("Next USN: %lld\n"), journalData.NextUsn);

    // Create a buffer to store the USN journal records
    PVOID buffer = new BYTE[journalData.AllocationDelta];

    // Enumerate the USN journal records
    READ_USN_JOURNAL_DATA readData = { 0 };
    readData.StartUsn = journalData.FirstUsn;
    readData.ReasonMask = USN_REASON_FILE_CREATE | USN_REASON_FILE_DELETE | USN_REASON_DATA_OVERWRITE;
    readData.ReturnOnlyOnClose = 0;
    readData.Timeout = 0;
    readData.BytesToWaitFor = 0;
    readData.UsnJournalID = journalData.UsnJournalID;

    if (!DeviceIoControl(hVolume, FSCTL_ENUM_USN_DATA, &readData, sizeof(readData), buffer, journalData.AllocationDelta, &bytesReturned, NULL))
    {
        printErrorMessage(GetLastError());
        CloseHandle(hVolume);
        return 1;
    }

    PUSN_RECORD usnRecord = (PUSN_RECORD)((PBYTE)buffer + sizeof(USN));
    while ((PBYTE)usnRecord < (PBYTE)buffer + bytesReturned) {
        _tprintf(_T("File name: %s\n"), usnRecord->FileName);
        _tprintf(_T("Reason: "));
        switch (usnRecord->Reason) {
        case USN_REASON_FILE_CREATE:
            _tprintf(_T("File Create\n"));
            break;
        case USN_REASON_FILE_DELETE:
            _tprintf(_T("File Delete\n"));
            break;
        case USN_REASON_DATA_OVERWRITE:
            _tprintf(_T("File Overwrite\n"));
            break;
        default:
            _tprintf(_T("Other\n"));
        }
        usnRecord = (PUSN_RECORD)((PBYTE)usnRecord + usnRecord->RecordLength);
    }

    // Clean up
    delete[] buffer;
    CloseHandle(hVolume);
    return 0;
}
