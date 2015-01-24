#include "stdafx.h"


CWirelessHelper::CWirelessHelper(void)
{
}


CWirelessHelper::~CWirelessHelper(void)
{
	 
}

std::wstring CWirelessHelper::ReadValuefromXML(const std::wstring &strXML, std::wstring strKey)
{
	std::wstring strVal;

	std::wstring start_tag, end_tag;
	start_tag = L"<" + strKey + L">";
	int start_tag_len = start_tag.length();

	end_tag   = L"</" + strKey + L">";
	int start_pos = strXML.find(start_tag, 0);
	int end_pos   = strXML.find(end_tag, start_pos);
	int end_tag_len = end_tag.length();

	if(start_pos != strXML.npos &&  end_pos != strXML.npos)
	{
		 strVal = strXML.substr(start_pos + start_tag_len, end_pos-(start_pos + start_tag_len));
	}
	return strVal;

}


std::wstring CWirelessHelper:: GenerateSSID (unsigned char* strSSID)
{
	char chSSID[MAX_PATH] = {0};
	sprintf_s (chSSID, "%s", strSSID);
	std::string ssid = chSSID;
	std::wstring wSSID(ssid.begin(), ssid.end());
	return wSSID;
}

void  CWirelessHelper :: GetOSVersion (DWORD& dwMajor, DWORD& dwMinor)
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	dwMajor = osvi.dwMajorVersion;
	dwMinor = osvi.dwMinorVersion;
}

bool CWirelessHelper:: Get_Wireless_Info(CWirelessSettings& wifi)
{
	DWORD dwWLANVersion = 0;

	HANDLE hClient = NULL;
	DWORD  dwResult = 0;
	WLAN_INTERFACE_INFO_LIST * pWInfoList;
	bool bFound =  false;
	WCHAR  strGuidString[40] = {0};
	DWORD dwOSMajorVersion = 0; DWORD dwOSMinorVersion = 0;
	GetOSVersion (dwOSMajorVersion, dwOSMinorVersion);
		
	dwResult = WlanOpenHandle (WLAN_API_MAKE_VERSION(2,0), NULL, &dwWLANVersion , &hClient);
	if(dwResult != ERROR_SUCCESS)
	{
		return false;
	}
	dwResult =  WlanEnumInterfaces (hClient, NULL, &pWInfoList);
	if (dwResult != ERROR_SUCCESS)
	{
		return false ;
	}
	for (int i = 0; i < (int)pWInfoList->dwNumberOfItems; i++)
	{
		WLAN_INTERFACE_INFO *pInfo = &pWInfoList->InterfaceInfo[i];
		if(pInfo->isState != wlan_interface_state_connected) continue;
		wifi.Set_Connected(true);
		StringFromGUID2 (pInfo->InterfaceGuid, (LPOLESTR) strGuidString, 39);
		wifi.Set_InterfaceGUID (strGuidString);
		wifi.Set_Adapter (pInfo->strInterfaceDescription);

		WLAN_CONNECTION_ATTRIBUTES *pConnectionAttributes;
		DWORD dwSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
		LPWSTR pProfileXML = NULL;

		dwResult = WlanQueryInterface (hClient, &pInfo->InterfaceGuid, 
			                          wlan_intf_opcode_current_connection, 
									  NULL, &dwSize, (PVOID*)&pConnectionAttributes, NULL);
		if (dwResult == ERROR_SUCCESS)
		{
			wifi.Set_Profile (pConnectionAttributes->strProfileName);
			wifi.Set_SSID (GenerateSSID(pConnectionAttributes->wlanAssociationAttributes.dot11Ssid.ucSSID));
			wifi.Set_Strength (pConnectionAttributes->wlanAssociationAttributes.wlanSignalQuality);
			wifi.Set_Type (pConnectionAttributes->wlanAssociationAttributes.dot11BssType ==
				            2 ? L"Adhoc" : L"Infrastructure");

			DWORD dwFlags = WLAN_PROFILE_GET_PLAINTEXT_KEY;
			DWORD dwGranted = 0;
		    
			if(dwOSMajorVersion > 5 && dwOSMinorVersion > 1) // Win7 and above we will get Password as plain text
			{
				dwResult = WlanGetProfile (hClient, &pInfo->InterfaceGuid,wifi.Get_Profile().c_str(), 
											  NULL, &pProfileXML, &dwFlags, &dwGranted);
				if(dwResult == ERROR_SUCCESS)
				{
					wifi.Set_Security(ReadValuefromXML(pProfileXML, L"authentication"));
					wifi.Set_Password(ReadValuefromXML(pProfileXML, L"keyMaterial"));
					OutputDebugString (pProfileXML);
				}
			}
			else // Vista, we will get cipher text, we need to convert to plain text
			{
				dwResult = WlanGetProfile (hClient, &pInfo->InterfaceGuid,wifi.Get_Profile().c_str(), 
					       NULL, &pProfileXML, 0, 0);
				if (dwResult == ERROR_SUCCESS)
				{
					wifi.Set_Security(ReadValuefromXML(pProfileXML, L"authentication"));
					std::wstring strKey  = ReadValuefromXML(pProfileXML, L"keyMaterial");
					std::wstring strPWD = DecryptData(strKey);
					wifi.Set_Password(strPWD);
				}
			}
			
		}

		WLAN_AVAILABLE_NETWORK_LIST *pWnwList;
		dwResult =  WlanGetAvailableNetworkList(hClient, &pInfo->InterfaceGuid, 0, NULL, &pWnwList);
		if(dwResult == ERROR_SUCCESS)
		{
			for (int i = 0; i < (int) pWnwList->dwNumberOfItems; i++)
			{
				WLAN_AVAILABLE_NETWORK  *pNetwok = &pWnwList->Network[i];
				wifi.Set_Available_NW(GenerateSSID(pNetwok->dot11Ssid.ucSSID));
			}
		}

	}
	
	if(pWInfoList)
	{
		WlanFreeMemory (pWInfoList);
		pWInfoList = NULL;
	}

	if(hClient)
	{
		WlanCloseHandle(hClient, NULL);
	}
	return true;
}

// This function uses WMI to enumerate all running Process and get ProcessID
DWORD CWirelessHelper :: GetProcessID(std::wstring strName)
{
    DWORD dwProcID = 0;
	bool bFound = false;
	HRESULT hr;

	hr = CoInitialize(NULL);
	hr =  CoInitializeSecurity( NULL,-1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
		                       RPC_C_IMP_LEVEL_IMPERSONATE, NULL,NULL, NULL);

	if (SUCCEEDED(hr))
	{
		ATL::CComPtr<IWbemLocator> pWbemLocator;
		hr = pWbemLocator.CoCreateInstance (CLSID_WbemLocator);

		IWbemServices *pServices;
		IEnumWbemClassObject * pEnumerator;

		hr = pWbemLocator->ConnectServer (ATL::CComBSTR(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0,0, &pServices);

		 hr = CoSetProxyBlanket(
		   pServices,                        // Indicates the proxy to set
		   RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		   RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		   NULL,                        // Server principal name 
		   RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		   RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		   NULL,                        // client identity
		   EOAC_NONE                    // proxy capabilities 
		);


		hr = pServices->ExecQuery (ATL::CComBSTR(L"WQL"), ATL::CComBSTR(L"SELECT * FROM Win32_Process"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

		while (pEnumerator)
		{
			IWbemClassObject *pclsObj;
			ULONG lReturn = 0;
			hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &lReturn);
			if ( hr != S_OK || lReturn == 0)
			{
				break;
			}

			ATL::CComVariant vPropName;
			ATL::CComVariant vPropID;

			pclsObj->Get(L"Name", 0, &vPropName, 0, 0);
			pclsObj->Get(L"ProcessId", 0, &vPropID, 0, 0);
		
			std::wstring strProcName = vPropName.bstrVal;

			if(strProcName.find(strName) != std::wstring::npos)
			{
				pclsObj->Get(L"ProcessId", 0, &vPropID, 0, 0);
				dwProcID = vPropID.lVal;
				bFound = true;
			}

		
			pclsObj->Release();
			VariantClear(&vPropName);
			VariantClear(&vPropID);
		
			if (bFound) break; 

		}

		if(pEnumerator)
		pEnumerator->Release();

		if(pServices)
			pServices->Release();

	}

	CoUninitialize();
	return dwProcID;
}

/*
 Windows Added Integrity Levels to Process and Secured Objects(Resources) to control access to resource and provide a second level security. (Vista onwards)
 Each process/Resource is assigned an integrity level (LOW, MEDIUM, HIGH, SYSTEM). When a process is accessing a resource its integrity level is compared with 
 the Resource's integrity level. It integrity level of process is less than resources , access is denied. By default a process launched by administrator has
 an integrity level of medium, if administrator runs the process in an elevated mode, the integrity level of that process becomes high. But there are certain
 system resources process cannot access even its running on high integrity level. (Eg : Crypto API's). For that we need to elevate the integrity level of 
 that particular thread process to system integrity level. So if SeDebugPrivilege is set on elevated process, the process/thread can access System integrity
 level resources. This functions shows how to set SeDebugPrivilege. Privilege is stored in Access Token. So we need to add new privileg to the Token as 
 shown below.
 */
bool CWirelessHelper::SetPrivilege (std::wstring strPrivilege, HANDLE hToken, bool bEnable)
{
	BOOL bReturn = false;
	LUID luid;
	bReturn =  LookupPrivilegeValue (NULL, strPrivilege.c_str(), &luid); // This is the value for SE_DUBUG_NAME
	if (bReturn == false) return false;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	if (AdjustTokenPrivileges (hToken, FALSE, &tp, 0, NULL, NULL) != ERROR_SUCCESS)
	{
		return false;
	}
	return true;
}

bool CWirelessHelper :: ImpersonateToLoginUser()
{
	 HANDLE hProcess;
	 HANDLE hToken;
	 BOOL bSuccess;

	DWORD dwProcID = GetProcessID (L"winlogon.exe");

	if (dwProcID < 1) return false;

	hProcess = OpenProcess (MAXIMUM_ALLOWED, FALSE, dwProcID);

	if (hProcess == INVALID_HANDLE_VALUE)
		return false;
			
	bSuccess = OpenProcessToken (hProcess, MAXIMUM_ALLOWED, &hToken);
	if (bSuccess)
	{

		SetPrivilege (SE_DEBUG_NAME, hToken, true); // 
 		bSuccess = ImpersonateLoggedOnUser (hToken);
	}
	
	if (hProcess)
	CloseHandle (hProcess);
	if (hToken)
	CloseHandle (hToken);
	
	return bSuccess;
}

std::wstring CWirelessHelper:: DecryptData(const std::wstring& strKey)
{
	std::wstring strPWD = L"";
	BYTE byteKey[1024] = {0};
	DWORD dwLength = 1024;
	DATA_BLOB DataOut, DataVerify;

	ImpersonateToLoginUser();

	BOOL bReturn = CryptStringToBinary (strKey.c_str(), strKey.length(), CRYPT_STRING_HEX, byteKey, &dwLength, 0, 0);
	if(bReturn)
	{
		DataOut.cbData = dwLength;
		DataOut.pbData = (BYTE*) byteKey;

	}
	if ( CryptUnprotectData (&DataOut, NULL, NULL, NULL, NULL, 0, &DataVerify))
	{
		CHAR str[MAX_PATH] = {0};
		sprintf_s(str, "%hs", DataVerify.pbData);
		std::string strData(str);
		std::wstring strPassword(strData.begin(), strData.end());
		strPWD = strPassword;
	}

	RevertToSelf();

	return strPWD;
}		
