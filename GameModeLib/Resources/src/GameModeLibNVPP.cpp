#include <Windows.h>
#include <PowrProf.h>
#include "nvapi.h"
#include "NvApiDriverSettings.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <iostream>

#pragma comment(lib, "PowrProf.lib")
#pragma comment (lib, "nvapi.lib")

//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using namespace std;

bool IsACPwr();

//Start Generic Functions
string ToString(bool b)
{
	return b ? "True" : "False";
}

wstring ToWString(bool b)
{
	return b ? L"True" : L"False";
}


wstring ToHex(DWORD v)
{
	std::wstringstream ss;
	ss << L"0x" << std::uppercase << std::setfill(L'0') << std::setw(8) << std::hex << v;
	return ss.str();
}

std::wstring tolower(std::wstring s)
{
	for (auto& c : s)
		c = tolower(c);
	return s;
}

std::wstring ctow(const char* src)
{
	return std::wstring(src, src + strlen(src));
}

static bool CanCrash = true;
void NVAPIError(NvAPI_Status status, std::wstring id, bool crash)
{
	NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	std::wcerr << L"NVAPI Error: " << szDesc << id << std::endl;
	if (crash && CanCrash)
		exit(-1);
}

void NVAPIError(NvAPI_Status status, std::wstring id)
{
	NVAPIError(status, id, false);
}


void PrintGUID(GUID* guid, bool nline)
{
	printf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	if (nline)
		cout << endl;
}

void PrintGUID(GUID* guid)
{
	PrintGUID(guid, true);
}

bool IsEqual(GUID* guid, GUID* guid2)
{
	return guid->Data1 == guid2->Data1 &&
		guid->Data2 == guid2->Data2 &&
		guid->Data3 == guid2->Data3 &&
		guid->Data4[0] == guid2->Data4[0] &&
		guid->Data4[1] == guid2->Data4[1] &&
		guid->Data4[2] == guid2->Data4[2] &&
		guid->Data4[3] == guid2->Data4[3] &&
		guid->Data4[4] == guid2->Data4[4] &&
		guid->Data4[5] == guid2->Data4[5] &&
		guid->Data4[6] == guid2->Data4[6] &&
		guid->Data4[7] == guid2->Data4[7];
}

//Power Plan Start
bool PowerPlanExists(GUID id)
{
	GUID schemeGuid;
	DWORD bufferSize = sizeof(schemeGuid);
	DWORD index = 0;
	while (PowerEnumerate(NULL, NULL, NULL, ACCESS_SCHEME, index, (UCHAR*)&schemeGuid, &bufferSize) == ERROR_SUCCESS)
	{
		if (IsEqualGUID(id, schemeGuid))
		{
			return true;
		}
		index++;
		bufferSize = sizeof(schemeGuid);
	}
	return false;
}
//End Generic Functions

class EnumPwR {
public:
	wstring Name;
	DWORD PwRValue;
	DWORD* NVAPIValue;

	// Constructor
	EnumPwR(const wstring name, DWORD pwr, DWORD nv[]) : Name(name), PwRValue(pwr)
	{
		NVAPIValue = nv;
	}
};

static bool IsWinModern = false;//Is Windows Windows 10 or Higher
static bool IsWin8 = false;
static bool IsWin8Dot1 = false;
static bool IsWin7 = false;
static bool IsVista = false;
static bool IsXP = false;
void GenIsWins()
{
	NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
	OSVERSIONINFOEXW osInfo;
	*(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");
	if (NULL != RtlGetVersion)
	{
		osInfo.dwOSVersionInfoSize = sizeof(osInfo);
		RtlGetVersion(&osInfo);
	}
	if (osInfo.dwMajorVersion >= 10)
	{
		IsWinModern = true;
	}
	else if (osInfo.dwMajorVersion == 6)
	{
		if (osInfo.dwMinorVersion == 1)
			IsWin7 = true;
		else if (osInfo.dwMinorVersion == 0)
			IsVista = true;
		else if (osInfo.dwMinorVersion == 2)
			IsWin8 = true;
		else if (osInfo.dwMinorVersion >= 3)
			IsWin8Dot1 = true;
	}
	else if (osInfo.dwMajorVersion <= 5)
	{
		IsXP = true;
	}
}

GUID ULTIMATE = { 0xE9A42B02, 0xD5DF, 0x448D, {0xAA, 0x00, 0x03, 0xF1,0x47, 0x49, 0xEB, 0x61 } };
GUID SUB_GRAPHICS = { 0x4f971e89, 0xeebd, 0x4455,{ 0xa8, 0xde, 0x9e, 0x59, 0x04, 0x0b, 0xf5, 0x63 } };
GUID SETTING_GRAPHICS = { 0x5fb4938d, 0x1ee8, 0x4b0f,{ 0x9a, 0x3c, 0x50, 0x3b, 0x37, 0x9f, 0x4c, 0x89 } };
GUID SETTING_GRAPHICS_PWR = { 0x5fb4938d, 0x1ee8, 0x4b0f,{ 0x9a, 0x3c, 0x50, 0x3b, 0x68, 0x9f, 0x4c, 0x69 } };

EnumPwR NONE(L"", 0, NULL);
EnumPwR PERFORMANCE_OFF(L"Off", 0, new DWORD[1]{ 4294967295 });
EnumPwR PERFORMANCE_SAVING(L"Power Savings (Integrated)", 1, new DWORD[3]{ SHIM_MCCOMPAT_INTEGRATED, SHIM_RENDERING_MODE_INTEGRATED, SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_IGPU_TRANSCODING });
EnumPwR PERFORMANCE_AUTO(L"Auto", 2, new DWORD[3]{ SHIM_MCCOMPAT_AUTO_SELECT, SHIM_RENDERING_MODE_AUTO_SELECT, SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE });
EnumPwR PERFORMANCE_HIGH(L"High Performance (NVIDIA)", 3, new DWORD[3]{ SHIM_MCCOMPAT_ENABLE, SHIM_RENDERING_MODE_ENABLE, SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE });
EnumPwR ENUMGPUS[] = { PERFORMANCE_OFF, PERFORMANCE_SAVING, PERFORMANCE_AUTO, PERFORMANCE_HIGH };

EnumPwR PWR_OFF(L"Off", 0, new DWORD[1]{ 4294967295 });
EnumPwR PWR_SAVING(L"Power Savings (Adaptive)", 1, new DWORD[1]{ PREFERRED_PSTATE_ADAPTIVE }); //Could Have Also Used PREFERRED_PSTATE_PREFER_MIN But TBH If your on High Performance Adaptive should be the lowest setting
EnumPwR PWR_AUTO(L"Driver Controlled", 2, new DWORD[1]{ PREFERRED_PSTATE_DRIVER_CONTROLLED });
EnumPwR PWR_OPTIMAL(L"Optimal Power", 3, new DWORD[1]{ PREFERRED_PSTATE_OPTIMAL_POWER });
EnumPwR PWR_HIGH(L"High Peformance", 4, new DWORD[1]{ PREFERRED_PSTATE_PREFER_MAX });
EnumPwR PWR_HIGH_CONSISTENT(L"High Performance Consistent", 5, new DWORD[1]{ PREFERRED_PSTATE_PREFER_CONSISTENT_PERFORMANCE });

EnumPwR ENUMPWRS[] = { PWR_OFF, PWR_SAVING, PWR_AUTO, PWR_OPTIMAL, PWR_HIGH, PWR_HIGH_CONSISTENT };

//start program arguments
static bool HasPwr = true;
static bool ForceInstall = false;
static bool IsAC = IsACPwr();

//Probably Incorrect for 256 characters probably like 256 / 4? I don't know enough C++
void PwrCreateSettingValue(GUID* sub, GUID* setting, int index, wstring name, wstring description)
{
	int i = index;//Used for Buffers
	PowerCreatePossibleSetting(NULL, sub, setting, index);
	PowerWritePossibleValue(NULL, sub, setting, REG_DWORD, index, reinterpret_cast<UCHAR*>(&i), sizeof(DWORD));

	name = name + L"\0";//Add Null Terminator
	UCHAR buffer[256];
	memcpy(buffer, name.c_str(), 256);
	PowerWritePossibleFriendlyName(NULL, sub, setting, index, buffer, 256);

	description = description + L"\0";//Add Null Terminator
	UCHAR b[256];
	memcpy(b, description.c_str(), 256);
	PowerWritePossibleDescription(NULL, sub, setting, index, b, 256);
}

void PwrNameSub(GUID* sub, wstring name, wstring description)
{
	name = name + L"\0";//Add Null Terminator
	UCHAR bname[256];
	memcpy(bname, name.c_str(), 256);
	PowerWriteFriendlyName(NULL, NULL, sub, NULL, bname, 256);

	description = description + L"\0";//Add Null Terminator
	UCHAR bdesc[256];
	memcpy(bdesc, description.c_str(), 256);
	PowerWriteDescription(NULL, NULL, sub, NULL, bdesc, 256);
}

void PwrNameSetting(GUID* sub, GUID* setting, wstring name, wstring description)
{
	name = name + L"\0";//Add Null Terminator
	UCHAR bname[256];
	memcpy(bname, name.c_str(), 256);
	PowerWriteFriendlyName(NULL, NULL, sub, setting, bname, 256);

	description = description + L"\0";//Add Null Terminator
	UCHAR bdesc[256];
	memcpy(bdesc, description.c_str(), 256);
	PowerWriteDescription(NULL, NULL, sub, setting, bdesc, 256);
}

void CreateSettings(GUID* Current)
{
	wcout << L"Creating Game Mode Lib Graphics Settings" << endl;
	//Create the Sub and the first Setting
	PowerCreateSetting(NULL, &SUB_GRAPHICS, &SETTING_GRAPHICS); //Creates the Setting
	PwrNameSub(&SUB_GRAPHICS, L"GameModeLib Graphics", L"Adds NVIDIA Preffered Graphics Processor & Power Level to the Power Plan");
	PowerWriteSettingAttributes(&SUB_GRAPHICS, &SETTING_GRAPHICS, 0); //Sets the Attribute to SHOW
	PwrNameSetting(&SUB_GRAPHICS, &SETTING_GRAPHICS, L"Preffered Graphics Processor", L"Switch Between Discrete (Dedicated) And Integrated Graphics");
	//Create Power Setting Values
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS, 0, L"Off", L"Uses NVIDIA Control Pannel Instead");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS, 1, L"Power Savings (Integrated)", L"Integrated Graphics");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS, 2, L"Auto", L"NVIDIA's Auto");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS, 3, L"High Performance (NVIDIA)", L"Dedicated NVIDIA Graphics");
	//Set Default Values for Default Power Plans
	PowerWriteDCDefaultIndex(NULL, &ULTIMATE, &SUB_GRAPHICS, &SETTING_GRAPHICS, 3);//Ultimate Performance Scheme
	PowerWriteACDefaultIndex(NULL, &ULTIMATE, &SUB_GRAPHICS, &SETTING_GRAPHICS, 3);//Ultimate Performance Scheme
	PowerWriteDCDefaultIndex(NULL, &GUID_MIN_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS, 3);//High Performance Scheme
	PowerWriteACDefaultIndex(NULL, &GUID_MIN_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS, 3);//High Performance Scheme
	PowerWriteDCDefaultIndex(NULL, &GUID_TYPICAL_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS, 2);//Balenced Scheme
	PowerWriteACDefaultIndex(NULL, &GUID_TYPICAL_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS, 2);//Balenced Scheme
	PowerWriteDCDefaultIndex(NULL, &GUID_MAX_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS, 1);//Power Saver Scheme
	PowerWriteACDefaultIndex(NULL, &GUID_MAX_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS, 1);//Power Saver Scheme

	if (!HasPwr)
	{
		PowerRemovePowerSetting(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR);
		return;
	}
	//Create GPU Power Level Settings
	PowerCreateSetting(NULL, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR); //Creates the Setting
	PowerWriteSettingAttributes(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 0); //Sets the Attribute to SHOW
	PwrNameSetting(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, L"GPU Power Level (Discrete)", L"NVIDIA Graphics Power Level");
	//Create Power Setting Values
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 0, L"Off", L"Uses NVIDIA Control Pannel Instead");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 1, L"Power Savings (Adaptive)", L"Adaptive GPU Power");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 2, L"Driver Controlled", L"Lowers Resoultion to Save Power when FPS is Low");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 3, L"Optimal Power", L"Good Peformance");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 4, L"High Peformance", L"Max Performance");
	PwrCreateSettingValue(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 5, L"High Performance Consistent", L"Max Performance Consistently. May Overheat Use with caution");
	//Set Default Values for Default Power Plans
	PowerWriteDCDefaultIndex(NULL, &ULTIMATE, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 4);//Ultimate Performance Scheme
	PowerWriteACDefaultIndex(NULL, &ULTIMATE, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 4);//Ultimate Performance Scheme
	PowerWriteDCDefaultIndex(NULL, &GUID_MIN_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 3);//High Performance Scheme
	PowerWriteACDefaultIndex(NULL, &GUID_MIN_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 3);//High Performance Scheme
	PowerWriteDCDefaultIndex(NULL, &GUID_TYPICAL_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 3);//Balenced Scheme
	PowerWriteACDefaultIndex(NULL, &GUID_TYPICAL_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 3);//Balenced Scheme
	PowerWriteDCDefaultIndex(NULL, &GUID_MAX_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 1);//Power Saver Scheme
	PowerWriteACDefaultIndex(NULL, &GUID_MAX_POWER_SAVINGS, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, 1);//Power Saver Scheme
	PowerSetActiveScheme(NULL, Current);//Sync Changes Instantly
}

bool HasACValue(GUID* ACTIVE, GUID* SUB, GUID* SETTING)
{
	UINT32 val = 404;
	ULONG t = REG_DWORD;
	DWORD size = 4;
	int ERR = PowerReadACValue(NULL, ACTIVE, SUB, SETTING, &t, (UCHAR *)&val, &size);
	return ERR == ERROR_SUCCESS && val != 404;
}

bool HasPwrValue(GUID* pp, GUID* sub, GUID* setting, bool ac)
{
	UINT32 val = 404;
	ULONG t = REG_DWORD;
	DWORD size = 4;
	int ERR = ac ? PowerReadACValue(NULL, pp, sub, setting, &t, (UCHAR *)&val, &size) : PowerReadDCValue(NULL, pp, sub, setting, &t, (UCHAR *)&val, &size);
	return ERR == ERROR_SUCCESS && val != 404;
}

/**
* Is The Power Currently Plugged In False Means it's on Battery
*/
bool IsACPwr()
{
	SYSTEM_POWER_STATUS PwrStatus = { 0 };
	GetSystemPowerStatus(&PwrStatus);
	int lvl = PwrStatus.ACLineStatus;
	return lvl != 0;//If it's 256(Unkown) or 1 then it's AC otherwise it's DC
}

DWORD GetPwrValue(GUID* pp, GUID* sub, GUID* setting, bool ac)
{
	UINT32 val = 404;
	ULONG t = REG_DWORD;
	DWORD size = 4;
	int ERR = ac ? PowerReadACValue(NULL, pp, sub, setting, &t, (UCHAR *)&val, &size) : PowerReadDCValue(NULL, pp, sub, setting, &t, (UCHAR *)&val, &size);
	return ERR == ERROR_SUCCESS ? val : 404;
}

DWORD SetPwrValue(GUID* pp, GUID* sub, GUID* setting, bool ac, DWORD val)
{
	return ac ? PowerWriteACValueIndex(NULL, pp, sub, setting, val) : PowerWriteACValueIndex(NULL, pp, sub, setting, val);
}

void SyncToNVAPI(DWORD PrefGPU, DWORD GPUPwRLVL, bool HasPPChanged)
{
	EnumPwR ENUM_PWR = NONE;
	EnumPwR ENUM_GRAPHICS = NONE;
	bool HasFoundPWR = false;
	bool HasFoundG = false;
	if (HasPwr)
	{
		for (auto v : ENUMPWRS)
		{
			if (v.PwRValue == GPUPwRLVL)
			{
				ENUM_PWR = v;
				HasFoundPWR = true;
				break;
			}
		}
	}
	for (auto v : ENUMGPUS)
	{
		if (v.PwRValue == PrefGPU)
		{
			ENUM_GRAPHICS = v;
			HasFoundG = true;
			break;
		}
	}

	//Both Modules Are Disabled Return From Doing Any Operations
	if (ENUM_GRAPHICS.PwRValue == 0 && ENUM_PWR.PwRValue == 0)
		return;

	NvDRSSessionHandle hSession = 0;
	NvAPI_Status status = NvAPI_DRS_CreateSession(&hSession);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"SESSION", true);
	}
	status = NvAPI_DRS_LoadSettings(hSession);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"LOAD_SETTINGS", true);
	}

	NvDRSProfileHandle hProfile = 0;
	status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
	if (status != NVAPI_OK)
		NVAPIError(status, L"GET_BASE_PROFILE");

	//GPU Power Level SYNC
	if (ENUM_PWR.PwRValue != 0)
	{
		NVDRS_SETTING pwrsetting = { 0 };
		pwrsetting.version = NVDRS_SETTING_VER;
		pwrsetting.settingId = PREFERRED_PSTATE_ID;
		pwrsetting.settingType = NVDRS_DWORD_TYPE;
		pwrsetting.u32CurrentValue = ENUM_PWR.NVAPIValue[0];
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &pwrsetting);
		if (status != NVAPI_OK)
			NVAPIError(status, L"GPU_POWER_LVL");
	}

	//Set Preffered Graphics Processor From the Power Plan To NVIDIA Control Pannel
	if (ENUM_GRAPHICS.PwRValue != 0)
	{
		NVDRS_SETTING gsetting1 = { 0 };
		gsetting1.version = NVDRS_SETTING_VER;
		gsetting1.settingId = SHIM_MCCOMPAT_ID;
		gsetting1.settingType = NVDRS_DWORD_TYPE;
		gsetting1.u32CurrentValue = ENUM_GRAPHICS.NVAPIValue[0];
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &gsetting1);
		if (status != NVAPI_OK)
			NVAPIError(status, L"PREF_GPU_1");

		NVDRS_SETTING gsetting2 = { 0 };
		gsetting2.version = NVDRS_SETTING_VER;
		gsetting2.settingId = SHIM_RENDERING_MODE_ID;
		gsetting2.settingType = NVDRS_DWORD_TYPE;
		gsetting2.u32CurrentValue = ENUM_GRAPHICS.NVAPIValue[1];
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &gsetting2);
		if (status != NVAPI_OK)
			NVAPIError(status, L"PREF_GPU_2");

		//Only Change Optimus Rendering Flags When the Power Plan Changes From one to Another or Before the Loop on Install
		if (HasPPChanged)
		{
			NVDRS_SETTING gsetting3 = { 0 };
			gsetting3.version = NVDRS_SETTING_VER;
			gsetting3.settingId = SHIM_RENDERING_OPTIONS_ID;
			gsetting3.settingType = NVDRS_DWORD_TYPE;
			gsetting3.u32CurrentValue = ENUM_GRAPHICS.NVAPIValue[2];
			status = NvAPI_DRS_SetSetting(hSession, hProfile, &gsetting3);
			if (status != NVAPI_OK)
				NVAPIError(status, L"PREF_GPU_3");
		}
	}

	// Save Changes
	status = NvAPI_DRS_SaveSettings(hSession);
	if (status != NVAPI_OK)
		NVAPIError(status, L"SAVING");
	// Cleanup
	NvAPI_DRS_DestroySession(hSession);
	hSession = 0;
}

void SyncFromNVAPI(GUID* CurrentPP, DWORD prefgpu, DWORD pwr)
{
	//Cannot Sync if Either Both PrefGPU and PWR are Both Off or in an Errored State
	if (prefgpu == 0 && pwr == 0)
		return;
	NvDRSSessionHandle hSession = 0;
	NvAPI_Status status = NvAPI_DRS_CreateSession(&hSession);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"SESSION", true);
	}
	status = NvAPI_DRS_LoadSettings(hSession);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"LOAD_SETTINGS", true);
	}

	NvDRSProfileHandle hProfile = 0;
	status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"GET_BASE_PROFILE", true);
	}
	bool dirty = false;

	//Sync Power Level
	if (HasPwr && pwr != 0)
	{
		NVDRS_SETTING USetting = { 0 };
		USetting.version = NVDRS_SETTING_VER;
		USetting.settingType = NVDRS_DWORD_TYPE;
		status = NvAPI_DRS_GetSetting(hSession, hProfile, PREFERRED_PSTATE_ID, &USetting);
		DWORD VAL_PWR = status == NVAPI_OK ? USetting.u32CurrentValue : PREFERRED_PSTATE_DEFAULT;
		//Search for the Power Index that Matches the NVIDIA Power State
		for (auto p : ENUMPWRS)
		{
			if (VAL_PWR == p.NVAPIValue[0] && p.PwRValue != pwr)
			{
				dirty = true;
				wcout << L"Syncing Changes:" << p.Name << L" " << p.PwRValue << L" Old Value:" << pwr << endl;
				SetPwrValue(CurrentPP, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, IsAC, p.PwRValue);
			}
		}
	}

	if (prefgpu != 0)
	{
		NVDRS_SETTING PrefGPUSetting1 = { 0 };
		PrefGPUSetting1.version = NVDRS_SETTING_VER;
		PrefGPUSetting1.settingType = NVDRS_DWORD_TYPE;
		status = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_MCCOMPAT_ID, &PrefGPUSetting1);
		DWORD VAL_PWR = status == NVAPI_OK ? PrefGPUSetting1.u32CurrentValue : SHIM_MCCOMPAT_AUTO_SELECT;

		NVDRS_SETTING PrefGPUSetting2 = { 0 };
		PrefGPUSetting2.version = NVDRS_SETTING_VER;
		PrefGPUSetting2.settingType = NVDRS_DWORD_TYPE;
		status = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_RENDERING_MODE_ID, &PrefGPUSetting2);
		DWORD VAL_PWR2 = status == NVAPI_OK ? PrefGPUSetting2.u32CurrentValue : SHIM_RENDERING_MODE_AUTO_SELECT;

		//Search for the Power Index that Matches the NVIDIA Power State
		for (auto p : ENUMGPUS)
		{
			if ((VAL_PWR == p.NVAPIValue[0] || VAL_PWR2 == p.NVAPIValue[1]) && p.PwRValue != prefgpu)
			{
				dirty = true;
				wcout << L"Syncing Changes:" << p.Name << L" " << p.PwRValue << L" Old Value:" << pwr << endl;
				SetPwrValue(CurrentPP, &SUB_GRAPHICS, &SETTING_GRAPHICS, IsAC, p.PwRValue);
			}
		}
	}

	// Cleanup
	NvAPI_DRS_DestroySession(hSession);
	hSession = 0;

	if (dirty)
		PowerSetActiveScheme(NULL, CurrentPP);//Sync Changes Instantly
}

void Uninstall()
{
	//Remove the Power Plan Settings
	PowerRemovePowerSetting(&SUB_GRAPHICS, &SETTING_GRAPHICS);
	PowerRemovePowerSetting(&SUB_GRAPHICS, &SETTING_GRAPHICS_PWR);
	PowerRemovePowerSetting(&SUB_GRAPHICS, NULL);
}

DWORD GetPrefGPU(GUID* pp)
{
	return GetPwrValue(pp, &SUB_GRAPHICS, &SETTING_GRAPHICS, IsAC);
}

DWORD GetPwrGPU(GUID* pp)
{
	return HasPwr ? GetPwrValue(pp, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, IsAC) : 0;
}

static bool CloseAfter = false;
int main(int argc, char **argv)
{
	GenIsWins();
	if (IsWin7)
	{
		PERFORMANCE_SAVING.NVAPIValue[2] = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_HANDLE_WIN7_ASYNC_RUNTIME_BUG;
		PERFORMANCE_AUTO.NVAPIValue[2] = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_HANDLE_WIN7_ASYNC_RUNTIME_BUG;
		PERFORMANCE_HIGH.NVAPIValue[2] = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_HANDLE_WIN7_ASYNC_RUNTIME_BUG;
	}
	//Handle Commands "/help" and "/uninstall"
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			wstring arg = tolower(ctow(argv[i]));
			if (arg == L"/?" || arg == L"/help")
			{
				wcout << endl << L"Description:" << endl;
				wcout << L"GameModeLibNVPP Adds NVIDIA Preffered Graphics Processor and GPU Power Levels to the \r\nWindows Power Plan and Syncs Changes" << endl;
				wcout << endl;
				wcout << L"GameModeLibNVPP.exe" << endl;
				wcout << L"GameModeLibNVPP.exe /Install" << endl;
				wcout << L"GameModeLibNVPP.exe /Uninstall" << endl;
				wcout << endl;
				wcout << L"/Install  Forces Installation" << endl;
				wcout << L"/NoPwr  Installs Without GPU Power Level Option" << endl;
				//Check if NVIDIA is Installed and if Not Return 404
				NvAPI_Status status = NvAPI_Initialize();
				exit((status != NVAPI_OK ? 404 : 0));
			}
			else if (arg == L"")
			{
				//User Command line Parse error ignore
			}
			else if (arg == L"uninstall" || arg == L"/uninstall")
			{
				Uninstall();
				exit(0);
			}
			else if (arg == L"install" || arg == L"/install")
			{
				ForceInstall = true;
			}
			else if (arg == L"/nopwr" || arg == L"/nopower")
			{
				HasPwr = false;
			}
			else if (arg == L"/h" || arg == L"/hide")
			{
				HWND hWnd = GetConsoleWindow();
				ShowWindow(hWnd, SW_HIDE);
			}
			else if (arg == L"/f")
			{
				FreeConsole();
			}
			else if (arg == L"/c")
			{
				CloseAfter = true;
			}
			else if (arg == L"/nocrash")
			{
				CanCrash = false;
			}
			else
			{
				wcerr << L"Unregonized Option:\"" << arg << L"\" Use /? or /help to See a List of Options" << endl;
				exit(-1);
			}
		}
	}

	NvAPI_Status status = NvAPI_Initialize();
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"INIT", true);
	}

	//Enforce Singleton
	CreateMutexA(0, FALSE, "Local\\GameModeLibNVPP");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		wcerr << L"Program Is Already Running" << endl;
		exit(-1);
	}

	GUID* Org;
	PowerGetActiveScheme(NULL, &Org);
	DWORD OrgGPU = GetPrefGPU(Org);
	DWORD OrgPwR = GetPwrGPU(Org);

	//Install If Required
	if (ForceInstall || !HasACValue(Org, &SUB_GRAPHICS, &SETTING_GRAPHICS))
	{
		CreateSettings(Org);

		//Handle Errors with a timeout of 6.5s about
		if (OrgGPU == 404)
		{
			int tries = 0;
			while (OrgGPU == 404 && tries < 256)
			{
				Sleep(25);
				OrgGPU = GetPrefGPU(Org);
				tries++;
			}
			if (OrgGPU == 404)
			{
				wcerr << L"Critical Error Preffered Graphics Processor Not Found" << endl;
				exit(-1);
			}
			OrgPwR = GetPwrGPU(Org);
		}
	}

	//Close After Installing If Flaged to do so
	if (CloseAfter)
	{
		exit(0);
	}

	//Update Has Power If it doesn't exist it will be false
	HasPwr = HasPwr ? HasPwrValue(Org, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, IsAC) : false;

	//Start the Main Loop of the program
	GUID* Current = Org;
	DWORD CurrentGPU = OrgGPU;
	DWORD CurrentPwR = OrgPwR;
	SyncToNVAPI(OrgGPU, OrgPwR, ForceInstall);
	int tries = 0;
	while (true)
	{
		Sleep(2500);
		IsAC = IsACPwr();//Sync Power Changes
		PowerGetActiveScheme(NULL, &Current);
		bool HasPPChanged = false;
		if (!IsEqual(Org, Current))
		{
			//Force Update on Settings
			Org = Current;
			HasPPChanged = true;
		}

		//Update Values TODO put only when power shcemes or setting switch via notifications
		CurrentGPU = GetPrefGPU(Current);
		CurrentPwR = GetPwrGPU(Current);

		//Refresh Errored State Power Plan Settings
		while (CurrentGPU == 404 || CurrentPwR == 404)
		{
			if (tries > 254)
			{
				wcerr << L"Critical Error Has Occured Unable to Find Preffered Graphics Processor and or GPU Power Level Settings" << endl;
				exit(-1);
			}
			//Refresh Critical Settings
			IsAC = IsACPwr();
			PowerGetActiveScheme(NULL, &Current);
			CurrentGPU = GetPrefGPU(Current);
			CurrentPwR = GetPwrGPU(Current);
			tries++;
			Sleep(250);
			if (CurrentGPU != 404 && CurrentPwR != 404)
				tries = 0;
		}

		if (OrgGPU != CurrentGPU || OrgPwR != CurrentPwR)
		{
			OrgGPU = CurrentGPU;
			OrgPwR = CurrentPwR;
			SyncToNVAPI(CurrentGPU, CurrentPwR, HasPPChanged);
		}
	}
}