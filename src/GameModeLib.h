#ifndef GAMEMODELIB_H_
#define GAMEMODELIB_H_
#include <vector>
namespace GAMEMODELIB {

//DECLARE VARIABLES HERE 
extern const unsigned long HIGH;
extern const unsigned long NORMAL;
extern const unsigned long LOW;
extern bool SetActivePP;
extern bool UGenInfo;
extern std::string WorkingDir;

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
unsigned long GetParentPID();
void init(std::string dir);
void uninit();
void Help();//the help command
/**
 * Install GameModeLib Requires ADMIN or SUDO Rights
 */
void Install(bool DisableDriveEncryption);
/**
 * UnInstall Revert any changes made by the Install Script
 */
void UnInstall();
//START UTILITY METHODS
std::wstring toupper(std::wstring s);
std::wstring trim(std::wstring str);
int IndexOf(std::wstring str, std::wstring key);
bool startsWith(std::wstring str, std::wstring key);
std::vector<std::wstring> split(const std::wstring& input, wchar_t c);
std::string toString(const std::wstring& wide_string);
std::wstring toWString(const std::string& string);
std::string toString(bool b);
bool parseBool(std::wstring str);
unsigned long ParseUnsignedLong(std::wstring str);
std::wstring RemSlash(std::wstring str);
std::wstring parent(std::wstring path);
int revIndexOf(std::wstring str, std::wstring key);
bool EndsWith (const std::wstring &fullString, const std::wstring &ending);
bool isFile(std::wstring file);
std::wstring GetAbsolutePath(const std::wstring &path);
std::wstring ReplaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to);

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
