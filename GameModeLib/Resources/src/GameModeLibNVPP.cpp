#include <Windows.h>
#include <PowrProf.h>
#include <iostream>

#pragma comment(lib, "PowrProf.lib")

using namespace std;

GUID ULTIMATE = { 0xE9A42B02, 0xD5DF, 0x448D, {0xAA, 0x00, 0x03, 0xF1,0x47, 0x49, 0xEB, 0x61 } };
GUID SUB_GRAPHICS = { 0x4f971e89, 0xeebd, 0x4455,{ 0xa8, 0xde, 0x9e, 0x59, 0x04, 0x0b, 0xf5, 0x63 } };
GUID SETTING_GRAPHICS = { 0x5fb4938d, 0x1ee8, 0x4b0f,{ 0x9a, 0x3c, 0x50, 0x3b, 0x37, 0x9f, 0x4c, 0x89 } };
GUID SETTING_GRAPHICS_PWR = { 0x5fb4938d, 0x1ee8, 0x4b0f,{ 0x9a, 0x3c, 0x50, 0x3b, 0x68, 0x9f, 0x4c, 0x69 } };

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

void PrintGUID(GUID* guid)
{
	printf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	cout << endl;
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

void CreateSettings()
{
	wcout << L"Creating GameMode Lib Graphics Settings" << endl;
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
}

bool HasACValue(GUID* ACTIVE, GUID* SUB, GUID* SETTING)
{
	LPBYTE ValBytes[sizeof(DWORD) + 1];
	ULONG t = REG_DWORD;
	DWORD size = sizeof(ValBytes);
	int ERR = PowerReadACValue(NULL, ACTIVE, SUB, SETTING, &t, reinterpret_cast<LPBYTE>(&ValBytes), &size);
	//cout << ERR << endl;
	return ERR == ERROR_SUCCESS;
}

int main() {
	GUID* ActiveGUID;
	if (PowerGetActiveScheme(NULL, &ActiveGUID) != ERROR_SUCCESS)
	{
		std::cerr << "Failed to get active power scheme." << std::endl;
		return -1;
	}

	if (!HasACValue(ActiveGUID, &SUB_GRAPHICS, &SETTING_GRAPHICS))
	{
		CreateSettings();
	}
}
