#include <Windows.h>
#include <psapi.h>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <tlhelp32.h>
#include <fstream>

#include <powrprof.h>
#include <objbase.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "GameModeLib.h"
using namespace std;

/**
 * WINDOWS LIBS USED
 *  psapi
 *  PowrProf
 *  ole32
 */
namespace GAMEMODELIB {

const unsigned long HIGH = HIGH_PRIORITY_CLASS;
const unsigned long NORMAL = NORMAL_PRIORITY_CLASS;
const unsigned long LOW = BELOW_NORMAL_PRIORITY_CLASS;
bool SetActivePP = false;
bool UGenInfo = false;
string WorkingDir;
wstring UGenDir = L"";
std::wofstream ugpu;
vector<wstring> ugpulines;

void init(string dir)
{
	CoInitialize(NULL);
	WorkingDir = dir;
	if(UGenInfo && UGenDir.empty())
		UGenDir = toWString(WorkingDir) + L"\\Uninstall";

	if(UGenInfo)
	{
		wstring ustall = UGenDir + L"\\UGpuEntry.reg";
		CreateDirectoryW(UGenDir.c_str(), NULL);
		bool printHeader = !GAMEMODELIB::isFile(ustall);
		//cache previous installation
		if(!printHeader)
		{
			std::wifstream ureg(ustall.c_str());
	        std::wstring line;
	        while (std::getline(ureg, line))
	        {
	        	line = trim(line);
	        	int i = GAMEMODELIB::IndexOf(line, L"\"");
	        	if(line != L"" && !GAMEMODELIB::startsWith(line, L";") && i >= 0)
	        	{
	        		auto prev = L' ';
	        		int index_end = -1;
	        		bool s = false;//Started
	        		for(std::size_t index = 0; index < line.length(); index++)
	        		{
	        			auto c = line[index];
	        			//ESC backslash
	        			if(c == L'\\' && prev == L'\\')
	        			{
	        				prev = L' ';
	        			}
                        if (c == L'"' && prev != L'\\')
                        {
                            if (s)
                            {
                                index_end = (int) (index - 1);
                                break;
                            }
                            s = true;
                        }
	        			prev = c;
	        		}
	        		//Handle Maulformed Lines without Erroring
	        		if(index_end == -1)
	        		{
	        			wcerr << L"Maulformed Reg Line:" << line << endl;
	        			index_end = line.length() - 1;
	        		}
	        		line = line.substr(i + 1, index_end - i);
	            	ReplaceAll(line, L"\\\\", L"\\");
	            	ReplaceAll(line, L"\\\"", L"\"");
	        		ugpulines.push_back(line);
	        	}
	        }
		}
		ugpu = std::wofstream(ustall.c_str(), std::ios::app);
		if (ugpu.is_open())
		{
			if(printHeader)
				ugpu << L"Windows Registry Editor Version 5.00" << endl << endl << L"[HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences]" << endl;
		}
	}
}

void uninit()
{
	CoUninitialize();
	ugpulines.clear();
	if(ugpu.is_open())
		ugpu.close();
}

/**
 * run a process and wait for the exit code
 */
int runProcess(const wstring& cmd)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	const wchar_t* app_const = cmd.c_str();
	// Start the child process.
	if( !CreateProcessW(NULL,   // No module name (use command line)
			const_cast<LPWSTR>(app_const),    // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory
			&si,            // Pointer to STARTUPINFO structure
			&pi )           // Pointer to PROCESS_INFORMATION structure
	)
	{
		wcerr << L"Failed to create Process:" + cmd;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = -2;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	// Close process and thread handles.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return exitCode;
}

unsigned long GetParentPID()
{
	unsigned long pid = GetCurrentProcessId();
	unsigned long ppid = -1;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(snapshot, &entry))
	{
		while (Process32Next(snapshot, &entry))
		{
			if (entry.th32ProcessID == pid)
        	{
				ppid = entry.th32ParentProcessID;
				break;
        	}
		}
	}
	CloseHandle(snapshot);
	return ppid;
}

void SetPriority(unsigned long PID, unsigned long Priority)
{
	 HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	 if (!SetPriorityClass(hProcess, Priority)) {
		 wcerr << L"Failed to set priority. Error: " << GetLastError() << endl;
	 }
	 CloseHandle(hProcess);
}

void SetHighPriority(unsigned long PID)
{
	SetPriority(PID, GAMEMODELIB::HIGH);
}

void SetHighPriority()
{
	SetHighPriority(GetCurrentProcessId());
}

/**
 * returns the full executable path of the running process
 */
string GetProcessName(unsigned long pid)
{
	string name = "";
	HANDLE phandle = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	WCHAR filename[MAX_PATH];
	GetModuleFileNameExW(phandle, NULL, filename, MAX_PATH);
	CloseHandle(phandle);
	return toString(wstring(filename));
}

bool RegExists(HKEY hKey, wstring &val)
{
	return RegQueryValueExW(hKey, val.c_str(), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
}

wstring GetRegString(HKEY hKey, const wstring& value)
{
    LONG lResult;
    DWORD dwType = REG_SZ;
    wchar_t szValue[257]; // Buffer to hold the value data
    DWORD dwSize = sizeof(szValue);

    lResult = RegQueryValueExW(hKey, value.c_str(), NULL, &dwType, reinterpret_cast<LPBYTE>(szValue), &dwSize);
    if (lResult == ERROR_SUCCESS)
    {
    	SetLastError(0);
        // Ensure null-termination of the string
        szValue[dwSize / sizeof(wchar_t)] = L'\0';
        return wstring(szValue);
    }

    // Return an empty string to indicate failure
    SetLastError(1);
    return L"";
}


void SetGPUPreference(string e, bool force)
{
	wstring exe = toWString(e);
	wstring data = L"GpuPreference=2;";
	HKEY hKey;
    LONG result = RegCreateKeyExW(
    	HKEY_CURRENT_USER,  // Hive (root key)
        L"SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences",
        0,                  // Reserved, must be zero
        NULL,                // Class (not used)
        REG_OPTION_NON_VOLATILE,  // Options
        KEY_WRITE | KEY_QUERY_VALUE,           // Desired access
        NULL,                // Security attributes
        &hKey,               // Resulting key handle
        NULL              // Disposition (not used)
    );
    if (result != ERROR_SUCCESS) {
    	wcout << L"Failed To Create Registry Directory of HKCU\\SOFTWARE\\Microsoft\\DirectX\\UserGpuPreferences" << endl;
    }
    bool exists = RegExists(hKey, exe);
    if(force || !exists)
    {
        if(UGenInfo && (std::find(ugpulines.begin(), ugpulines.end(), exe) == ugpulines.end()) )
        {
        	ugpulines.push_back(exe);//Prevent Duplication
        	wstring exereg = exe;
        	//ESC Path
        	ReplaceAll(exereg, L"\\", L"\\\\");
        	ReplaceAll(exereg, L"\"", L"\\\"");
        	if(!exists)
        	{
				ugpu << L"\"" + exereg + L"\"=-" << std::endl;
        	}
        	else
        	{
        		wstring sz = GetRegString(hKey, exe);
        		if(GetLastError() == 0)
        		{
        			//ESC Data
            		ReplaceAll(sz, L"\\", L"\\\\");
            		ReplaceAll(sz, L"\"", L"\\\"");
            		ugpu << L"\"" + exereg + L"\"=\"" + sz + L"\"" << endl;
        		}
        	}
        }

        if(RegSetValueExW(hKey, exe.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (data.length() + 1) * sizeof(wchar_t)) != ERROR_SUCCESS)
        {
        	cout << "Failed To Add GPU Entry:" << e << endl;
        }
    }
    RegCloseKey(hKey);
}

void SetGPUPreference(bool force)
{
	string s = GetProcessName(GetCurrentProcessId());
	SetGPUPreference(s, force);
}

void SetGPUPreference()
{
	SetGPUPreference(false);
}

void printGUID(GUID* guid)
{
	printf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
	  guid->Data1, guid->Data2, guid->Data3,
	  guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
	  guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	cout << endl;
}

//for some odd reason there was no simple guid to cstring std::string or even wstring
wstring toWString(GUID* guid)
{
	std::wstringstream stream;
    stream << std::setw(8) << std::setfill(L'0') << std::hex << guid->Data1 << L"-"
           << std::setw(4) << std::setfill(L'0') << std::hex << guid->Data2 << L"-"
           << std::setw(4) << std::setfill(L'0') << std::hex << guid->Data3 << L"-"
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[0])
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[1]) << L"-"
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[2])
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[3])
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[4])
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[5])
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[6])
           << std::setw(2) << std::setfill(L'0') << std::hex << static_cast<unsigned short>(guid->Data4[7]);

    return stream.str();
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

void SetPowerPlan(string guid, string name)
{
	//Gen Uninstall Information
    if(UGenInfo)
    {
    	wstring upp = UGenDir + L"\\PowerPlan.txt";
    	if(!isFile(upp))
    	{
    		 GUID active;
    		 GUID* activeref = &active;
    		 if (PowerGetActiveScheme(NULL, &activeref) == ERROR_SUCCESS)
    		 {
    			 std::wofstream ppfile(upp.c_str());
    			 if (ppfile.is_open()) {
    				 ppfile << toWString(activeref) << endl;
    			 }
    			 else {
    				 wcerr << L"Unable to Write PowerPlan.txt Error Code:" << GetLastError();
    			 }
    			 ppfile.close();
    		 }
    		 else {
    			 wcerr << L"Unable to Get Active PowerPlan" << endl;
    		 }
    	}
    }

	wstring str = toWString(guid);
	const wchar_t* GameModeLibPP = str.c_str();
    const wchar_t* StrHighPerf = L"{8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c}";
    GUID HighPerfGUID, GameModeGUID;
    CLSIDFromString(StrHighPerf, &HighPerfGUID);
    CLSIDFromString(GameModeLibPP, &GameModeGUID);
    if(!PowerPlanExists(GameModeGUID))
    {
    	cout << "Creating Power Plan:\"" << name << "\" GUID:" << guid << endl;
    	string exe = GetProcessName(GetCurrentProcessId());
    	string batch = exe.substr(0, exe.rfind('\\')) + "\\GameModePowerPlan.bat";
    	runProcess(toWString("cmd /c call \"" + batch + "\" \"" + guid.substr(1, guid.size() - 2) + "\" \"" + name + "\""));
    }
    //Set Active Power Plan if true regardless of whether or not the power plan was created
    else if(SetActivePP)
    {
        if (PowerSetActiveScheme(NULL, &GameModeGUID) != ERROR_SUCCESS) {
            cerr << "ERROR setting active scheme: " << GetLastError() << endl;
        }
    }
}

void SetPowerPlan()
{
	SetPowerPlan("{b8e6d75e-26e8-5e8f-efef-e94a209a3467}", "Game Mode");
}

/**
 * Run Process As Admin with Visible Window
 */
void RunAdmin(wstring exe, wstring params)
{
	SHELLEXECUTEINFOW shExInfo = {0};
	shExInfo.cbSize = sizeof(shExInfo);
	shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shExInfo.hwnd = 0;
	shExInfo.lpVerb = L"runas"; //Force ADMIN UAC Prompt
	shExInfo.lpFile = exe.c_str(); //EXE
	shExInfo.lpParameters = params.c_str(); //ARGS
	shExInfo.lpDirectory = 0;
	shExInfo.nShow = SW_SHOWNORMAL;
	shExInfo.hInstApp = 0;

	if (ShellExecuteExW(&shExInfo))
	{
	    WaitForSingleObject(shExInfo.hProcess, INFINITE);
	    CloseHandle(shExInfo.hProcess);
	}
}

wstring GetAbsolutePath(const wstring &path) {
	wstring copypath = trim(path);
	//handle empty strings or drives
	if(copypath == L"")
	{
		wchar_t buffer[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, buffer);
		return RemSlash(wstring(buffer));
	}
	else if((copypath.size() == 2) && (copypath.substr(1, 1) == L":"))
	{
		return copypath;
	}
    wchar_t absolutePath[MAX_PATH];

    DWORD length = GetFullPathNameW(path.c_str(), MAX_PATH, absolutePath, nullptr);

    if (length == 0) {
        return L"";
    }

    return RemSlash(wstring(absolutePath));
}

bool isFile(wstring file)
{
	DWORD attr = GetFileAttributesW(file.c_str());
	if(attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY))
	    return false;   //  not a file
	return true;
}

std::wstring toWString(const std::string& string)
{
    if (string.empty())
    {
        return L"";
    }

    const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, &string.at(0), (int)string.size(), nullptr, 0);
    if (size_needed <= 0)
    {
        throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
    }

    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &string.at(0), (int)string.size(), &result.at(0), size_needed);
    return result;
}

std::string toString(const std::wstring& wide_string)
{
    if (wide_string.empty())
    {
        return "";
    }

    const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_string.at(0), (int)wide_string.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
    }

    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wide_string.at(0), (int)wide_string.size(), &result.at(0), size_needed, nullptr, nullptr);
    return result;
}

};
