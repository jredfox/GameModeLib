#include <Windows.h>
#include <iostream>
#include <psapi.h>
using namespace std;

namespace GAMEMODELIB {

const DWORD HIGH = HIGH_PRIORITY_CLASS;
const DWORD NORMAL = NORMAL_PRIORITY_CLASS;
const DWORD LOW = BELOW_NORMAL_PRIORITY_CLASS;

void init()
{
	CoInitialize(NULL);
}

void uninit()
{
	CoUninitialize();
}

void SetPriority(unsigned long PID, unsigned long Priority)
{
	 HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	 if (!SetPriorityClass(hProcess, Priority)) {
		 wcerr << L"Failed to set priority. Error: " << GetLastError() << endl;
	 }
	 CloseHandle(hProcess);
}

void SetHighPriority()
{
	SetPriority(GetCurrentProcessId(), GAMEMODELIB::HIGH);
}

/**
 * returns the full executable path of the running process
 */
string getProcessName(unsigned long pid)
{
	string name = "";
	HANDLE phandle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	TCHAR filename[MAX_PATH];
	GetModuleFileNameEx(phandle, NULL, filename, MAX_PATH);
	CloseHandle(phandle);
	return string(filename);
}

bool RegExists(HKEY hKey, string &val)
{
	return RegQueryValueExA(hKey, val.c_str(), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
}

void AddGPUPreference(string e, bool force)
{
	string data = "GpuPreference=2;";
	HKEY hKey;
    LONG result = RegCreateKeyExA(
    	HKEY_CURRENT_USER,  // Hive (root key)
        "SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences",
        0,                   // Reserved, must be zero
        NULL,                // Class (not used)
        REG_OPTION_NON_VOLATILE,  // Options
        KEY_WRITE | KEY_WOW64_64KEY | KEY_QUERY_VALUE,           // Desired access
        NULL,                // Security attributes
        &hKey,               // Resulting key handle
        NULL              // Disposition (not used)
    );
    if (result != ERROR_SUCCESS) {
    	wcout << L"Failed To Create Registry Directory of HKCU\\SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences" << endl;
    }
    if((force || !RegExists(hKey, e)) && RegSetValueExA(hKey, e.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), data.length() + 1) != ERROR_SUCCESS)
    {
    	cout << "Failed To Add GPU Entry:" << e << endl;
    }
    RegCloseKey(hKey);
}

void AddGPUPreference(bool force)
{
	string s = getProcessName(GetCurrentProcessId());
	AddGPUPreference(s, force);
}

void AddGPUPreference()
{
	AddGPUPreference(true);
}

//Power Plan Start
#include <powrprof.h>
#include <objbase.h>
void AddPowerPlan()
{
	//Generated from string {GAMEMODELIB'SPP}
    const wchar_t* GameModeLibPP = L"{b8e6d75e-26e8-5e8f-efef-e94a209a3467}";
    const wchar_t* StrHighPerf = L"{8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c}";
    GUID gmlpp;
    GUID hppp;

    // Convert the string to a GUID
    HRESULT hr = CLSIDFromString(GameModeLibPP, &gmlpp);
    HRESULT hr2 = CLSIDFromString(StrHighPerf, &hppp);
    if(hr != NOERROR || hr2 != NOERROR) {
    	cout << "ERR GUID" << endl;
    	return;
    }
    GUID* GUID1 = &gmlpp;
//	if(PowerDuplicateScheme(0, HighPerfGUID, &GameModeGUID) != ERROR_SUCCESS)
//	{
//		cerr << "ERROR " << GetLastError() << endl;
//	}
}

};
