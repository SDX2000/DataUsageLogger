#pragma once 

#include <Windows.h>
#include <string>
#include <vector>

struct AccessPointInfo
{
    std::wstring ProfileName;
    std::wstring SSID;
    ULONG SignalStrength;
    std::wstring Type;
};

struct WifiAdapterInfo
{
    GUID GUID;
    std::wstring Description;
    AccessPointInfo AccessPoint;
};

std::vector<WifiAdapterInfo> GetWifiAdapters();
void print(const WifiAdapterInfo& adapter);