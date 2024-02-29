@ECHO OFF
setlocal enableDelayedExpansion

REM ## Set Vars ##
set SIDS=%~1
set Settings=%~2
set rc=%~dp0Resources
set ugen=%~dp0Uninstall\Global

IF /I "%Settings:~0,1%" NEQ "T" (GOTO INSTALL)
set regs=!regs!^;Main.reg
:INSTALL

IF /I "%Settings:~1,1%" NEQ "T" (GOTO GRAPHICS)
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Intel.reg)
::AMD 3D Graphics Settings
start /MIN cmd /c call "!rc!\AMD3dSettings.exe" "!ugen!"
:GRAPHICS

REM ## Disable Sticky Keys ##
IF /I "%Settings:~2,1%" NEQ "T" (GOTO STKYKYS)
set regs=!regs!^;StickyKeys.reg
:STKYKYS

REM ## TouchPad Disable Palmcheck ##
IF /I "%Settings:~3,1%" NEQ "T" (GOTO TOUCHPAD)
set regs=!regs!^;TouchPad.reg
REM ## Elantech ##
reg query "HKCU\SOFTWARE\Elantech" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;ElanTech.reg)
REM ## Synaptics ##
set syntp=%~dp0TMP\Install\SynapticsUser.reg
call "!rc!\GenSynaptics.exe" "!syntp!"
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Synaptics.reg^;!syntp!)
:TOUCHPAD

REM ## Disable Full Screen Optimizations ##
IF /I "%Settings:~4,1%" NEQ "T" (GOTO DISFSO)
set regs=!regs!^;DisableFSO.reg
:DISFSO

REM ## Disable Full Screen Optimizations ##
IF /I "%Settings:~5,1%" NEQ "T" (GOTO PPThrottling)
set regs=!regs!^;PowerThrottling.reg
:PPThrottling

echo !regs!