@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enable Registry Access ##
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul
REM ## set vars ##
mkdir "%~dp0Uninstall" >nul 2>&1
call :GETISA
set dregquery=%~dp0Executables\DotRegQuery-!ISA!^.exe
set gmexe=%~dp0GameModeLib-!ISA!^.exe

REM ## Start Main Installation ##
set gmset=%~1
IF /I "%gmset:~0,1%" NEQ "T" (GOTO MAIN)
set umain=%~dp0Uninstall\Main.reg
IF NOT EXIST "!umain!" (call "!dregquery!" "%~dp0Main.reg" "%~dp0Uninstall" >"!umain!")
echo Installing GameModeLib Main Settings
REM ## Create GameMode Power Plan and Enable Dedicated Graphics for Java and Python ##
call "!gmexe!" -UGenInfo -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe" -PowerPlan -SetPowerPlan
call "%~dp0Executables\PowerModeOverlay.exe" "ded574b5-45a0-4f42-8737-46345c09c238"
REM ## Main Installation Settings ##
reg import "%~dp0Main.reg"
:MAIN

REM ## Start OEM 3d Graphics Settings ##
IF /I "%gmset:~1,1%" NEQ "T" (GOTO GRAPHICS)
echo Installing GameModeLib 3D Graphic Settings
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (
set umain=%~dp0Uninstall\Intel.reg
IF NOT EXIST "!umain!" (reg export "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" "!umain!")
reg add "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" /t REG_BINARY /d 0300000000000000000000000000000000000000000000000000000002000000 /f
)
call "%~dp0Executables\AMD3dSettings.exe" "%~dp0Uninstall"
:GRAPHICS

REM ## Disable Bitlocker on C Drive If Enabled ##
IF /I "%gmset:~2,1%" NEQ "T" (GOTO BTLCKR)
echo Disabling Bitlocker OS "C:" Drive
manage-bde -unlock C^: >nul 2>&1
manage-bde -off C^: >nul 2>&1
:BTLCKR

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "%gmset:~3,1%" NEQ "T" (GOTO WDLOWCPU)
echo Enabling Windows Defender Low CPU Priority
call :CHKTAMPER
start /MIN cmd /c call "%~dp0InstallWDLowCPU.bat" "F"
:WDLOWCPU

REM ## Fully Disable Windows Defender Except FireWall ##
IF /I "%gmset:~4,1%" NEQ "T" (GOTO WDDISABLE)
echo Disabling Windows Defender
IF "!chkedtamper!" NEQ "T" (call :CHKTAMPER)
start /MIN cmd /c call "%~dp0InstallWDDisabler.bat" "F"
:WDDISABLE

REM ## Disable Sticky Keys ##
IF /I "%gmset:~5,1%" NEQ "T" (GOTO STKYKYS)
set ureg=%~dp0Uninstall\StickyKeys.reg
IF NOT EXIST "!ureg!" (call "!dregquery!" "%~dp0StickyKeys.reg" "%~dp0Uninstall" >"!ureg!")
echo Disabling Sticky Keys
reg import "%~dp0StickyKeys.reg"
call "%~dp0Executables\StickyKeysSetFlag.exe" "506"
:STKYKYS

REM ## Start Disabling PalmRejction,PalmCheck, SmartSense and Disable Touchpad While Typing ##
IF /I "%gmset:~6,1%" NEQ "T" (GOTO TOUCHPAD)
set touch=F
reg query "HKCU\SOFTWARE\Elantech" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (GOTO ENDELANTECH)
set ureg=%~dp0Uninstall\ElanTech.reg
IF NOT EXIST "!ureg!" (call "!dregquery!" "%~dp0ElanTech.reg" "%~dp0Uninstall" >"!ureg!")
echo Enabling TouchPad While Key Is Down ElanTech
set touch=T
reg import "%~dp0Elantech.reg"
:ENDELANTECH
reg query "HKLM\SOFTWARE\Synaptics" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (GOTO ENDSYN)
echo Enabling TouchPad While Key Is Down Synaptics
set ureg=%~dp0Uninstall\Synaptics.reg
set uureg=%~dp0Uninstall\SynapticsUser.reg
IF NOT EXIST "!ureg!" (call "!dregquery!" "%~dp0Synaptics.reg" "%~dp0Uninstall" >"!ureg!")
IF NOT EXIST "!uureg!" (reg export "HKEY_CURRENT_USER\SOFTWARE\Synaptics" "!uureg!")
set touch=T
reg import "%~dp0Synaptics.reg"
FOR /F "tokens=* delims=" %%A in ('reg query "HKEY_CURRENT_USER\SOFTWARE\Synaptics\SynTP" ^| findstr /I /B /C:"HKEY_CURRENT_USER\\SOFTWARE\\Synaptics\\SynTP\\"') DO (
echo Disabling PalmCheck^: %%A
reg query "%%A" /v "PalmDetectConfig_Backup" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (
call :QUERYVAL "%%A" "PalmDetectConfig"
reg add "%%A" /v "PalmDetectConfig_Backup" /t REG_DWORD /d !datval! /f >nul 2>&1
)
reg add "%%A" /v "PalmDetectConfig" /t REG_DWORD /d 0 /f
reg add "%%A" /v "PalmRejectAlways" /t REG_DWORD /d 0 /f
reg add "%%A" /v "PalmRT" /t REG_DWORD /d 0 /f
)
:ENDSYN
REM ## handle default windows touchpad settings ##
IF "!touch!" NEQ "T" (
set ureg=%~dp0Uninstall\TouchPad.reg
IF NOT EXIST "!ureg!" (call "!dregquery!" "%~dp0TouchPadDefault.reg" "%~dp0Uninstall" >"!ureg!")
echo Enabling TouchPad While Key Is Down Unknown
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\PrecisionTouchPad" /v AAPThreshold /t REG_DWORD /d 0 /f
)
:TOUCHPAD

REM ## Disable Full Screen Optimizations That May Cause Input Lag ##
IF /I "%gmset:~7,1%" NEQ "T" (GOTO DISFSO)
set ureg=%~dp0Uninstall\DisableFSO.reg
IF NOT EXIST "!ureg!" (call "!dregquery!" "%~dp0DisableFSO.reg" "%~dp0Uninstall" >"!ureg!")
echo Disabling Full Screen Optimizations
reg import "%~dp0DisableFSO.reg"
:DISFSO

IF /I "%gmset:~8,1%" NEQ "T" (GOTO PPThrottling)
set ureg=%~dp0Uninstall\PowerThrottling.reg
IF NOT EXIST "!ureg!" (call "!dregquery!" "%~dp0PowerThrottling.reg" "%~dp0Uninstall" >"!ureg!")
echo Disabling Power Throttling
reg import "%~dp0PowerThrottling.reg"
:PPThrottling

:END
exit /b

:QUERYVAL
set datval=NUL
set key=%~1
set val=%~2
reg query "%key%" /v "%val%" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (exit /b)
FOR /F "delims=" %%A IN ('reg query "%key%" /v "%val%"') DO (
IF "%%A" NEQ "%key%" (
For /F "tokens=3*" %%B IN ("%%A") DO (
    set datval=%%B
)
)
)
exit /b

:GETISA
IF /I "!PROCESSOR_ARCHITECTURE!" EQU "ARM64" (
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
cscript /NOLOGO "%~dp0Executables\MSG.vbs" "Disable Tamper Protection"
start windowsdefender://threatsettings/
set /p a="Press ENTER To Continue..."
GOTO CHKTAMPER
)
set chkedtamper=T
exit /b