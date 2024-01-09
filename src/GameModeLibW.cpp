#include <Windows.h>
#include <iostream>
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

};
