#ifndef GAMEMODELIB_H_
#define GAMEMODELIB_H_
namespace GAMEMODELIB {
//DEFINE METHODS HERE
void SetPriority(unsigned long PID, unsigned long Priority);
void SetHighPriority();
void AddGPUPreference(std::string e, bool force);//Adds a specific EXE to the Dedicated GPU Entries with force being an option to always overwrite it
void AddGPUPreference(bool force);//Adds current EXE to Dedicated GPU Entries if it doesn't exist or forcibly if force is true
void AddGPUPreference();//Adds current EXE forcibly to Dedicated GPU Entries always
void AddPowerPlan();
void AddPowerPlan(std::string guid, std::string name);//Adds Game Mode Power Plan with a specific GUID creating it if it doesn't exist
void init();
void uninit();

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
