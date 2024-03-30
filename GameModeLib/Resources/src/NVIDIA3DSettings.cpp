//NVIDIA3D Settings
//3d Settings --> Preffered Graphics Processor --> High Performance
//3d Settings --> VSYNC --> Application Settings
//3d Settings --> PowerMode --> Optimal Power

#include "Windows.h"
#include "nvapi.h"
#include "NvApiDriverSettings.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iomanip>
#include <fcntl.h>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include <io.h>
#include <iostream>

#pragma comment (lib, "nvapi.lib")

static bool HasVSYNC = true;
static bool HasPwr = true;
static bool ForceAuto = false;
static bool ForceIntegrated = false;
static bool SkipDefaultExports = false;
static bool ForceOptimal = false;
static const DWORD SETTING_DEFAULT_VALUE = 4294967295;
static DWORD SHIM_FIXED = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE;

void PrintError(NvAPI_Status status, std::wstring id)
{
	NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	std::wcerr << L"NVAPI Error: " << szDesc << id << std::endl;
}

void PrintError(NvAPI_Status status)
{
	PrintError(status, L"");
}

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

bool DisplayProfileContents(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile)
{
	// (0) this function assumes that the hSession and hProfile are
	// valid handles obtained from nvapi.
	NvAPI_Status status;
	// (1) First, we retrieve generic profile information
	// The structure will provide us with name, number of applications
	// and number of settings for this profile.
	NVDRS_PROFILE profileInformation = { 0 };
	profileInformation.version = NVDRS_PROFILE_VER;
	status = NvAPI_DRS_GetProfileInfo(hSession,
		hProfile,
		&profileInformation);
	if (status != NVAPI_OK) {
		PrintError(status, L"GET_PROFILE");
		return false;
	}
	wprintf(L"Profile Name: %s\n", profileInformation.profileName);
	printf("Number of Applications associated with the Profile: %d\n",
		profileInformation.numOfApps);
	printf("Number of Settings associated with the Profile: %d\n",
		profileInformation.numOfSettings);
	printf("Is Predefined: %d\n", profileInformation.isPredefined);
	// (2) Now we enumerate through all the applications on the profile,
	// if there is any
	if (profileInformation.numOfApps > 0) {
		NVDRS_APPLICATION *appArray =
			new NVDRS_APPLICATION[profileInformation.numOfApps];
		NvU32 numAppsRead = profileInformation.numOfApps, i;
		// (3) It is possible to enumerate all applications one by one,
		// or all at once on a preallocated array. The numAppsRead
		// represents the number of NVDRS_APPLICATION structures
		// allocated in the array. It will be modified on return of the
		// function contain the number of actual applications that have
		// been filled by NVAPI
		appArray[0].version = NVDRS_APPLICATION_VER;
		status = NvAPI_DRS_EnumApplications(hSession,
			hProfile,
			0,
			&numAppsRead,
			appArray);
		if (status != NVAPI_OK) {
			PrintError(status, L"GET_PROFILE_INFO");
			delete[] appArray;
			return false;
		}
		for (i = 0; i < numAppsRead; i++) {
			wprintf(L"Executable: %s\n", appArray[i].appName);
			wprintf(L"User Friendly Name: %s\n",
				appArray[i].userFriendlyName);
			printf("Is Predefined: %d\n", appArray[i].isPredefined);
		}
		delete[] appArray;
	}
	// (4) Now we enumerate all the settings on the profile
	if (profileInformation.numOfSettings > 0) {
		NVDRS_SETTING *setArray =
			new NVDRS_SETTING[profileInformation.numOfSettings];
		NvU32 numSetRead = profileInformation.numOfSettings, i;
		// (5) The function to retrieve the settings in a profile works
		// like the function to retrieve the applications.
		setArray[0].version = NVDRS_SETTING_VER;
		status = NvAPI_DRS_EnumSettings(hSession,
			hProfile,
			0,
			&numSetRead,
			setArray);
		if (status != NVAPI_OK) {
			PrintError(status, L"GET_PROFILE_ENUM_SETTINGS");
			return false;
		}
		for (i = 0; i < numSetRead; i++) {
			/*if (setArray[i].settingLocation !=
				NVDRS_CURRENT_PROFILE_LOCATION) {
				// (6) The settings that are not from the Current Profile
				// are inherited from the Base or Global profiles. Skip them.
				continue;
			}*/
			std::wcout << setArray[i].settingName << std::endl;
			wprintf(L"Setting Name: %s\n", setArray[i].settingName);
			printf("Setting ID: %X\n", setArray[i].settingId);
			printf("Predefined? : %d\n",
				setArray[i].isCurrentPredefined);
			switch (setArray[i].settingType) {
				// (7) a setting can be of different types and be using
				// different fields on the NVDRS_SETTING union
			case NVDRS_DWORD_TYPE:
				printf("Setting Value DWORD: %X\n",
					setArray[i].u32CurrentValue);
				break;
			case NVDRS_BINARY_TYPE:
			{
				//unsigned int len;
				printf("Setting Binary (length=%d) :",
					setArray[i].binaryCurrentValue.valueLength);
				/*for (len = 0;
				len < setArray[i].binaryCurrentValue.valueLength;
					len++)
				{
					printf(" %02x",
						setArray[i].binaryCurrentValue.valueData[len]);
				}
				printf("\n");*/
			}
			break;
			case NVDRS_WSTRING_TYPE:
				wprintf(L"Setting Value SZ: %s",
					setArray[i].wszCurrentValue);
				break;
			}
		}
	}
	printf("\n");
	return true;
}

/**
void EnumerateProfilesOnSystem(NvDRSSessionHandle hSession)
{
	NvAPI_Status status;
	NvDRSProfileHandle hProfile = 0;
	unsigned int index = 0;
	std::string pname = "3D App - Default Global Settings";
	while ((status = NvAPI_DRS_EnumProfiles(hSession, index, &hProfile)) == NVAPI_OK)
	{
		index++;
		NVDRS_PROFILE profileInformation = { 0 };
		profileInformation.version = NVDRS_PROFILE_VER;
		status = NvAPI_DRS_GetProfileInfo(hSession, hProfile, &profileInformation);
		wprintf(L"Profile Name: %s\n", profileInformation.profileName);
	}
	if (status != NVAPI_END_ENUMERATION && status != NVAPI_OK)
		PrintError(status);
}*/

std::wstring ToWString(bool b)
{
	return b ? L"True" : L"False";
}

std::wstring ToHex(NvU32 v)
{
	std::wstringstream ss;
	ss << L"0x" << std::uppercase << std::setfill(L'0') << std::setw(8) << std::hex << v;
	return ss.str();
}

std::vector<std::wstring> Split(const std::wstring& input, wchar_t c) {
	std::vector<std::wstring> arr;
	size_t startPos = 0;
	size_t foundPos = input.find(c, startPos);
	while (foundPos != std::wstring::npos)
	{
		std::wstring sub = input.substr(startPos, foundPos - startPos);
		arr.push_back(sub);
		startPos = foundPos + 1;
		foundPos = input.find(c, startPos);
	}
	std::wstring lastSub = input.substr(startPos);
	arr.push_back(lastSub);
	return arr;
}

DWORD FromHex(std::wstring v)
{
	return std::stoul(v, nullptr, 16);
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

int IndexOf(std::wstring str, std::wstring key)
{
	size_t found = str.find(key);
	if (found != std::string::npos)
		return static_cast<int>(found);
	return -1;
}

void SetSettings(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile)
{
	NVDRS_SETTING USetting = { 0 };
	USetting.version = NVDRS_SETTING_VER;
	USetting.settingType = NVDRS_DWORD_TYPE;
	NvAPI_Status ustatus = NvAPI_DRS_GetSetting(hSession, hProfile, VSYNCMODE_ID, &USetting);
	std::wcout << ToHex(VSYNCMODE_ID) << L"=" << ToHex((ustatus == NVAPI_OK ? USetting.u32CurrentValue : VSYNCMODE_PASSIVE));
	USetting = { 0 };
	USetting.version = NVDRS_SETTING_VER;
	USetting.settingType = NVDRS_DWORD_TYPE;
	ustatus = NvAPI_DRS_GetSetting(hSession, hProfile, PREFERRED_PSTATE_ID, &USetting);
	NvU32 pwr = USetting.u32CurrentValue;
	std::wcout << L";" << ToHex(PREFERRED_PSTATE_ID) << L"=" << ToHex(ustatus == NVAPI_OK ? pwr : PREFERRED_PSTATE_OPTIMAL_POWER);
	USetting = { 0 };
	USetting.version = NVDRS_SETTING_VER;
	USetting.settingType = NVDRS_DWORD_TYPE;
	ustatus = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_MCCOMPAT_ID, &USetting);
	std::wcout << L";" << ToHex(SHIM_MCCOMPAT_ID) << L"=" << ToHex(ustatus == NVAPI_OK ? USetting.u32CurrentValue : SHIM_MCCOMPAT_AUTO_SELECT);
	USetting = { 0 };
	USetting.version = NVDRS_SETTING_VER;
	USetting.settingType = NVDRS_DWORD_TYPE;
	ustatus = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_RENDERING_MODE_ID, &USetting);
	std::wcout << L";" << ToHex(SHIM_RENDERING_MODE_ID) << L"=" << ToHex(ustatus == NVAPI_OK ? USetting.u32CurrentValue : SHIM_RENDERING_MODE_AUTO_SELECT);
	USetting = { 0 };
	USetting.version = NVDRS_SETTING_VER;
	USetting.settingType = NVDRS_DWORD_TYPE;
	ustatus = NvAPI_DRS_GetSetting(hSession, hProfile, SHIM_RENDERING_OPTIONS_ID, &USetting);
	std::wcout << L";" << ToHex(SHIM_RENDERING_OPTIONS_ID) << L"=" << ToHex(ustatus == NVAPI_OK ? USetting.u32CurrentValue : SHIM_FIXED);
	std::wcout << std::endl;

	//Start Setting Settings
	NvAPI_Status status;

	//Set V-SYNC to Default Value
	if (HasVSYNC)
	{
		NVDRS_SETTING drsSetting = { 0 };
		drsSetting.version = NVDRS_SETTING_VER;
		drsSetting.settingId = VSYNCMODE_ID;
		drsSetting.settingType = NVDRS_DWORD_TYPE;
		drsSetting.u32CurrentValue = VSYNCMODE_PASSIVE;
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting);
		if (status != NVAPI_OK)
			PrintError(status, L"V-SYNC");
	}

	//Set PowerMode to PREFERRED_PSTATE_OPTIMAL_POWER if it's not max performance already
	if (HasPwr && (ForceOptimal || pwr != PREFERRED_PSTATE_PREFER_MAX && pwr != PREFERRED_PSTATE_PREFER_CONSISTENT_PERFORMANCE))
	{
		NVDRS_SETTING drsSetting = { 0 };
		drsSetting.version = NVDRS_SETTING_VER;
		drsSetting.settingId = PREFERRED_PSTATE_ID;
		drsSetting.settingType = NVDRS_DWORD_TYPE;
		drsSetting.u32CurrentValue = PREFERRED_PSTATE_OPTIMAL_POWER;
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting);
		if (status != NVAPI_OK)
			PrintError(status, L"POWER");
	}

	//Set Preffered Graphics Processor to High Performance
	NVDRS_SETTING drsSetting1 = { 0 };
	drsSetting1.version = NVDRS_SETTING_VER;
	drsSetting1.settingId = SHIM_MCCOMPAT_ID;
	drsSetting1.settingType = NVDRS_DWORD_TYPE;

	NVDRS_SETTING drsSetting2 = { 0 };
	drsSetting2.version = NVDRS_SETTING_VER;
	drsSetting2.settingId = SHIM_RENDERING_MODE_ID;
	drsSetting2.settingType = NVDRS_DWORD_TYPE;

	NVDRS_SETTING drsSetting3 = { 0 };
	drsSetting3.version = NVDRS_SETTING_VER;
	drsSetting3.settingId = SHIM_RENDERING_OPTIONS_ID;
	drsSetting3.settingType = NVDRS_DWORD_TYPE;

	//Force Automatic NVIDIA Selection of Graphics
	if (ForceAuto)
	{
		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_AUTO_SELECT;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_AUTO_SELECT;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting3.u32CurrentValue = SHIM_FIXED;
	}
	//Force Integrated Graphics
	else if (ForceIntegrated)
	{
		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_INTEGRATED;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_INTEGRATED;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting3.u32CurrentValue = SHIM_FIXED | SHIM_RENDERING_OPTIONS_IGPU_TRANSCODING;
	}
	//Force Dedicated Graphics
	else
	{
		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_ENABLE;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_ENABLE;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting3.u32CurrentValue = SHIM_FIXED;
	}

	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting1);
	if (status != NVAPI_OK)
		PrintError(status, L"OPTIMUS_1");

	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting2);
	if (status != NVAPI_OK)
		PrintError(status, L"OPTIMUS_2");

	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting3);
	//Handle Integrated Graphics Flags Error
	if (status != NVAPI_OK)
	{
		std::wcerr << L"Falling Back From Graphics To SHIM_FIX for SHIM_RENDERING_OPTIONS_ID" << std::endl;
		NVDRS_SETTING fallback = { 0 };
		fallback.version = NVDRS_SETTING_VER;
		fallback.settingId = SHIM_RENDERING_OPTIONS_ID;
		fallback.settingType = NVDRS_DWORD_TYPE;
		fallback.u32CurrentValue = SHIM_FIXED;
		status = NvAPI_DRS_SetSetting(hSession, hProfile, &fallback);
		if (status != NVAPI_OK)
			PrintError(status, L"OPTIMUS_3");
	}
}

void SetSettings(NvDRSSessionHandle hSession, NvDRSProfileHandle hProfile, std::map<DWORD, DWORD> map)
{
	int index = 0;
	std::map<DWORD, DWORD>::iterator it;
	for (it = map.begin(); it != map.end(); it++)
	{
		DWORD SETTING_ID = it->first;
		DWORD SETTING_VALUE = it->second;

		//Print an NVIDIA Setting before setting it
		NVDRS_SETTING USetting = { 0 };
		USetting.version = NVDRS_SETTING_VER;
		USetting.settingType = NVDRS_DWORD_TYPE;
		NvAPI_Status ustatus = NvAPI_DRS_GetSetting(hSession, hProfile, SETTING_ID, &USetting);
		std::wcout << (index != 0 ? L";" : L"") << ToHex(SETTING_ID) << L"=" << (ustatus == NVAPI_OK ? ToHex(USetting.u32CurrentValue) : L"Default");

		if (SETTING_VALUE != SETTING_DEFAULT_VALUE)
		{
			//Set an NVIDIA Control Pannel Setting
			NVDRS_SETTING setting = { 0 };
			setting.version = NVDRS_SETTING_VER;
			setting.settingId = SETTING_ID;
			setting.settingType = NVDRS_DWORD_TYPE;
			setting.u32CurrentValue = SETTING_VALUE;
			NvAPI_Status status = NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
			if (status != NVAPI_OK)
				PrintError(status, L"Setting ID:" + ToHex(SETTING_ID));
		}
		//Restore Default Imports Back to their Default Value
		else
		{
			NvAPI_DRS_RestoreProfileDefaultSetting(hSession, hProfile, SETTING_ID);
		}
		index++;
	}
	std::wcout << std::endl;
}

void RestoreDefaults(NvDRSSessionHandle hSession, NvDRSProfileHandle Base, NvDRSProfileHandle Global)
{
	NvAPI_Status status = NvAPI_DRS_RestoreProfileDefault(hSession, Base);
	if (status != NVAPI_OK)
		PrintError(status, L"RESTORE_BASE_PROFILE");
	status = NvAPI_DRS_RestoreProfileDefault(hSession, Global);
	if (status != NVAPI_OK)
		PrintError(status, L"RESTORE_GLOBAL_PROFILE");
}

void Help()
{
	std::wcout << L"NVIDIA3DSettings.exe" << std::endl;
	std::wcout << L"NVIDIA3DSettings.exe Import <SETTING_ID=DWORD_VALUE;SETTING_ID2=DWORDVALUE2>" << std::endl;
	std::wcout << L"NVIDIA3DSettings.exe Export" << std::endl;
	std::wcout << L"NVIDIA3DSettings.exe Query <SETTING_ID;SETTING_ID=DEFAULT_VALUE>" << std::endl;
	std::wcout << L"NOTE: Setting IDs & Values Must be in hex 0x00A879CF (Base 16 Hex Format Prefixed with 0x)" << std::endl;
	std::wcout << std::endl;
	std::wcout << L"/Restore  Restores NVIDIA 3D Settings Before the Command Runs" << std::endl;
	std::wcout << L"/Restore:true  Restores NVIDIA 3D Settings After Exporting or Querying" << std::endl;
	std::wcout << L"/SkipDefaultExports  Skips All Default Profile Settings when Exporting" << std::endl;
	std::wcout << std::endl;
	std::wcout << L"N/A While Importing / Exporting Or Querying Flags Below" << std::endl;
	std::wcout << L"/NoVSYNC  Doesn't Set VSYNC" << std::endl;
	std::wcout << L"/NoPower /NoPWR  Doesn't Set GPU Power Mode" << std::endl;
	std::wcout << L"/ForceIntegrated /Integrated  Sets Integrated Graphics" << std::endl;
	std::wcout << L"/Auto /ForceAuto  Sets Graphics to Auo Mode" << std::endl;
	std::wcout << L"/ForceOptimal  Sets Graphics Power to Optimal Regardless of High Performance" << std::endl;
	exit(0);
}

void ExportProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle profile, std::map<DWORD, DWORD> &expmap)
{
	//INITIALIZE THE MAP
	std::map<std::wstring, NvU32> SETTINGS_GLOBAL;
	SETTINGS_GLOBAL[L"OGL_AA_LINE_GAMMA_ID"] = 0x2089BF6C;
	SETTINGS_GLOBAL[L"OGL_CPL_GDI_COMPATIBILITY_ID"] = 0x2072C5A3;
	SETTINGS_GLOBAL[L"OGL_CPL_PREFER_DXPRESENT_ID"] = 0x20D690F8;
	SETTINGS_GLOBAL[L"OGL_DEEP_COLOR_SCANOUT_ID"] = 0x2097C2F6;
	SETTINGS_GLOBAL[L"OGL_DEFAULT_SWAP_INTERVAL_ID"] = 0x206A6582;
	SETTINGS_GLOBAL[L"OGL_DEFAULT_SWAP_INTERVAL_FRACTIONAL_ID"] = 0x206C4581;
	SETTINGS_GLOBAL[L"OGL_DEFAULT_SWAP_INTERVAL_SIGN_ID"] = 0x20655CFA;
	SETTINGS_GLOBAL[L"OGL_EVENT_LOG_SEVERITY_THRESHOLD_ID"] = 0x209DF23E;
	SETTINGS_GLOBAL[L"OGL_EXTENSION_STRING_VERSION_ID"] = 0x20FF7493;
	SETTINGS_GLOBAL[L"OGL_FORCE_BLIT_ID"] = 0x201F619F;
	SETTINGS_GLOBAL[L"OGL_FORCE_STEREO_ID"] = 0x204D9A0C;
	SETTINGS_GLOBAL[L"OGL_IMPLICIT_GPU_AFFINITY_ID"] = 0x20D0F3E6;
	SETTINGS_GLOBAL[L"OGL_MAX_FRAMES_ALLOWED_ID"] = 0x208E55E3;
	SETTINGS_GLOBAL[L"OGL_OVERLAY_PIXEL_TYPE_ID"] = 0x209AE66F;
	SETTINGS_GLOBAL[L"OGL_OVERLAY_SUPPORT_ID"] = 0x206C28C4;
	SETTINGS_GLOBAL[L"OGL_QUALITY_ENHANCEMENTS_ID"] = 0x20797D6C;
	SETTINGS_GLOBAL[L"OGL_SINGLE_BACKDEPTH_BUFFER_ID"] = 0x20A29055;
	SETTINGS_GLOBAL[L"OGL_SLI_MULTICAST_ID"] = 0x2092D3BE;
	SETTINGS_GLOBAL[L"OGL_THREAD_CONTROL_ID"] = 0x20C1221E;
	SETTINGS_GLOBAL[L"OGL_TMON_LEVEL_ID"] = 0x202888C1;
	SETTINGS_GLOBAL[L"OGL_TRIPLE_BUFFER_ID"] = 0x20FDD1F9;
	SETTINGS_GLOBAL[L"AA_BEHAVIOR_FLAGS_ID"] = 0x10ECDB82;
	SETTINGS_GLOBAL[L"AA_MODE_ALPHATOCOVERAGE_ID"] = 0x10FC2D9C;
	SETTINGS_GLOBAL[L"AA_MODE_GAMMACORRECTION_ID"] = 0x107D639D;
	SETTINGS_GLOBAL[L"AA_MODE_METHOD_ID"] = 0x10D773D2;
	SETTINGS_GLOBAL[L"AA_MODE_REPLAY_ID"] = 0x10D48A85;
	SETTINGS_GLOBAL[L"AA_MODE_SELECTOR_ID"] = 0x107EFC5B;
	SETTINGS_GLOBAL[L"AA_MODE_SELECTOR_SLIAA_ID"] = 0x107AFC5B;
	SETTINGS_GLOBAL[L"ANISO_MODE_LEVEL_ID"] = 0x101E61A9;
	SETTINGS_GLOBAL[L"ANISO_MODE_SELECTOR_ID"] = 0x10D2BB16;
	SETTINGS_GLOBAL[L"ANSEL_ALLOW_ID"] = 0x1035DB89;
	SETTINGS_GLOBAL[L"ANSEL_ALLOWLISTED_ID"] = 0x1085DA8A;
	SETTINGS_GLOBAL[L"ANSEL_ENABLE_ID"] = 0x1075D972;
	SETTINGS_GLOBAL[L"APPIDLE_DYNAMIC_FRL_FPS_ID"] = 0x10835016;
	SETTINGS_GLOBAL[L"APPIDLE_DYNAMIC_FRL_THRESHOLD_TIME_ID"] = 0x10835017;
	SETTINGS_GLOBAL[L"APPLICATION_PROFILE_NOTIFICATION_TIMEOUT_ID"] = 0x104554B6;
	SETTINGS_GLOBAL[L"APPLICATION_STEAM_ID_ID"] = 0x107CDDBC;
	SETTINGS_GLOBAL[L"BATTERY_BOOST_APP_FPS_ID"] = 0x10115C8C;
	SETTINGS_GLOBAL[L"CPL_HIDDEN_PROFILE_ID"] = 0x106D5CFF;
	SETTINGS_GLOBAL[L"CUDA_EXCLUDED_GPUS_ID"] = 0x10354FF8;
	SETTINGS_GLOBAL[L"D3DOGL_GPU_MAX_POWER_ID"] = 0x10D1EF29;
	SETTINGS_GLOBAL[L"EXPORT_PERF_COUNTERS_ID"] = 0x108F0841;
	SETTINGS_GLOBAL[L"EXTERNAL_QUIET_MODE_ID"] = 0x10115C8D;
	SETTINGS_GLOBAL[L"FRL_FPS_ID"] = 0x10835002;
	SETTINGS_GLOBAL[L"FXAA_ALLOW_ID"] = 0x1034CB89;
	SETTINGS_GLOBAL[L"FXAA_ENABLE_ID"] = 0x1074C972;
	SETTINGS_GLOBAL[L"FXAA_INDICATOR_ENABLE_ID"] = 0x1068FB9C;
	SETTINGS_GLOBAL[L"LATENCY_INDICATOR_AUTOALIGN_ID"] = 0x1095F170;
	SETTINGS_GLOBAL[L"MCSFRSHOWSPLIT_ID"] = 0x10287051;
	SETTINGS_GLOBAL[L"NV_QUALITY_UPSCALING_ID"] = 0x10444444;
	SETTINGS_GLOBAL[L"OPTIMUS_MAXAA_ID"] = 0x10F9DC83;
	SETTINGS_GLOBAL[L"PHYSXINDICATOR_ID"] = 0x1094F16F;
	SETTINGS_GLOBAL[L"PREFERRED_PSTATE_ID"] = 0x1057EB71;
	SETTINGS_GLOBAL[L"PREVENT_UI_AF_OVERRIDE_ID"] = 0x103BCCB5;
	SETTINGS_GLOBAL[L"SHIM_MAXRES_ID"] = 0x10F9DC82;
	SETTINGS_GLOBAL[L"SHIM_MCCOMPAT_ID"] = 0x10F9DC80;
	SETTINGS_GLOBAL[L"SHIM_RENDERING_MODE_ID"] = 0x10F9DC81;
	SETTINGS_GLOBAL[L"SHIM_RENDERING_OPTIONS_ID"] = 0x10F9DC84;
	SETTINGS_GLOBAL[L"SLI_GPU_COUNT_ID"] = 0x1033DCD1;
	SETTINGS_GLOBAL[L"SLI_PREDEFINED_GPU_COUNT_ID"] = 0x1033DCD2;
	SETTINGS_GLOBAL[L"SLI_PREDEFINED_GPU_COUNT_DX10_ID"] = 0x1033DCD3;
	SETTINGS_GLOBAL[L"SLI_PREDEFINED_MODE_ID"] = 0x1033CEC1;
	SETTINGS_GLOBAL[L"SLI_PREDEFINED_MODE_DX10_ID"] = 0x1033CEC2;
	SETTINGS_GLOBAL[L"SLI_RENDERING_MODE_ID"] = 0x1033CED1;
	SETTINGS_GLOBAL[L"VRPRERENDERLIMIT_ID"] = 0x10111133;
	SETTINGS_GLOBAL[L"VRRFEATUREINDICATOR_ID"] = 0x1094F157;
	SETTINGS_GLOBAL[L"VRROVERLAYINDICATOR_ID"] = 0x1095F16F;
	SETTINGS_GLOBAL[L"VRRREQUESTSTATE_ID"] = 0x1094F1F7;
	SETTINGS_GLOBAL[L"VRR_APP_OVERRIDE_ID"] = 0x10A879CF;
	SETTINGS_GLOBAL[L"VRR_APP_OVERRIDE_REQUEST_STATE_ID"] = 0x10A879AC;
	SETTINGS_GLOBAL[L"VRR_MODE_ID"] = 0x1194F158;
	SETTINGS_GLOBAL[L"VSYNCSMOOTHAFR_ID"] = 0x101AE763;
	SETTINGS_GLOBAL[L"VSYNCVRRCONTROL_ID"] = 0x10A879CE;
	SETTINGS_GLOBAL[L"VSYNC_BEHAVIOR_FLAGS_ID"] = 0x10FDEC23;
	SETTINGS_GLOBAL[L"WKS_API_STEREO_EYES_EXCHANGE_ID"] = 0x11AE435C;
	SETTINGS_GLOBAL[L"WKS_API_STEREO_MODE_ID"] = 0x11E91A61;
	SETTINGS_GLOBAL[L"WKS_MEMORY_ALLOCATION_POLICY_ID"] = 0x11112233;
	SETTINGS_GLOBAL[L"WKS_STEREO_DONGLE_SUPPORT_ID"] = 0x112493BD;
	SETTINGS_GLOBAL[L"WKS_STEREO_SUPPORT_ID"] = 0x11AA9E99;
	SETTINGS_GLOBAL[L"WKS_STEREO_SWAP_MODE_ID"] = 0x11333333;
	SETTINGS_GLOBAL[L"AO_MODE_ID"] = 0x00667329;
	SETTINGS_GLOBAL[L"AO_MODE_ACTIVE_ID"] = 0x00664339;
	SETTINGS_GLOBAL[L"AUTO_LODBIASADJUST_ID"] = 0x00638E8F;
	SETTINGS_GLOBAL[L"EXPORT_PERF_COUNTERS_DX9_ONLY_ID"] = 0x00B65E72;
	SETTINGS_GLOBAL[L"ICAFE_LOGO_CONFIG_ID"] = 0x00DB1337;
	SETTINGS_GLOBAL[L"LODBIASADJUST_ID"] = 0x00738E8F;
	SETTINGS_GLOBAL[L"MAXWELL_B_SAMPLE_INTERLEAVE_ID"] = 0x0098C1AC;
	SETTINGS_GLOBAL[L"PRERENDERLIMIT_ID"] = 0x007BA09E;
	SETTINGS_GLOBAL[L"PS_SHADERDISKCACHE_ID"] = 0x00198FFF;
	SETTINGS_GLOBAL[L"PS_SHADERDISKCACHE_DLL_PATH_WCHAR_ID"] = 0x0019A002;
	SETTINGS_GLOBAL[L"PS_SHADERDISKCACHE_FLAGS_ID"] = 0x00F4889B;
	SETTINGS_GLOBAL[L"PS_SHADERDISKCACHE_MAX_SIZE_ID"] = 0x00AC8497;
	SETTINGS_GLOBAL[L"PS_TEXFILTER_ANISO_OPTS2_ID"] = 0x00E73211;
	SETTINGS_GLOBAL[L"PS_TEXFILTER_BILINEAR_IN_ANISO_ID"] = 0x0084CD70;
	SETTINGS_GLOBAL[L"PS_TEXFILTER_DISABLE_TRILIN_SLOPE_ID"] = 0x002ECAF2;
	SETTINGS_GLOBAL[L"PS_TEXFILTER_NO_NEG_LODBIAS_ID"] = 0x0019BB68;
	SETTINGS_GLOBAL[L"QUALITY_ENHANCEMENTS_ID"] = 0x00CE2691;
	SETTINGS_GLOBAL[L"QUALITY_ENHANCEMENT_SUBSTITUTION_ID"] = 0x00CE2692;
	SETTINGS_GLOBAL[L"REFRESH_RATE_OVERRIDE_ID"] = 0x0064B541;
	SETTINGS_GLOBAL[L"SET_POWER_THROTTLE_FOR_PCIe_COMPLIANCE_ID"] = 0x00AE785C;
	SETTINGS_GLOBAL[L"SET_VAB_DATA_ID"] = 0x00AB8687;
	SETTINGS_GLOBAL[L"VSYNCMODE_ID"] = 0x00A879CF;
	SETTINGS_GLOBAL[L"VSYNCTEARCONTROL_ID"] = 0x005A375C;

	//Populate the Map From all Known Settings
	std::map<std::wstring, NvU32>::iterator it;
	for (it = SETTINGS_GLOBAL.begin(); it != SETTINGS_GLOBAL.end(); it++)
	{
		NVDRS_SETTING SETTING_GET = { 0 };
		SETTING_GET.version = NVDRS_SETTING_VER;
		SETTING_GET.settingType = NVDRS_DWORD_TYPE;
		DWORD SETTING_ID = (DWORD)it->second;
		NvAPI_Status status_get = NvAPI_DRS_GetSetting(hSession, profile, SETTING_ID, &SETTING_GET);
		//Skip If SkipDefaults And Is NULL or Map Already has the Key and is NULL
		if ((SkipDefaultExports || expmap.count(SETTING_ID)) && status_get != NVAPI_OK)
			continue;
		expmap[SETTING_ID] = status_get == NVAPI_OK ? SETTING_GET.u32CurrentValue : SETTING_DEFAULT_VALUE;
	}

	//Iterate Dynamically Over all NVAPI Settings for the Profile
	NVDRS_PROFILE profileInformation = { 0 };
	profileInformation.version = NVDRS_PROFILE_VER;
	NvAPI_Status status = NvAPI_DRS_GetProfileInfo(hSession, profile, &profileInformation);
	if (status != NVAPI_OK)
		PrintError(status, L"EXPORT_GET_PROFILE_INFO");

	if (profileInformation.numOfSettings > 0)
	{
		NVDRS_SETTING *setArray = new NVDRS_SETTING[profileInformation.numOfSettings];
		NvU32 numSetRead = profileInformation.numOfSettings, i;
		setArray[0].version = NVDRS_SETTING_VER;
		status = NvAPI_DRS_EnumSettings(hSession, profile, 0, &numSetRead, setArray);
		if (status != NVAPI_OK)
		{
			PrintError(status, L"EXPORT_GET_PROFILE_SETTINGS");
			return;
		}
		else
		{
			//Print All Settings
			for (i = 0; i < numSetRead; i++)
			{
				NVDRS_SETTING setting = setArray[i];
				if (setting.settingType == NVDRS_DWORD_TYPE)
				{
					expmap[setting.settingId] = setting.u32CurrentValue;
				}
			}
		}
	}
}

void QueryProfile(NvDRSSessionHandle hSession, NvDRSProfileHandle profile, std::map<DWORD, DWORD> m)
{
	std::map<DWORD, DWORD>::iterator it;
	int i = 0;
	for (it = m.begin(); it != m.end(); it++)
	{
		DWORD SETTING_ID = it->first;
		DWORD VAL = it->second;
		bool hv = VAL != SETTING_DEFAULT_VALUE;

		NVDRS_SETTING SETTING_GET = { 0 };
		SETTING_GET.version = NVDRS_SETTING_VER;
		SETTING_GET.settingType = NVDRS_DWORD_TYPE;
		NvAPI_Status status_get = NvAPI_DRS_GetSetting(hSession, profile, SETTING_ID, &SETTING_GET);
		std::wcout << (i != 0 ? L";" : L"") << ToHex(SETTING_ID) << L"=" << (status_get == NVAPI_OK ? ToHex(SETTING_GET.u32CurrentValue) : (hv ? ToHex(VAL) : L"Default"));
		i++;
	}
	std::wcout << std::endl;
}

int main(int argc, char **argv)
{
	setlocale(LC_CTYPE, "");
	GenIsWins();
	if (IsWin7)
	{
		SHIM_FIXED = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE | SHIM_RENDERING_OPTIONS_HANDLE_WIN7_ASYNC_RUNTIME_BUG;
	}
	bool SetIds = false;
	bool Export = false;
	bool Restore = false;
	bool RestoreAfter = false;
	bool Query = false;
	std::map<DWORD, DWORD> map;
	std::vector<std::wstring> args;
	//Handle Arguments if any
	if (argc > 1)
	{
		//Handle the Arguments
		for (int i = 1; i < argc; i++)
		{
			std::wstring w = tolower(ctow(argv[i]));
			if (w == L"/?" || w == L"/help")
			{
				Help();
			}
			else if (w == L"/restore:true")
			{
				RestoreAfter = true;
			}
			else if (w == L"/restore" || w == L"/restore:false")
			{
				Restore = true;
			}
			else if (w == L"/novsync")
			{
				HasVSYNC = false;
			}
			else if (w == L"/auto" || w == L"/forceauto")
			{
				ForceAuto = true;
			}
			else if (w == L"/forceintegrated" || w == L"/integrated")
			{
				ForceIntegrated = true;
			}
			else if (w == L"/skipdefaultexports")
			{
				SkipDefaultExports = true;
			}
			else if (w == L"/forceoptimal")
			{
				ForceOptimal = true;
			}
			else if (w == L"/nopower" || w == L"/nopwr")
			{
				HasPwr = false;
			}
			else
			{
				args.push_back(w);
			}
		}
		if (static_cast<int>(args.size()) > 0)
		{
			std::wstring arg1 = args[0];
			if (arg1 == L"import" || arg1 == L"set")
			{
				SetIds = true;
				std::wstring strsettings = args[1];
				std::vector<std::wstring> arr = Split(strsettings, L';');
				for (auto it = std::begin(arr); it != std::end(arr); ++it)
				{
					std::wstring setting_entry = *it;
					int index_split = IndexOf(setting_entry, L"=");
					if (index_split == -1)
					{
						std::wcerr << L"ERROR Parsing:\"" << strsettings << "\"" << std::endl;
						exit(-1);
					}
					DWORD setting_id = FromHex(setting_entry.substr(0, index_split));
					std::wstring val = tolower(setting_entry.substr(index_split + 1));
					DWORD setting_value = (val == L"null" || val == L"default") ? SETTING_DEFAULT_VALUE : FromHex(val);
					map[setting_id] = setting_value;
				}
			}
			else if (arg1 == L"export")
			{
				Export = true;
			}
			else if (arg1 == L"query")
			{
				Query = true;
				std::wstring strsettings = args[1];
				std::vector<std::wstring> arr = Split(strsettings, L';');
				for (auto it = std::begin(arr); it != std::end(arr); ++it)
				{
					std::wstring setting_entry = *it;
					int index_split = IndexOf(setting_entry, L"=");
					//Parse No Default Values
					if (index_split == -1)
					{
						map[FromHex(setting_entry)] = SETTING_DEFAULT_VALUE;
						continue;
					}
					DWORD setting_id = FromHex(setting_entry.substr(0, index_split));
					DWORD setting_value = FromHex(setting_entry.substr(index_split + 1));
					map[setting_id] = setting_value;
				}
			}
			else if (arg1 == L"")
			{

			}
			//Help Command
			else
			{
				Help();
			}
		}
	}

	// (0) Initialize NVAPI. This must be done first of all
	NvAPI_Status status = NvAPI_Initialize();
	if (status != NVAPI_OK)
	{
		PrintError(status, L"INIT");
		exit(-1);
	}
	// (1) Create the session handle to access driver settings
	NvDRSSessionHandle hSession = 0;
	status = NvAPI_DRS_CreateSession(&hSession);
	if (status != NVAPI_OK)
	{
		PrintError(status, L"SESSION");
		exit(-1);
	}
	// (2) load all the system settings into the session
	status = NvAPI_DRS_LoadSettings(hSession);
	if (status != NVAPI_OK)
	{
		PrintError(status, L"GET_SETTINGS");
		exit(-1);
	}
	// (3) Obtain the Base profile. Any setting needs to be inside
	// a profile, putting a setting on the Base Profile enforces it
	// for all the processes on the system
	NvDRSProfileHandle hProfile = 0;
	status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
	if (status != NVAPI_OK)
		PrintError(status, L"GET_BASE_PROFILE");

	NvDRSProfileHandle GlobalProfile = 0;
	status = NvAPI_DRS_GetCurrentGlobalProfile(hSession, &GlobalProfile);
	if (status != NVAPI_OK)
		PrintError(status, L"GET_PROFILE_GLOBAL");

	//Restores the Global & Default Profile if flagged
	if (Restore)
	{
		RestoreDefaults(hSession, hProfile, GlobalProfile);
	}

	//Export (Print Current Settings)
	if (Export)
	{
		std::map<DWORD, DWORD> expmap;
		//Export Profile's Data into the Map
		ExportProfile(hSession, hProfile, expmap);
		ExportProfile(hSession, GlobalProfile, expmap);
		//Print the Profile's Data
		std::map<DWORD, DWORD>::iterator expit;
		int index = 0;
		for (expit = expmap.begin(); expit != expmap.end(); expit++)
		{
			DWORD v = expit->second;
			std::wcout << (index != 0 ? L";" : L"") << ToHex(expit->first) << L"=" << (v == SETTING_DEFAULT_VALUE ? L"Default" : ToHex(v));
			index++;
		}

		//Restore After Default If Flagged
		if (RestoreAfter)
		{
			RestoreDefaults(hSession, hProfile, GlobalProfile);
		}
	}
	else if (Query)
	{
		QueryProfile(hSession, hProfile, map);
		QueryProfile(hSession, GlobalProfile, map);

		//Restore After Default If Flagged
		if (RestoreAfter)
		{
			RestoreDefaults(hSession, hProfile, GlobalProfile);
		}
	}
	//Set Array of Setting ID to Setting Values (Uninstall For GameModeLib)
	else if (SetIds)
	{
		SetSettings(hSession, hProfile, map);
		SetSettings(hSession, GlobalProfile, map);
	}
	//Apply Preffered Graphics Processor & Other Settings to All Default Global Profiles
	else
	{
		SetSettings(hSession, hProfile);
		SetSettings(hSession, GlobalProfile);
	}

	// Save Changes
	status = NvAPI_DRS_SaveSettings(hSession);
	if (status != NVAPI_OK)
		PrintError(status, L"SAVING");
	// Cleanup
	NvAPI_DRS_DestroySession(hSession);
}