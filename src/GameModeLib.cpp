#include <iostream>
#include <vector>
#include "GameModeLib.h"
using namespace std;

namespace GAMEMODELIB {

wstring toupper(wstring s)
{
	for(auto& c : s)
		c = std::toupper(c);
	return s;
}

wstring trim(wstring str)
{
    str.erase(str.find_last_not_of(' ')+1);         //suffixing spaces
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    return str;
}

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

vector<std::wstring> split(const std::wstring& input, wchar_t c) {
	vector<std::wstring> arr;
    size_t startPos = 0;
    size_t foundPos = input.find(c, startPos);
    while (foundPos != std::wstring::npos)
    {
        std::wstring sub = input.substr(startPos, foundPos - startPos);
        arr.push_back(sub);
        startPos = foundPos + 1;
        foundPos = input.find(c, startPos);
    }
    std::wstring lastSub = input.substr(startPos);
    arr.push_back(lastSub);
    return arr;
}

bool parseBool(std::wstring str)
{
	return toupper(str) == L"TRUE";
}

void Help()
{
	cout << "GameModeLib {Options}" << endl;
	cout << "-SetHighPriority:{Optional PID}" << endl;
	cout << "-SetPriority:{High, Normal, Low}:{Optional PID}" << endl;
	cout << "-GPUEntry {EXE|true;EXE2}" << endl;
	cout << "-GPUEntryCurrent:{Bool Force}" << endl;
	cout << "-PowerPlan" << endl;
	cout << "-CreatePowerPlan {GUID:NAME}" << endl << endl;
	cout << "-SetPowerPlan" << endl;
	cout << "Example: GameModeLib -SetHighPriority -PowerPlan" << endl;
	cout << "Example: GameModeLib -CreatePowerPlan \"a8bb3a1c-6456-5ec6-8070-566dc60e4022:Epic Power Plan\"" << endl;
	cout << "Example: GameModeLib -SetPriority:LOW -GPUEntry \"C:\\Program Files\\Java\\jre-1.8\\bin\\java.exe\\;C:\\Program Files\\Java\\jre-1.8\\bin\\javaw.exe|True\"" << endl;
	cout << "Example: GameModeLib -SetPriority:NORMAL:2412" << endl;
	exit(0);
}

};



