#include <Windows.h>
#include <PowrProf.h>
#include "nvapi.h"
#include "NvApiDriverSettings.h"
#include <string>
#include <iostream>

#pragma comment(lib, "PowrProf.lib")
#pragma comment (lib, "nvapi.lib")

using namespace std;

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

GUID ULTIMATE = { 0xE9A42B02, 0xD5DF, 0x448D, {0xAA, 0x00, 0x03, 0xF1,0x47, 0x49, 0xEB, 0x61 } };
GUID SUB_GRAPHICS = { 0x4f971e89, 0xeebd, 0x4455,{ 0xa8, 0xde, 0x9e, 0x59, 0x04, 0x0b, 0xf5, 0x63 } };
GUID SETTING_GRAPHICS = { 0x5fb4938d, 0x1ee8, 0x4b0f,{ 0x9a, 0x3c, 0x50, 0x3b, 0x37, 0x9f, 0x4c, 0x89 } };
GUID SETTING_GRAPHICS_PWR = { 0x5fb4938d, 0x1ee8, 0x4b0f,{ 0x9a, 0x3c, 0x50, 0x3b, 0x68, 0x9f, 0x4c, 0x69 } };

const EnumPwR NONE(L"", 0, NULL);
const EnumPwR* NONE_REF = &NONE;
const EnumPwR PERFORMANCE_OFF(L"Off", 0, NULL);
const EnumPwR PERFORMANCE_SAVING(L"Power Savings (Integrated)", 1, new DWORD[3] { SHIM_MCCOMPAT_INTEGRATED, SHIM_RENDERING_MODE_INTEGRATED, SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE });
const EnumPwR PERFORMANCE_AUTO(L"Auto", 2, new DWORD[3]{ SHIM_MCCOMPAT_AUTO_SELECT, SHIM_RENDERING_MODE_AUTO_SELECT, SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE });
const EnumPwR PERFORMANCE_HIGH(L"High Performance (NVIDIA)", 3, new DWORD[3]{ SHIM_MCCOMPAT_ENABLE, SHIM_RENDERING_MODE_ENABLE, SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE });
EnumPwR ENUMGPUS[] = { PERFORMANCE_OFF, PERFORMANCE_SAVING, PERFORMANCE_AUTO, PERFORMANCE_HIGH };

const EnumPwR PWR_OFF(L"Off", 0, NULL);
const EnumPwR PWR_SAVING(L"Power Savings (Adaptive)", 1, new DWORD[1]{ PREFERRED_PSTATE_ADAPTIVE }); //Could Have Also Used PREFERRED_PSTATE_PREFER_MIN But TBH If your on High Performance Adaptive should be the lowest setting
const EnumPwR PWR_AUTO(L"Driver Controlled", 2, new DWORD[1] { PREFERRED_PSTATE_DRIVER_CONTROLLED });
const EnumPwR PWR_OPTIMAL(L"Optimal Power", 3, new DWORD[1] { PREFERRED_PSTATE_OPTIMAL_POWER });
const EnumPwR PWR_HIGH(L"High Peformance", 4, new DWORD[1] { PREFERRED_PSTATE_PREFER_MAX });
const EnumPwR PWR_HIGH_CONSISTENT(L"High Performance Consistent", 5, new DWORD[1]{ PREFERRED_PSTATE_PREFER_CONSISTENT_PERFORMANCE });

EnumPwR ENUMPWRS[] = { PWR_OFF, PWR_SAVING, PWR_AUTO, PWR_OPTIMAL, PWR_HIGH, PWR_HIGH_CONSISTENT };

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

void PrintGUID(GUID* guid, bool nline)
{
	printf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	if(nline)
		cout << endl;
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

void PrintGUID(GUID* guid)
{
	PrintGUID(guid, true);
}

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
	UINT32 val = 300;
	ULONG t = REG_DWORD;
	DWORD size = 4;
	int ERR = PowerReadACValue(NULL, ACTIVE, SUB, SETTING, &t, (UCHAR *)&val, &size);
	return ERR == ERROR_SUCCESS && val != 300;
}

string ToString(bool b)
{
	return b ? "True" : "False";
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
	UINT32 val = 300;
	ULONG t = REG_DWORD;
	DWORD size = 4;
	int ERR = ac ? PowerReadACValue(NULL, pp, sub, setting, &t, (UCHAR *)&val, &size) : PowerReadDCValue(NULL, pp, sub, setting, &t, (UCHAR *)&val, &size);
	return val;
}

void NVAPIError(NvAPI_Status status, std::wstring id)
{
	NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	std::wcerr << L"NVAPI Error: " << szDesc << id << std::endl;
}

#include <sstream>
#include <iomanip>
wstring ToHex(DWORD v)
{
	std::wstringstream ss;
	ss << L"0x" << std::uppercase << std::setfill(L'0') << std::setw(8) << std::hex << v;
	return ss.str();
}


void SyncToNVAPI(DWORD PrefGPU, DWORD GPUPwRLVL)
{
	EnumPwR ENUM_PWR = NONE;
	EnumPwR ENUM_GRAPHICS = NONE;
	bool HasFoundPWR = false;
	bool HasFoundG = false;
	for (auto v : ENUMPWRS)
	{
		if (v.PwRValue == GPUPwRLVL)
		{
			ENUM_PWR = v;
			HasFoundPWR = true;
			break;
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
	if (!HasFoundG || !HasFoundPWR)
	{
		wcout << L"Critical ERROR EnumPwR Not Found" << endl;
		exit(-1);
	}

	//Both Modules Are Disabled Return From Doing Any Operations
	if (ENUM_GRAPHICS.PwRValue == 0 && ENUM_GRAPHICS.PwRValue == 0)
		return;

	NvAPI_Status status = NvAPI_Initialize();
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"INIT");
		exit(-1);
	}
	NvDRSSessionHandle hSession = 0;
	status = NvAPI_DRS_CreateSession(&hSession);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"SESSION");
		exit(-1);
	}
	status = NvAPI_DRS_LoadSettings(hSession);
	if (status != NVAPI_OK)
	{
		NVAPIError(status, L"LOAD_SETTINGS");
		exit(-1);
	}

	NvDRSProfileHandle hProfile = 0;
	status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
	if (status != NVAPI_OK)
		NVAPIError(status, L"GET_BASE_PROFILE");

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

	if (ENUM_GRAPHICS.PwRValue != 0)
	{
		NVDRS_SETTING gsetting1 = { 0 };
		gsetting1.version = NVDRS_SETTING_VER;
		gsetting1.settingId = SHIM_MCCOMPAT_ID;
		gsetting1.settingType = NVDRS_DWORD_TYPE;
		gsetting1.u32CurrentValue = ENUM_GRAPHICS.NVAPIValue[0];

		NVDRS_SETTING gsetting2 = { 0 };
		gsetting2.version = NVDRS_SETTING_VER;
		gsetting2.settingId = SHIM_RENDERING_MODE_ID;
		gsetting2.settingType = NVDRS_DWORD_TYPE;
		gsetting2.u32CurrentValue = ENUM_GRAPHICS.NVAPIValue[1];

		NVDRS_SETTING gsetting3 = { 0 };
		gsetting3.version = NVDRS_SETTING_VER;
		gsetting3.settingId = SHIM_RENDERING_OPTIONS_ID;
		gsetting3.settingType = NVDRS_DWORD_TYPE;
		gsetting3.u32CurrentValue = ENUM_GRAPHICS.NVAPIValue[2];

		status = NvAPI_DRS_SetSetting(hSession, hProfile, &gsetting1);
		if (status != NVAPI_OK)
			NVAPIError(status, L"PREF_GPU_1");
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &gsetting2);
		if (status != NVAPI_OK)
			NVAPIError(status, L"PREF_GPU_2");
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &gsetting3);
		if (status != NVAPI_OK)
			NVAPIError(status, L"PREF_GPU_3");
	}

	// Save Changes
	status = NvAPI_DRS_SaveSettings(hSession);
	if (status != NVAPI_OK)
		NVAPIError(status, L"SAVING");
	// Cleanup
	NvAPI_DRS_DestroySession(hSession);
}

void SyncFromNVAPI(GUID* CurrentPP)
{
	//TODO Get NVAPI Current Settings and then change them from the power plan
	PowerSetActiveScheme(NULL, CurrentPP);//Sync Changes Instantly
}

int main() {
	GUID* OrgGUID;
	if (PowerGetActiveScheme(NULL, &OrgGUID) != ERROR_SUCCESS)
	{
		std::cerr << "Failed to get active power scheme." << std::endl;
		return -1;
	}

	//Create the Settings If They do not exist
	if (!HasACValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS))
	{
		CreateSettings(OrgGUID);
	}

	//Start the Main Loop of the program
	GUID* CurrentGUID;
	bool IsAC = IsACPwr();
	DWORD OrgPGP = GetPwrValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS, IsAC);//Preffered Graphics Processor
	DWORD OrgGP = GetPwrValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, IsAC);//Graphics Power Level
	DWORD CurrentPGP = OrgPGP;//Preffered Graphics Processor
	DWORD CurrentGP = OrgGP;//Graphics Power Level
	SyncToNVAPI(CurrentPGP, CurrentGP);
	while (true)
	{
		Sleep(2500);
		IsAC = IsACPwr();
		PowerGetActiveScheme(NULL, &CurrentGUID);
		if (!IsEqual(OrgGUID, CurrentGUID))
		{
			//Force Update on Settings
			OrgGUID = CurrentGUID;
			//Update Values
			CurrentPGP = GetPwrValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS, IsAC);//Preffered Graphics Processor
			CurrentGP =  GetPwrValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, IsAC);//Graphics Power Level
		}
		
		//Detect Changes From the Power Plan
		if (OrgPGP != CurrentPGP || OrgGP != CurrentGP)
		{
			OrgPGP = CurrentPGP;
			OrgGP = CurrentGP;
			SyncToNVAPI(CurrentPGP, CurrentGP);
		}
		//TODO SYNC NVIDIA with the power plan
		else
		{
			SyncFromNVAPI(CurrentGUID);
			//Update Checker Values
			CurrentPGP = GetPwrValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS, IsAC);//Preffered Graphics Processor
			CurrentGP = GetPwrValue(OrgGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS_PWR, IsAC);//Graphics Power Level
			OrgPGP = CurrentPGP;
			OrgGP = CurrentGP;
		}
	}
}
