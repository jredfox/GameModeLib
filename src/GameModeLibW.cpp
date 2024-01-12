#include <Windows.h>
#include <iostream>
#include <psapi.h>
using namespace std;

namespace GAMEMODELIB {

const DWORD HIGH = HIGH_PRIORITY_CLASS;
const DWORD NORMAL = NORMAL_PRIORITY_CLASS;
const DWORD LOW = BELOW_NORMAL_PRIORITY_CLASS;

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

void AddGPUPreference(string e)
{
	string data = "GpuPreference=2;";
	HKEY hKey;
    LONG result = RegCreateKeyExW(
    	HKEY_CURRENT_USER,  // Hive (root key)
        L"SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences",
        0,                   // Reserved, must be zero
        NULL,                // Class (not used)
        REG_OPTION_NON_VOLATILE,  // Options
        KEY_WRITE | KEY_WOW64_64KEY,           // Desired access
        NULL,                // Security attributes
        &hKey,               // Resulting key handle
        NULL              // Disposition (not used)
    );
    if (result != ERROR_SUCCESS) {
    	wcout << L"Failed To Create Registry Directory of HKCU\\SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences" << endl;
    }
    if(RegSetValueExA(hKey, e.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), data.length() + 1) != ERROR_SUCCESS)
    {
    	cout << "Failed To Add GPU Entry:" << e << endl;
    }
    RegCloseKey(hKey);
}

void AddGPUPreference()
{
	string s = getProcessName(GetCurrentProcessId());
	AddGPUPreference(s);
}

};
