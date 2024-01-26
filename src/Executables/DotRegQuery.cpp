//============================================================================
// Name        : HelloWorld.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <shlobj.h>
using namespace std;

int IndexOf(wstring str, wstring key)
{
	size_t found = str.find(key);
	if(found != std::string::npos)
		 return static_cast<int>(found);
	return -1;
}

bool startsWith(std::wstring str, std::wstring key)
{
	return (IndexOf(str, key) == 0);
}

wstring ReplaceAll(wstring& str, const wstring& from, const wstring& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}


wstring trim(wstring str)
{
    str.erase(str.find_last_not_of(' ')+1);         //suffixing spaces
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    return str;
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
			TRUE,          // Set handle inheritance to FALSE
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

#include <chrono>
#include <cstdint>

uint64_t GetMS() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

bool RegExists(HKEY hKey, wstring &val)
{
	return RegQueryValueExW(hKey, val.c_str(), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
}

HKEY GetTree(wstring branch)
{
	if(startsWith(branch, L"HKLM") || startsWith(branch, L"HKEY_LOCAL_MACHINE"))
		return HKEY_LOCAL_MACHINE;
	else if(startsWith(branch, L"HKCU") || startsWith(branch, L"HKEY_CURRENT_USER"))
		return HKEY_CURRENT_USER;
	else if(startsWith(branch, L"HKCR") || startsWith(branch, L"HKEY_CLASSES_ROOT"))
		return HKEY_CLASSES_ROOT;
	else if(startsWith(branch, L"HKU") || startsWith(branch, L"HKEY_USERS"))
		return HKEY_USERS;
	else if(startsWith(branch, L"HKCC") || startsWith(branch, L"HKEY_CURRENT_CONFIG"))
		return HKEY_CURRENT_CONFIG;
	return NULL;
}

std::wstring EscapeString(const wchar_t* input) {
    std::wstring escapedString;
    while (*input) {
        if (*input == L'"') {
            escapedString += L"\\\"";
        } else if (*input == L'\\') {
            escapedString += L"\\\\";
        } else {
            escapedString += *input;
        }
        ++input;
    }
    return escapedString;
}

int main() {
	setlocale(LC_CTYPE, "");
	wstring cmdline = GetCommandLineW();
	int argv;
	LPWSTR* cargs = CommandLineToArgvW(cmdline.c_str(), &argv);
	wstring file = cargs[1];
	wstring udir = cargs[2];
	SHCreateDirectoryExW(NULL, udir.c_str(), NULL);
	std::wifstream srchfile(file.c_str());
    if (srchfile.is_open())
    {
    	wcout << L"Windows Registry Editor Version 5.00" << endl;
        std::wstring line;
        wstring key = L"";
        wstring subkey = L"";
        HKEY tree;
        HKEY hsubkey;
        bool canQuery = true;
        while (std::getline(srchfile, line))
        {
        	line = trim(line);
        	if(line != L"" && !startsWith(line, L";"))
        	{
        		if(line.front() == L'[')
        		{
        			RegCloseKey(hsubkey);
        			bool delkey = (line.at(1) == L'-');
        			int start = (delkey ? 2 : 1);
        			key = line.substr(start, line.length() - start - 1);
        			tree = GetTree(key);
        			subkey = key.substr(IndexOf(key, L"\\") + 1);
        			if(delkey)
        			{
        				canQuery = false;
        				if(RegOpenKeyExW(tree, subkey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hsubkey) == ERROR_SUCCESS)
        				{
        					wstring keyname = key;
        					wstring ureg = udir + L"\\" + ReplaceAll(keyname, L"\\", L"_") + L".reg";
        					runProcess(L"cmd /c reg export \"" + key + L"\" \"" + ureg + L"\" /y >nul 2>&1");
        				}
        			}
        			else
        			{
    					if(RegOpenKeyExW(tree, subkey.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hsubkey) != ERROR_SUCCESS)
    					{
    						wcout << endl << L"[-" + key + L"]" << endl;
    						canQuery = false;
    					}
    					else
    					{
    						wcout << endl << L"[" << key << L"]" << endl;
                			canQuery = true;
    					}
        			}
        		}
        		else if(canQuery)
        		{
        			int index = IndexOf(line, L"=");
        			if(index < 0)
        				continue;
        			wstring valname = line.substr(0, index);
        			if(startsWith(valname, L"\""))
        				valname = valname.substr(1, valname.length() - 2);
        			DWORD type, size;
        			LONG result = RegQueryValueExW(hsubkey, valname.c_str(), NULL, &type, NULL, &size);
        			if(result == ERROR_SUCCESS)
        			{
        				BYTE* data = new BYTE[size + 1];
        				result = RegQueryValueExW(hsubkey, valname.c_str(), NULL, &type, data, &size);
        				switch (type)
        				{
        					case REG_SZ: {
        						wcout << (L"\"" + valname + L"\"=\"") << EscapeString(reinterpret_cast<const wchar_t*>(data)) << L"\"" << endl;
        					}
        					break;
        					case REG_DWORD: {
        						DWORD dwordValue = *reinterpret_cast<DWORD*>(data);
        						wcout << (L"\"" + valname + L"\"=dword:") << std::setfill(L'0') << std::setw(8) << std::hex << dwordValue << endl;
        						break;
        					}
        					default:
        						wcerr << L"Unsupported REG TYPE:" << type << endl;
        					break;
        				}
        				delete[] data;
        			}
        			//Handle Missing Values
        			else if(result == ERROR_FILE_NOT_FOUND)
        			{
        				wcout << L"\"" + valname + L"\"=-" << endl;
        			}
        			else
        			{
        				wcerr << L"ERR Querying Value:" << result << L" REG:" << (key + L" " + valname) << endl;
        			}
        		}
        	}
        }
        RegCloseKey(hsubkey);
    }
    else
    {
    	wcerr << L"Err Loading REG FILE: " << GetLastError() << std::endl;
    }
    srchfile.close();
}
