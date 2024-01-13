#include <Windows.h>
#include <iostream>
#include "GameModeLib.h"
#include <rpc.h>
using namespace std;

int main() {
	setlocale(LC_CTYPE, "");
	GAMEMODELIB::init();
	GAMEMODELIB::SetHighPriority();
	GAMEMODELIB::AddGPUPreference(true);
//	GAMEMODELIB::AddPowerPlan();
	GAMEMODELIB::uninit();
}
