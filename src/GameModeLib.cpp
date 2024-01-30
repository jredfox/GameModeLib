#include <iostream>
#include <vector>
#include <string>
#include "GameModeLib.h"
using namespace std;

namespace GAMEMODELIB {

wstring toupper(wstring s)
{
	for(auto& c : s)
		c = ::toupper(c);
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

unsigned long ParseUnsignedLong(wstring str)
{
	int base = GAMEMODELIB::startsWith(str, L"0X") ? 16 : 10;
	return stoul(GAMEMODELIB::toString(str), NULL, base);
}

std::string toString(bool b)
{
	return b ? "true" : "false";
}

int revIndexOf(wstring str, wstring key)
{
	size_t found = str.rfind(key);
	if(found != std::string::npos)
		 return static_cast<int>(found);
	return -1;
}

wstring parent(wstring path)
{
	int index = revIndexOf(path, L"\\");
	return path.substr(0, index);
}

wstring RemSlash(wstring str)
{
	if(str.size() > 2 && (EndsWith(str, L"\\") || EndsWith(str, L"/")) )
	{
		str = str.substr(0, str.length() - 1);
	}
	return str;
}

wstring ReplaceAll(wstring& str, const wstring& from, const wstring& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

bool EndsWith (const std::wstring &fullString, const std::wstring &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void Help()
{
	cout << "GameModeLib {Options}" << endl;
	cout << "-SetHighPriority:{Optional PID}" << endl;
	cout << "-SetPriority:{High, Normal, Low}:{Optional PID}" << endl;
	cout << "-GPUEntry {EXE|true;EXE2}" << endl;
	cout << "-GPUEntryCurrent:{Bool Force}" << endl;
	cout << "-UGenInfo" << endl;
	cout << "-PowerPlan" << endl;
	cout << "-CreatePowerPlan {GUID:NAME}" << endl;
	cout << "-SetPowerPlan" << endl;
	cout << endl;
	cout << "Example: GameModeLib -SetHighPriority -PowerPlan" << endl;
	cout << "Example: GameModeLib -CreatePowerPlan \"a8bb3a1c-6456-5ec6-8070-566dc60e4022:Epic Power Plan\"" << endl;
	cout << "Example: GameModeLib -SetPriority:LOW -GPUEntry \"C:\\Program Files\\Java\\jre-1.8\\bin\\java.exe\\;C:\\Program Files\\Java\\jre-1.8\\bin\\javaw.exe|True\"" << endl;
	cout << "Example: GameModeLib -SetPriority:NORMAL:2412" << endl;
	exit(0);
}

};



