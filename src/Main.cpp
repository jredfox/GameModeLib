#include <iostream>
#include <Windows.h>
#include "GameModeLib.h"
using namespace std;

int main() {
	GAMEMODELIB::SetHighPriority();
	return 0;
}
