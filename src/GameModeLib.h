#ifndef GAMEMODELIB_H_
#define GAMEMODELIB_H_
#include <vector>
namespace GAMEMODELIB {

//DECLARE VARIABLES HERE
extern const unsigned long HIGH;
extern const unsigned long NORMAL;
extern const unsigned long LOW;
extern bool SetActivePP;

//DECLARE METHODS HERE
void SetPriority(unsigned long PID, unsigned long Priority);
void SetHighPriority(unsigned long PID);
void SetHighPriority();
void SetGPUPreference(std::string e, bool force);//Adds a specific EXE to the Dedicated GPU Entries with force being an option to always overwrite it
void SetGPUPreference(bool force);//Adds current EXE to Dedicated GPU Entries if it doesn't exist or forcibly if force is true
void SetGPUPreference();//Adds current EXE forcibly to Dedicated GPU Entries always
void SetPowerPlan();
void SetPowerPlan(std::string guid, std::string name);//Adds Game Mode Power Plan with a specific GUID creating it if it doesn't exist
std::string GetProcessName(unsigned long PID);
void init();
void uninit();
void Help();//the help command
//START UTILITY METHODS
std::wstring toupper(std::wstring s);
std::wstring trim(std::wstring str);
int IndexOf(std::wstring str, std::wstring key);
bool startsWith(std::wstring str, std::wstring key);
std::vector<std::wstring> split(const std::wstring& input, wchar_t c);
std::string toString(const std::wstring& wide_string);
bool parseBool(std::wstring str);
unsigned long GetParentPID();

//DEFINE IMPL DEFINITIONS HERE
#ifdef COMPILE_WINDOWS
#include "GameModeLibW.cpp"
#elif defined(COMPILE_MACOS)
#include "GameModeLibM.cpp"
#elif defined(COMPILE_LINUX)
#include "GameModeLibL.cpp"
#endif
};
#endif
