#pragma once

#include <Windows.h>
#include <wlanapi.h>
#include <WindowsX.h>
#include <vector>
#include <string>
#include <WbemCli.h> // WMI

#pragma comment(lib, "wlanapi.lib") // WLAN
#pragma comment(lib, "Crypt32.lib") // Crypt API
#pragma comment(lib, "wbemuuid.lib") // WMI

class CWirelessSettings
{
public:
	CWirelessSettings() :m_bConnected(false), m_ulStrength(0),m_strInterfaceGUID(L""),
						m_strAdapter(L""), m_strProfile(L""), m_strSSID(L""), m_strSecurity(L""), 
						m_strType(L"")
	{

	}
private:
	bool           m_bConnected;
	unsigned long  m_ulStrength;    
	std::wstring   m_strInterfaceGUID;
	std::wstring   m_strAdapter;
	std::wstring   m_strProfile;
	std::wstring   m_strSSID;
	std::wstring   m_strSecurity;
	std::wstring   m_strType;
	std::wstring   m_strPassword;
	std::vector<std::wstring> m_availableNW;


public:
	void Set_Connected(bool bCon) {m_bConnected = bCon;}
	bool Get_Connected() {return m_bConnected;}
	void Set_Strength (unsigned long nVal) { m_ulStrength = nVal;}
	unsigned long Get_Strength () { return m_ulStrength;}
	void Set_InterfaceGUID (std::wstring strGUID) { m_strInterfaceGUID = strGUID;}
	std::wstring Get_InterfaceGUID() { return m_strInterfaceGUID;}
	void Set_Adapter(std::wstring strAdapter) {m_strAdapter = strAdapter; }
	std::wstring Get_Adapter () { return m_strAdapter;}
	void Set_Profile (std::wstring strProfile) { m_strProfile = strProfile;}
	std::wstring Get_Profile () { return m_strProfile;}
	void Set_SSID (std::wstring ssid) { m_strSSID = ssid;}
	std::wstring Get_SSID () { return m_strSSID;}
	void Set_Security(std::wstring sec) { m_strSecurity = sec;}
	std::wstring Get_Security () { return m_strSecurity;}
	void Set_Type (std::wstring type) { m_strType = type;}
	std::wstring Get_Type () { return m_strType;}
	void Set_Password (std::wstring pwd) { m_strPassword = pwd;}
	std::wstring Get_Password () { return m_strPassword;}
	void Set_Available_NW (std::wstring strNW) { m_availableNW.push_back(strNW);}
	std::vector<std::wstring> Get_Available_NW() { return m_availableNW;}
	
};



class CWirelessHelper
{
public:
	CWirelessHelper(void);
	~CWirelessHelper(void);
	bool Get_Wireless_Info(CWirelessSettings& wifi);
private:
	std::wstring GenerateSSID (unsigned char* strSSID);
	void GetOSVersion (DWORD& dwMajor, DWORD& dwMinor);
	std::wstring ReadValuefromXML(const std::wstring &strXML, std::wstring strKey);
	DWORD  GetProcessID(std::wstring strName);
	bool SetPrivilege (std::wstring strPrivilege, HANDLE hToken, bool bEnable);
	bool  ImpersonateToLoginUser();
	std::wstring DecryptData(const std::wstring& strKey);
};

