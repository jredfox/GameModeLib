#ifndef GAMEMODELIB_H_
#define GAMEMODELIB_H_
namespace GAMEMODELIB {
//DEFINE METHODS HERE
void SetPriority(unsigned long PID, unsigned long Priority);
void SetHighPriority();

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
