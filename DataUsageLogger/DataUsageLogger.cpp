// DataUsageLogger.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <shlwapi.h>
#include <iostream>
#include <sstream>
#include <tchar.h>


#include "WifiAdapterInfo.h"
#include "shlobj.h"


#pragma comment(lib,"shlwapi.lib")

using namespace std;

bool GetDataUsage(const GUID &guid, UINT &megabytesTx, UINT &megabytesRx);



// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
void CurrentDateTime(wstring &dateStr, wstring &timeStr) {
    time_t     now = time(0);
    struct tm  tstruct;
    WCHAR       buf[256];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    wcsftime(buf, _countof(buf), L"%d-%m-%Y", &tstruct);
    dateStr = buf;
    wcsftime(buf, _countof(buf), L"%X", &tstruct);
    timeStr = buf;
}

BOOL DirectoryExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL FileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool LogDataUsageForAdapter(WifiAdapterInfo ai)
{
    TCHAR szAppDataPath[MAX_PATH];
    // Get path for each computer, non-user specific and non-roaming data.
    if (FAILED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szAppDataPath)))
    {
        return false;
    }

    TCHAR szPath[MAX_PATH];

    swprintf(szPath, _countof(szPath), L"%s\\SDXTECH", szAppDataPath);

    if (!DirectoryExists(szPath)){
        CreateDirectory(szPath, NULL);
    }

    swprintf(szPath, _countof(szPath), L"%s\\DataUsageLogger", szPath);

    if (!DirectoryExists(szPath)){
        CreateDirectory(szPath, NULL);
    }

    swprintf(szPath, _countof(szPath), L"%s\\%s.csv", szPath, ai.AccessPoint.SSID.c_str());

    bool writeHeader = false;
    if (!FileExists(szPath)) {
        writeHeader = true;
    }

    FILE * fp = _wfopen(szPath, L"a");
    if (!fp){
        wcerr << L"ERROR: Could not open outputfile:" << szPath << endl;
        return false;
    }

    if (writeHeader) {
        fwprintf(fp, L"DATE,TIME,MB UPLOADED,MB DOWNLOADED\n");
    }

    UINT megabytesTx, megabytesRx;

    if (GetDataUsage(ai.GUID, megabytesTx, megabytesRx)){
        wstring date, time;
        CurrentDateTime(date, time);
        fwprintf(fp, L"%s,%s,%u,%u\n", date.c_str(), time.c_str(), megabytesTx, megabytesRx);
    }

    fflush(fp);
    fclose(fp);

    return true;
}



void LogAllAdapters()
{
    vector<WifiAdapterInfo> adapters = GetWifiAdapters();
    for each (WifiAdapterInfo adapter in adapters)
    {
        LogDataUsageForAdapter(adapter);
    }
}