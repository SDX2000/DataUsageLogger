#include "WifiAdapterInfo.h"

#include <tchar.h>
#include <wlanapi.h>
#include <locale>
#include <codecvt>
#include <iostream>


#pragma comment(lib, "wlanapi.lib") // WLAN

using namespace std;


wstring_convert<std::codecvt_utf8_utf16<wchar_t>> gConverter;


vector<WifiAdapterInfo> GetWifiAdapters()
{
    DWORD dwWLANVersion = 0;

    HANDLE hClient = NULL;
    DWORD  dwResult = 0;
    WLAN_INTERFACE_INFO_LIST * pWInfoList;
    bool bFound = false;
    vector<WifiAdapterInfo> adapters;

    dwResult = WlanOpenHandle(WLAN_API_MAKE_VERSION(2, 0), NULL, &dwWLANVersion, &hClient);

    if (dwResult != ERROR_SUCCESS)
    {
        return adapters;
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pWInfoList);

    if (dwResult != ERROR_SUCCESS)
    {
        return adapters;
    }



    for (int i = 0; i < (int)pWInfoList->dwNumberOfItems; i++)
    {
        WLAN_INTERFACE_INFO *pInfo = &pWInfoList->InterfaceInfo[i];
        if (pInfo->isState != wlan_interface_state_connected)
            continue;
        WifiAdapterInfo ai;

        ai.GUID = pInfo->InterfaceGuid;

        ai.Description = pInfo->strInterfaceDescription;

        WLAN_CONNECTION_ATTRIBUTES *pConnectionAttributes;
        DWORD dwSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);

        dwResult = WlanQueryInterface(hClient, &pInfo->InterfaceGuid,
            wlan_intf_opcode_current_connection,
            NULL, &dwSize, (PVOID*)&pConnectionAttributes, NULL);
        if (dwResult == ERROR_SUCCESS)
        {
            ai.AccessPoint.ProfileName = pConnectionAttributes->strProfileName;
            ai.AccessPoint.SSID = gConverter.from_bytes((CHAR*)pConnectionAttributes->wlanAssociationAttributes.dot11Ssid.ucSSID);
            ai.AccessPoint.SignalStrength = pConnectionAttributes->wlanAssociationAttributes.wlanSignalQuality;
            ai.AccessPoint.Type = pConnectionAttributes->wlanAssociationAttributes.dot11BssType == 2 ? L"Adhoc" : L"Infrastructure";
        }
        adapters.push_back(ai);
    }

    if (pWInfoList)
    {
        WlanFreeMemory(pWInfoList);
        pWInfoList = NULL;
    }

    if (hClient)
    {
        WlanCloseHandle(hClient, NULL);
    }
    return adapters;
}

void print(const WifiAdapterInfo& adapter)
{
    WCHAR  strGuidString[40] = { 0 };

    StringFromGUID2(adapter.GUID, (LPOLESTR)strGuidString, 39);
    wcout << L"GUID: " << strGuidString << endl;

    wcout << L"Adapter description: " << adapter.Description << endl;
    wcout << L"Profile: " << adapter.AccessPoint.ProfileName << endl;
    wcout << L"SSID: " << adapter.AccessPoint.SSID << endl;
    wcout << L"Strength: " << adapter.AccessPoint.SignalStrength << endl;
    wcout << L"Type: " << adapter.AccessPoint.Type << endl;
}