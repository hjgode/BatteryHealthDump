// BatteryHealthDump.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "C:/Program Files (x86)/Intermec/Developer Library/Include/ITCSSApi.h"
#pragma comment (lib, "C:/Program Files (x86)/Intermec/Developer Library/Lib/WCE600/WM6/Armv4i/ITCSSApi.lib")

#include "C:/Program Files (x86)/Intermec/Developer Library/Include/smartsyserrors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TCHAR* regKey = L"Software\\Intermec\\BatteryHealth";

//XML to query
static const TCHAR s80211QueryXml[] =
_T("<Subsystem Name=\"Funk Security\">\r\n \
	<Group Name=\"802.11 Radio\">\r\n \
			<Field Name=\"ZeroConfig\"></Field>\r\n \
	</Group>\r\n \
</Subsystem>");

static const TCHAR prefix[]=L"<Subsystem Name=\"Device Monitor\">\
		<Group Name=\"ITCHealth\">\"";
TCHAR* suffix=
		L"</Group>\
		</Subsystem>";

TCHAR* base = L"<Field Name=\"System\\Power\\Battery\\";
TCHAR* closeField = L"</Field>";
TCHAR* closeValue = L"\">";
TCHAR* values[]={
	L"BackupBatteryFlag", 
	L"BackupBatteryFullLifeTime", 
	L"BackupBatteryLifePercent", 
	L"BackupBatteryLifeTime", 
	L"BackupBatteryVoltage", 
	L"BatteryAverageCurrent", 
	L"BatteryAverageInterval", 
	L"BatteryCPUUsage", 
	L"BatteryChargerState", 
	L"BatteryChemistry", 
	L"BatteryCurrent", 
	L"BatteryFlag", 
	L"BatteryHealth", 
	L"BatteryHighTemperatureReached", 
	L"BatteryIdleTimeout", 
	L"BatteryLastChanged", 
	L"BatteryLifePercent", 
	L"BatteryLowTemperatureReached", 
	L"BatteryPrevCPUUsage", 
	L"BatteryTemperature", 
	L"BatteryVoltage", 
	L"BatterymAHourConsumed", 
	L"ChargingTime", 
	L"ExtremeTemperatureTime", 
	L"HighTemperatureTime", 
	L"LastFullCharge", 
	L"LowTemperatureTime", 
	L"ManufacturedDate", 
	L"PartNumber", 
	L"SerialNumber", 
	L"UsageTime",
	NULL
};

static const TCHAR batteryHealthXML[] = (
L"<Subsystem Name=\"Device Monitor\">\
		<Group Name=\"ITCHealth\">\
			<Field Name=\"System\\Power\\ACLineStatus\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BackupBatteryFlag\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BackupBatteryFullLifeTime\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BackupBatteryLifePercent\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BackupBatteryLifeTime\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BackupBatteryVoltage\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryAverageCurrent\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryAverageInterval\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryCPUUsage\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryChargerState\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryChemistry\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryCurrent\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryFlag\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryHealth\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryHighTemperatureReached\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryIdleTimeout\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryLastChanged\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryLifePercent\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryLowTemperatureReached\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryPrevCPUUsage\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryTemperature\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatteryVoltage\"></Field>\
			<Field Name=\"System\\Power\\Battery\\BatterymAHourConsumed\"></Field>\
			<Field Name=\"System\\Power\\Battery\\ChargingTime\"></Field>\
			<Field Name=\"System\\Power\\Battery\\ExtremeTemperatureTime\"></Field>\
			<Field Name=\"System\\Power\\Battery\\HighTemperatureTime\"></Field>\
			<Field Name=\"System\\Power\\Battery\\LastFullCharge\"></Field>\
			<Field Name=\"System\\Power\\Battery\\LowTemperatureTime\"></Field>\
			<Field Name=\"System\\Power\\Battery\\ManufacturedDate\"></Field>\
			<Field Name=\"System\\Power\\Battery\\PartNumber\"></Field>\
			<Field Name=\"System\\Power\\Battery\\SerialNumber\"></Field>\
			<Field Name=\"System\\Power\\Battery\\UsageTime\"></Field>\
		</Group>\
	</Subsystem>");

TCHAR* getTimeStr(){
	static TCHAR wReturn[32]={0};
	SYSTEMTIME time;
	GetSystemTime(&time);
	wsprintf(wReturn, L"%04i%02i%02iT%02i:%02i:%02i", time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond);
	return wReturn;
}

//build full xml to query using our elements
TCHAR* getXMLString(){
	static TCHAR wReturn[8000];
	wsprintf(wReturn, L"");
	wcscat(wReturn, prefix);
	wcscat(wReturn, L"\0");//add terminating zero after wcscat
	int i=0;
	while(values[i]!=NULL){
		wcscat(wReturn, base);wcscat(wReturn, L"\0");
		wcscat(wReturn, values[i]);wcscat(wReturn, L"\0");
		wcscat(wReturn, closeValue);wcscat(wReturn, L"\0");
		wcscat(wReturn, closeField);wcscat(wReturn, L"\0");
		DEBUGMSG(1, (L"added: %s\n", values[i]));
		i++;
	}
	wcscat(wReturn, suffix);
	wcscat(wReturn, L"\0");
	return wReturn;
}

void updateBatteryHealth(){
	TCHAR* test=getXMLString();
	const TCHAR* wszXmlQuery = batteryHealthXML;//.GetBuffer();
	ULONG uTimeOut=2; //seconds
	size_t cData = 8000; 
	TCHAR *pRetData = (TCHAR *)malloc (cData * sizeof (TCHAR));

	//open registry key for storage
	HKEY hKey=NULL; DWORD dwDisposition;
	LONG lRes = RegCreateKeyEx(HKEY_LOCAL_MACHINE, regKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	if(lRes!=ERROR_SUCCESS)
		DEBUGMSG(1, (L"RegCreateKeyEx failed: %i\n", GetLastError()));
	if(hKey!=NULL){
		lRes = RegSetValueEx(hKey, L"LastXMLUpdate", 0, REG_SZ, (LPBYTE)getTimeStr(), wcslen(getTimeStr())*sizeof(TCHAR));
		if(lRes!=ERROR_SUCCESS)
			DEBUGMSG(1, (L"RegSetValue for %s failed: %i\n", L"LastXMLUpdate", GetLastError()));
	}

	//ITCSSAPI_RETURN_TYPE result = ITCSSGet(s80211QueryXml, pRetData, &cData, uTimeOut);
	//ITCSSAPI_RETURN_TYPE result = ITCSSGet(wszXmlQuery, pRetData, &cData, uTimeOut);
	ITCSSAPI_RETURN_TYPE result = ITCSSGet(test, pRetData, &cData, uTimeOut);
	if(result==E_SS_SUCCESS){
		if(hKey!=NULL){
			lRes = RegSetValueEx(hKey, L"LastXMLresult", 0, REG_SZ, (LPBYTE)L"SUCCESS", wcslen(L"SUCCESS")*sizeof(TCHAR));
			if(lRes!=ERROR_SUCCESS)
				DEBUGMSG(1, (L"RegSetValue for %s failed: %i\n", L"LastXMLresult", GetLastError()));
		}
		//Parse out SSID from returned XML
		//XML should be like this <Field Name=\"SSID\">xxx</Field>
		int i=0;
		while(values[i]!=NULL){
			TCHAR sValue[128] = {0};
			//base plus values[i]
			TCHAR pLookfor[255]={0};
			wsprintf(pLookfor, L"%s%s%s", base, values[i], closeValue);
			//find value field
			TCHAR *pData = _tcsstr (pRetData, pLookfor);
			if (pData != NULL)
			{
				pData = pData + wcslen(pLookfor);
				for (int ii=0; *pData != '<'; ii++, pData++)
				{
					sValue[ii] = *pData;
				}
				DEBUGMSG(1, (L"%s = %s\n", values[i], sValue));
			}else{ //value not found
				DEBUGMSG(1, (L"%s not found\n", values[i]));
			}
			//save to registry if opened
			if(hKey!=NULL){
				lRes = RegSetValueEx(hKey, values[i], 0, REG_SZ, (LPBYTE)sValue, wcslen(sValue)*sizeof(TCHAR));
				if(lRes!=ERROR_SUCCESS)
					DEBUGMSG(1, (L"RegSetValue for %s failed: %i\n", values[i], GetLastError()));
			}
			i++;
			//clear
			pData=NULL;
		}
	}else
	{
		DEBUGMSG(1, (L"ITCSSGet failed: 0x%08x (%i)\n", result, result));
		if(hKey!=NULL){
			lRes = RegSetValueEx(hKey, L"LastXMLresult", 0, REG_SZ, (LPBYTE)L"ERROR", wcslen(L"ERROR")*sizeof(TCHAR));
			if(lRes!=ERROR_SUCCESS)
				DEBUGMSG(1, (L"RegSetValue for %s failed: %i\n", L"LastXMLresult", GetLastError()));
		}
		//0xc16e0021	E_SSAPI_OPERATION_FAILED
	}
	if(hKey!=NULL)
		RegCloseKey(hKey);
	//free memory
	free (pRetData);
}

int _tmain(int argc, _TCHAR* argv[])
{
	updateBatteryHealth();
	return 0;
}

