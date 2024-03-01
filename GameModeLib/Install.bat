@ECHO OFF
setlocal enableDelayedExpansion

REM #################### Usage #############################################################################################
REM ##  Modules are T/F and are Indexed From 0-8 and is the First Argument example "TTTTTTTTT"                            ##
REM ##  Main, Graphics, Sticky Keys, TouchPad, Full Screen Optimizations, PowerThrottling, BitLocker, WDLowCPU, WDDisable ##
REM ##  SIDs or Usernames or * for all Users is the Second Argument Examples Below                                        ##
REM ##  "*" "jredfox;jgrayfox" "S-1-5-21-368394509-689051271-14200874-1012;S-1-5-21-368394509-689051271-14200874-1013"    ##
REM ########################################################################################################################

REM ## Set Vars ##
echo !TIME!
set Settings=%~1
set SIDS=%~2
set rc=%~dp0Resources
set ustall=%~dp0Uninstall
set ugen=!ustall!\Global
REM ## Enable Registry Access ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1

REM ## Main Registry Settings ##
IF /I "%Settings:~0,1%" NEQ "T" (GOTO RMAIN)
set regs=!regs!^;Main.reg
:RMAIN

REM ## Graphics 3D Settings Registry ##
IF /I "%Settings:~1,1%" NEQ "T" (GOTO RGRAPHICS)
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Intel.reg)
:RGRAPHICS

REM ## Disable Sticky Keys Via The Registry ##
IF /I "%Settings:~2,1%" NEQ "T" (GOTO RSTKYKYS)
set regs=!regs!^;StickyKeys.reg
:RSTKYKYS

REM ## TouchPad Disable Palmcheck ##
IF /I "%Settings:~3,1%" NEQ "T" (GOTO RTOUCHPAD)
set regs=!regs!^;TouchPad.reg
REM ## Elantech ##
reg query "HKCU\SOFTWARE\Elantech" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;ElanTech.reg)
REM ## Synaptics ##
set syntp=%~dp0TMP\Install\SynapticsUser.reg
call "!rc!\GenSynaptics.exe" "!syntp!"
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Synaptics.reg^;!syntp!)
:RTOUCHPAD

REM ## Disable Full Screen Optimizations ##
IF /I "%Settings:~4,1%" NEQ "T" (GOTO RDISFSO)
set regs=!regs!^;DisableFSO.reg
:RDISFSO

REM ## Disable Full Screen Optimizations ##
IF /I "%Settings:~5,1%" NEQ "T" (GOTO RPPThrottling)
set regs=!regs!^;PowerThrottling.reg
:RPPThrottling

REM ## Install Registry Settings of all Enabled Modules ##
IF "!regs!" NEQ "" (
echo Installing GameModeLib Full Edition
call "!rc!\RegImport.exe" "TTTTF" "!SIDS!" "!rc!;!ustall!;!regs:~1!"
)

IF /I "%Settings:~0,1%" NEQ "T" (GOTO MAIN)
call :GETISA
call "%~dp0GameModeLib-!ISA!^.exe" -UGenInfo "!ugen!" -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe" -PowerPlan -SetPowerPlan
call "!rc!\PowerModeOverlay.exe" "ded574b5-45a0-4f42-8737-46345c09c238"
:MAIN

REM ## Graphics 3D Settings ##
IF /I "%Settings:~1,1%" NEQ "T" (GOTO GRAPHICS)
::AMD 3D Graphics Settings
start /MIN cmd /c call "!rc!\AMD3dSettings.exe" "!ugen!"
:GRAPHICS

REM ## Update Sticky Keys ##
IF /I "%Settings:~2,1%" NEQ "T" (GOTO STKYKYS)
call "!rc!\StickyKeysSetFlag.exe" "sync"
:STKYKYS

REM ## Disable Bitlocker on C Drive If Enabled ##
IF /I "%Settings:~6,1%" NEQ "T" (GOTO BTLCKR)
echo Disabling Bitlocker OS "C:" Drive
manage-bde -unlock C^: >nul 2>&1
manage-bde -off C^: >nul 2>&1
:BTLCKR

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "%Settings:~7,1%" NEQ "T" (GOTO WDLOWCPU)
echo Enabling Windows Defender Low CPU Priority
call :CHKTAMPER
start /MIN cmd /c call "%~dp0InstallWDLowCPU.bat" "F"
:WDLOWCPU

REM ## Fully Disable Windows Defender Except FireWall ##
IF /I "%Settings:~8,1%" NEQ "T" (GOTO WDDISABLE)
echo Disabling Windows Defender
IF "!chkedtamper!" NEQ "T" (call :CHKTAMPER)
start /MIN cmd /c call "%~dp0InstallWDDisabler.bat" "F"
:WDDISABLE

echo !TIME!

:END
exit /b

:GETISA
set arc=%PROCESSOR_ARCHITEW6432%
IF "!arc!" EQU "" (set arc=%PROCESSOR_ARCHITECTURE%)
IF /I "!arc!" EQU "ARM64" (
set ISA=ARM64
exit /b
)
call "%~dp0GameModeLib-x64.exe" "/?" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (set ISA=x86) ELSE (set ISA=x64)
exit /b

:CHKTAMPER
set tameper=F
FOR /F "delims=" %%I IN ('powershell "Get-MpComputerStatus | select IsTamperProtected"') DO (
set a=%%I
set a=!a: =!
IF "!a!" EQU "True" (set tameper=T)
IF "!a!" EQU "true" (set tameper=T)
)
IF "!tameper!" EQU "T" (
cscript /NOLOGO "!rc!\MSG.vbs" "Disable Tamper Protection"
start windowsdefender://threatsettings/
set /p a="Press ENTER To Continue..."
GOTO CHKTAMPER
)
set chkedtamper=T
exit /b