#include <Windows.h>
#include <iostream>
#include "GameModeLib.h"
using namespace std;

//TODO:
/**
 * -SetPriority:<PRIORITY>:<Optional PID>
 * -CreatePowerPlan
 */

int main() {
	setlocale(LC_CTYPE, "");
	wstring cmdline = GetCommandLineW();
	int argv;
	LPWSTR* cargs = CommandLineToArgvW(cmdline.c_str(), &argv);
	GAMEMODELIB::init();
	//Scan for flags that effect other arguments
	for(int i = 1; i < argv; i++)
	{
		wstring s = cargs[i];
		wstring t = GAMEMODELIB::toupper(GAMEMODELIB::trim(s));
		if (t == L"-SETPOWERPLAN")
		{
			GAMEMODELIB::SetActivePP = true;
		}
	}
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
		else if(GAMEMODELIB::startsWith(t, L"-SETHIGHPRIORITY"))
		{
			vector<wstring> argp = GAMEMODELIB::split(t, ':');
			if(argp.size() < 2)
			{
				GAMEMODELIB::SetHighPriority(GAMEMODELIB::GetParentPID());
			}
			else
			{
				wstring strpid = argp[1];
				int base = GAMEMODELIB::startsWith(strpid, L"0X") ? 16 : 10;
				unsigned long PID = stoul(GAMEMODELIB::toString(strpid), NULL, base);
				GAMEMODELIB::SetHighPriority(PID);
			}
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
				GAMEMODELIB::SetGPUPreference(GAMEMODELIB::toString(exe), force);
			}
		}
	}
	GAMEMODELIB::uninit();
}
