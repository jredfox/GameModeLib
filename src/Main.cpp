#include <iostream>
#include <Windows.h>
#include "GameModeLib.h"
using namespace std;

STICKYKEYS g_StartupStickyKeys = {sizeof(STICKYKEYS), 0};
TOGGLEKEYS g_StartupToggleKeys = {sizeof(TOGGLEKEYS), 0};
FILTERKEYS g_StartupFilterKeys = {sizeof(FILTERKEYS), 0};

int main() {
	GAMEMODELIB::SetHighPriority();
	return 0;
}
