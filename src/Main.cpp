#include <Windows.h>
#include <iostream>
#include "GameModeLib.h"
using namespace std;

int main() {
	setlocale(LC_CTYPE, "");
	GAMEMODELIB::SetHighPriority();
	GAMEMODELIB::AddGPUPreference();
	return 0;
}
