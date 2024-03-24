//NVIDIA3D Settings
//3d Settings --> Preffered Graphics Processor --> High Performance
//3d Settings --> Preffered OpenGL Graphics Processor --> High Performance?
//3d Settings --> Preffered D3D Graphics Processor --> High Performance?
//3d Settings --> VSYNC --> Application Settings
//3d Settings --> PowerMode --> Optimal Power

//TODO: 3D Settings Profile, Global Profile, All Settings Per Profile
#include "Windows.h"
#include "nvapi.h"
#include "NvApiDriverSettings.h"
#include <stdlib.h>
#include <stdio.h>

#include <iostream>

#pragma comment (lib, "nvapi.lib")

/*
 This function is used to print to the command line a text message
 describing the nvapi error and quits
*/
void PrintError(NvAPI_Status status)
{
	NvAPI_ShortString szDesc = { 0 };
	NvAPI_GetErrorMessage(status, szDesc);
	printf(" NVAPI error: %s\n", szDesc);
	exit(-1);
}

bool DisplayProfileContents(NvDRSSessionHandle hSession,
	NvDRSProfileHandle hProfile)
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
		PrintError(status);
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
			PrintError(status);
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
			PrintError(status);
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
				unsigned int len;
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

#include <iomanip>
#include <fcntl.h>
#include <io.h>

int main(int argc, char **argv)
{
	setlocale(LC_CTYPE, "");
	// (0) Initialize NVAPI. This must be done first of all
	NvAPI_Status status = NvAPI_Initialize();
	if (status != NVAPI_OK)
		PrintError(status);
	// (1) Create the session handle to access driver settings
	NvDRSSessionHandle hSession = 0;
	status = NvAPI_DRS_CreateSession(&hSession);
	if (status != NVAPI_OK)
		PrintError(status);
	// (2) load all the system settings into the session
	status = NvAPI_DRS_LoadSettings(hSession);
	if (status != NVAPI_OK)
		PrintError(status);
	// (3) Obtain the Base profile. Any setting needs to be inside
	// a profile, putting a setting on the Base Profile enforces it
	// for all the processes on the system
	NvDRSProfileHandle hProfile = 0;
	status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
	if (status != NVAPI_OK)		PrintError(status);

	//Print Profile
	//DisplayProfileContents(hSession, hProfile);

	//Set V-SYNC to Default Value
	NVDRS_SETTING drsSetting = { 0 };
	drsSetting.version = NVDRS_SETTING_VER;
	drsSetting.settingId = VSYNCMODE_ID;
	drsSetting.settingType = NVDRS_DWORD_TYPE;
	drsSetting.u32CurrentValue = VSYNCMODE_PASSIVE;
	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting);
	if (status != NVAPI_OK)
		PrintError(status);

	//Set PowerMode to PREFERRED_PSTATE_OPTIMAL_POWER
	drsSetting = { 0 };
	drsSetting.version = NVDRS_SETTING_VER;
	drsSetting.settingId = PREFERRED_PSTATE_ID;
	drsSetting.settingType = NVDRS_DWORD_TYPE;
	drsSetting.u32CurrentValue = PREFERRED_PSTATE_OPTIMAL_POWER;
	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting);
	if (status != NVAPI_OK)
		PrintError(status);

	//Set Preffered OpenGL Graphics Processor to 2 Is this High Performance?
	drsSetting = { 0 };
	drsSetting.version = NVDRS_SETTING_VER;
	drsSetting.settingId = OGL_IMPLICIT_GPU_AFFINITY_ID;
	drsSetting.settingType = NVDRS_DWORD_TYPE;
	drsSetting.u32CurrentValue = 2;
	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting);
	if (status != NVAPI_OK)
		PrintError(status);

	//Set Preffered Graphics Processor to High Performance
	bool ForceIntegrated = false;
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

	if (ForceIntegrated) {
		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_INTEGRATED;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_INTEGRATED;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE;
	}
	else {
		drsSetting1.u32CurrentValue = SHIM_MCCOMPAT_ENABLE;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting2.u32CurrentValue = SHIM_RENDERING_MODE_ENABLE;//0 for Integrated 1 HIGH PERFORMANCE and 10 For Auto
		drsSetting3.u32CurrentValue = SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE;
	}

	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting1);
	if (status != NVAPI_OK)
		PrintError(status);

	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting2);
	if (status != NVAPI_OK)
		PrintError(status);

	status = NvAPI_DRS_SetSetting(hSession, hProfile, &drsSetting3);
	if (status != NVAPI_OK)
		PrintError(status);

	//Setting IDs: OGL_IMPLICIT_GPU_AFFINITY_ID 
	//10F9DC80

	// Save Changes
	status = NvAPI_DRS_SaveSettings(hSession);
	if (status != NVAPI_OK)
		PrintError(status);
	// Cleanup
	NvAPI_DRS_DestroySession(hSession);
}