#include <Windows.h>
#include <iostream>
#include "GameModeLib.h"
using namespace std;

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
				GAMEMODELIB::SetGPUPreference(GAMEMODELIB::toString(exe), force);
			}
		}
	}
	GAMEMODELIB::uninit();
}
