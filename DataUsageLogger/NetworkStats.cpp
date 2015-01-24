#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <objbase.h>
#include <wtypes.h>
#include <stdio.h>
#include <stdlib.h>

// Need to link with Iphlpapi.lib
#pragma comment(lib, "iphlpapi.lib")

// Need to link with Ole32.lib to print GUID
#pragma comment(lib, "ole32.lib")


bool GetAdapterIndex(const GUID &guid, ULONG *adapterIndex)
{
    if (adapterIndex == NULL)
    {
        return false;
    }

    ULONG retVal = NO_ERROR;

    NET_LUID luid = { 0 };

    retVal = ConvertInterfaceGuidToLuid(&guid, &luid);

    if (retVal != NO_ERROR) {
        wprintf(L"ConvertInterfaceAliasToLuid returned error: %lu\n", retVal);
        return false;
    }

    retVal = ConvertInterfaceLuidToIndex(&luid, adapterIndex);

    if (retVal != NO_ERROR) {
        wprintf(L"ConvertInterfaceLuidToIndex returned error: %lu\n", retVal);
        return false;
    }

    return true;
}

bool GetDataUsage(const GUID &guid, UINT &megabytesTx, UINT &megabytesRx)
{
    ULONG retVal = 0;
    ULONG ifIndex;

    MIB_IF_ROW2 ifRow;

    // Make sure the ifRow is zeroed out
    SecureZeroMemory((PVOID)&ifRow, sizeof(MIB_IF_ROW2));
    GetAdapterIndex(guid, &ifIndex);

    ifRow.InterfaceIndex = ifIndex;

    retVal = GetIfEntry2(&ifRow);

    if (retVal != NO_ERROR) {
        wprintf(L"GetIfEntry returned error: %lu\n", retVal);
        return false;
    }

    megabytesTx = (UINT)(ifRow.OutOctets / 1024 / 1024);
    megabytesRx = (UINT)(ifRow.InOctets / 1024 / 1024);
    return true;
}