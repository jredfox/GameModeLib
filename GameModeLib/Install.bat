@ECHO OFF
setlocal enableDelayedExpansion

REM #################### Usage #############################################################################################
REM ##  Modules are T/F and are Indexed From 0-8 and is the First Argument example "TTTTTTTTT"                            ##
REM ##  Main, Graphics, Sticky Keys, TouchPad, Full Screen Optimizations, PowerThrottling, BitLocker, WDLowCPU, WDDisable ##
REM ##  SIDs or Usernames or * for all Users is the Second Argument Examples Below.                                       ##
REM ##  ".DEFAULT" User When your Signed Out and "Default" All Future Users                                               ##
REM ##  "*" "jredfox;jgrayfox" "S-1-5-21-368394509-689051271-14200874-1012;S-1-5-21-368394509-689051271-14200874-1013"    ##
REM ##  "*;Default" Applies to all current and new users                                                                  ##
REM ########################################################################################################################

REM ## Set Vars ##
echo Time !TIME!
set Settings=%~1
set SIDS=%~2
set rc=%~dp0Resources
set ustall=%~dp0Uninstall
set ugen=!ustall!\Global
set itmp=%~dp0TMP\Install
set gmpdir=%PROGRAMFILES%\GameModeLib
set nvfile=!ugen!\NVIDIA.txt
set logs=%~dp0Logs
set log_amd=!logs!\log_amd.txt
set log_nvidia=!logs!\log_nvidia.txt
set log_wd=!logs!\log_wd.txt
set log_wdlowcpu=!logs!\log_wdlowcpu.txt
mkdir "!ugen!" >nul 2>&1
mkdir "!logs!" >nul 2>&1
mkdir "!gmpdir!" >nul 2>&1
call :CLNUP
call :GETISA
call :CHKADMIN
call :HASNV
REM ## Set HasDef ##
call "%~dp0\Resources\FindSplitStr.exe" "!SIDS!" ";" "Default" "True"
IF !ERRORLEVEL! NEQ 0 (set HasDef=F) ELSE (set HasDef=T)
REM ## Enable Registry Access ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1

REM ## Main Registry Settings ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO RMAIN)
set regs=!regs!^;Main.reg
:RMAIN

REM ## Graphics 3D Settings Registry ##
IF /I "!Settings:~1,1!" NEQ "T" (GOTO RGRAPHICS)
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Intel.reg)
:RGRAPHICS

REM ## Disable Sticky Keys Via The Registry ##
IF /I "!Settings:~2,1!" NEQ "T" (GOTO RSTKYKYS)
set regs=!regs!^;StickyKeys.reg
:RSTKYKYS

REM ## TouchPad Disable Palmcheck ##
IF /I "!Settings:~3,1!" NEQ "T" (GOTO RTOUCHPAD)
set regs=!regs!^;TouchPad.reg
REM ## Elantech ##
reg query "HKCU\SOFTWARE\Elantech" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;ElanTech.reg)
REM ## Synaptics ##
set syntp=!itmp!\SynapticsUser.reg
call "!rc!\GenSynaptics.exe" "!syntp!"
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Synaptics.reg^;!syntp!)
:RTOUCHPAD

REM ## Disable Full Screen Optimizations All Current Users ##
IF /I "!Settings:~4,1!" NEQ "T" (GOTO DISFSO)
set regs=!regs!^;DisableFSO.reg
REM ## Disable Full Screen Optimizations For All New Users ##
IF /I "!HasDef!" NEQ "T" (GOTO DISFSO)
echo Installing GameModeLib DisableFSO For New Users
copy /Y "!rc!\DisableFSO.bat" "!gmpdir!\DisableFSO.bat" >nul 2>&1
set regs=!regs!^;DisableFSO_1.reg
:DISFSO

REM ## Disable Full Screen Optimizations ##
IF /I "!Settings:~5,1!" NEQ "T" (GOTO RPPThrottling)
set regs=!regs!^;PowerThrottling.reg
:RPPThrottling

REM ## NVIDIA PowerPlan If Enabled ##
IF /I "!Settings:~9,1!" NEQ "T" (GOTO RNVPP)
IF "!HASNV!" NEQ "T" (GOTO RNVPP)
IF /I "!IsAdmin!" NEQ "T" (set regs=!regs!^;GameModeLibNVPP.reg)
:RNVPP

REM ## Create Power Plan and Generate GPUEntries reg ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO MAIN)
call "%~dp0GameModeLib-!ISA!^.exe" -GenReg -UGenInfo "%~dp0TMP\Install" -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe" -PowerPlan -SetPowerPlan
set regs=!regs!^;%~dp0TMP\Install\UGpuEntry.reg
REM Enforce NVIDIA GPU Is Always Used Statically As It Should Be Part of the power plan TODO When GameModeLibNVPP.exe SubModule Is Installed Do Not Run this Code
IF /I "!Settings:~1,1!" NEQ "T" (call "!rc!\NVIDIAInstall.bat" "!nvfile!" "!log_nvidia!" "T")
:MAIN

REM ## Install Registry Settings of all Enabled Modules ##
IF "!regs!" NEQ "" (
echo Installing GameModeLib Full Edition
call "!rc!\RegImport.exe" "TTTT" "!SIDS!" "!rc!;!ustall!;!regs:~1!"
)

IF /I "!Settings:~0,1!" NEQ "T" (GOTO MAINPOST)
IF NOT EXIST "!ugen!\PowerPlan.txt" (copy /Y "!itmp!\PowerPlan.txt" "!ugen!\PowerPlan.txt" >nul)
call "!rc!\PowerModeOverlay.exe" "ded574b5-45a0-4f42-8737-46345c09c238"
:MAINPOST

REM ## Graphics 3D Settings ##
IF /I "!Settings:~1,1!" NEQ "T" (GOTO GRAPHICS)
::AMD 3D Graphics Settings
start /MIN cmd /c call "!rc!\AMD3dSettings.exe" "!ugen!" ^>"!log_amd!" ^2^>^&1
::NVIDIA 3D Graphics Settings Set Preffered Graphics Processor to High Performance
start /MIN cmd /c call "!rc!\NVIDIAInstall.bat" "!nvfile!" "!log_nvidia!"
:GRAPHICS

REM ## Update Sticky Keys ##
IF /I "!Settings:~2,1!" NEQ "T" (GOTO STKYKYS)
call "!rc!\StickyKeysSetFlag.exe" "sync"
:STKYKYS

REM ## NVIDIA Preffered GPU to the Power Plan ##
IF /I "!Settings:~9,1!" NEQ "T" (GOTO NVPP)
IF "!HASNV!" NEQ "T" (GOTO NVPP)
taskkill /F /FI "IMAGENAME eq GameModeLibNVPP*"
start /B "" "!rc!\GameModeLibNVPP-CLI.exe" "/Install" "/NoPwR" "/F"
IF /I "!IsAdmin!" NEQ "T" (
setx "GameModeLibNVPP" "!rc!\GameModeLibNVPP.exe"
set GameModeLibNVPP=!rc!\GameModeLibNVPP.exe
) ELSE (
copy /Y "!rc!\GameModeLibNVPP.exe" "!gmpdir!\GameModeLibNVPP.exe" >nul 2>&1
set nvp=\"!gmpdir!\GameModeLibNVPP.exe\"
schtasks /create /tn "GameModeLibNVPP" /tr "!nvp!" /sc onlogon /F
)
:NVPP

REM ## Start ADMIN Only Modules ##
IF /I "!IsAdmin!" NEQ "T" (GOTO END)

REM ## Disable Bitlocker on C Drive If Enabled ##
IF /I "!Settings:~6,1!" NEQ "T" (GOTO BTLCKR)
echo Disabling Bitlocker OS "C:" Drive
manage-bde -unlock C^: >nul 2>&1
manage-bde -off C^: >nul 2>&1
:BTLCKR

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "!Settings:~7,1!" NEQ "T" (GOTO WDLOWCPU)
echo Enabling Windows Defender Low CPU Priority
call :CHKTAMPER
start /MIN cmd /c call "%~dp0InstallWDLowCPU.bat" "F" ^>"!log_wdlowcpu!" ^2^>^&1
:WDLOWCPU

REM ## Fully Disable Windows Defender Except FireWall ##
IF /I "!Settings:~8,1!" NEQ "T" (GOTO WDDISABLE)
echo Disabling Windows Defender
IF "!chkedtamper!" NEQ "T" (call :CHKTAMPER)
start /MIN cmd /c call "%~dp0InstallWDDisabler.bat" "F" ^>"!log_wd!" ^2^>^&1
:WDDISABLE

:END
call :CLNUP
echo Time !TIME!
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

:CLNUP
del /F /S /Q /A "%~dp0TMP" >nul 2>&1
rd /S /Q "%~dp0TMP" >nul 2>&1
exit /b

:CHKADMIN
set IsAdmin=F
net session >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set IsAdmin=T)
exit /b

:HASNV
call "!rc!\GameModeLibNVPP-CLI.exe" "/?" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set HasNV=T) ELSE (set HasNV=F)
set HasNV=T
exit /b
