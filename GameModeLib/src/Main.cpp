#include <Windows.h>
#include <iostream>
#include "GameModeLib.h"

using namespace std;

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")

//Windows Linker Options: -O3 -static -static-libgcc -static-libstdc++ -L PowrProf -L Ole32
//Windows DEPS: Ole32, PowrProf

int main() {
	setlocale(LC_CTYPE, "");
	wstring cmdline = GetCommandLineW();
	int argv;
	LPWSTR* cargs = CommandLineToArgvW(cmdline.c_str(), &argv);
	wstring WorkingDir = GAMEMODELIB::parent(GAMEMODELIB::GetAbsolutePath(wstring(cargs[0])));

	//Scan for flags that effect other arguments
	for(int i = 1; i < argv; i++)
	{
		wstring s = cargs[i];
		wstring t = GAMEMODELIB::toupper(GAMEMODELIB::trim(s));
		if (t == L"-SETPOWERPLAN")
		{
			GAMEMODELIB::SetActivePP = true;
		}
		else if(t == L"-UGENINFO")
		{
			GAMEMODELIB::UGenInfo = true;
			GAMEMODELIB::UGenDir = cargs[i + 1];
		}
	}
	//INIT
	GAMEMODELIB::init(GAMEMODELIB::toString(WorkingDir));

	for(int i = 1; i < argv; i++)
	{
		wstring s = cargs[i];
		wstring t = GAMEMODELIB::toupper(GAMEMODELIB::trim(s));
		if(t == L"/?" || t == L"/HELP")
		{
			GAMEMODELIB::Help();
		}
		else if(t == L"-POWERPLAN")
		{
			GAMEMODELIB::SetPowerPlan();
		}
		else if(GAMEMODELIB::startsWith(t, L"-CREATEPOWERPLAN"))
		{
			wstring unparsed = GAMEMODELIB::trim(cargs[i + 1]);
			vector<wstring> argp = GAMEMODELIB::split(unparsed, ':');
			wstring guid = argp[0];
			wstring name = argp[1];
			GAMEMODELIB::SetPowerPlan(GAMEMODELIB::toString(guid), GAMEMODELIB::toString(name));
		}
		else if(GAMEMODELIB::startsWith(t, L"-SETHIGHPRIORITY"))
		{
			vector<wstring> argp = GAMEMODELIB::split(t, ':');
			unsigned long PID = argp.size() < 2 ? GAMEMODELIB::GetParentPID() : GAMEMODELIB::ParseUnsignedLong(argp[1]);
			GAMEMODELIB::SetHighPriority(PID);
		}
		else if(GAMEMODELIB::startsWith(t, L"-SETPRIORITY"))
		{
			vector<wstring> secs = GAMEMODELIB::split(t, ':');
			wstring p = secs[1].substr(0, 1);
			unsigned long PRIORITY;
			if(p == L"H") {
				PRIORITY = GAMEMODELIB::HIGH;
			}
			else if(p == L"N") {
				PRIORITY = GAMEMODELIB::NORMAL;
			}
			else if(p == L"L") {
				PRIORITY = GAMEMODELIB::LOW;
			}
			else {
				cerr << "Maulformed -SetPriority" << endl;
				exit(-1);
			}
			unsigned long PID = secs.size() > 2 ? GAMEMODELIB::ParseUnsignedLong(secs[2]) : GAMEMODELIB::GetParentPID();
			GAMEMODELIB::SetPriority(PID, PRIORITY);
		}
		else if(GAMEMODELIB::startsWith(t, L"-GPUENTRYCURRENT"))
		{
			vector<wstring> unp = GAMEMODELIB::split(t, ':');
			bool force = unp.size() > 1 ? GAMEMODELIB::parseBool(unp[1]) : false;
			GAMEMODELIB::SetGPUPreference(GAMEMODELIB::GetProcessName(GAMEMODELIB::GetParentPID()), force);
		}
		else if(GAMEMODELIB::startsWith(t, L"-GPUENTRY"))
		{
			wstring unparsed = GAMEMODELIB::trim(cargs[i + 1]);
			if(GAMEMODELIB::startsWith(unparsed, L"-"))
			{
				cerr << "Maulformed Argument -GPUENTRY" << endl;
				exit(-1);
			}
			vector<wstring> exeargs = GAMEMODELIB::split(unparsed, ';');
			for(wstring unparsedexe : exeargs)
			{
				vector<wstring> exeentry = GAMEMODELIB::split(unparsedexe, '|');
				wstring exe = exeentry[0];
				bool force = exeentry.size() > 1 ? GAMEMODELIB::parseBool(exeentry[1]) : false;
				if(exe.substr(0, 1) != L":")
				{
					wstring envpath = _wgetenv(L"PATH");
					vector<wstring> paths = GAMEMODELIB::split(envpath, ';');
					for(wstring p : paths)
					{
						p = GAMEMODELIB::RemSlash(p);
						if(p == L"")
							continue;
						GAMEMODELIB::ReplaceAll(p, L"/", L"\\");
						GAMEMODELIB::ReplaceAll(p, L"\\\\", L"\\");
						wstring actual_exe = p + L"\\" + exe;
						if(GAMEMODELIB::isFile(actual_exe)) {
							GAMEMODELIB::SetGPUPreference(GAMEMODELIB::toString(actual_exe), force);
						}
					}
					wstring abexe = GAMEMODELIB::GetAbsolutePath(exe);
					if(GAMEMODELIB::isFile(abexe)) {
						GAMEMODELIB::SetGPUPreference(GAMEMODELIB::toString(abexe), force);
					}
				}
				else {
					GAMEMODELIB::SetGPUPreference(GAMEMODELIB::toString(exe), force);
				}
			}
		}
	}
	GAMEMODELIB::uninit();
}
