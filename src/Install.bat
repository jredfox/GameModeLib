@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enable Registry Access ##
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul

REM ## Start Main Installation ##
set gmset=%~1
IF /I "%gmset:~0,1%" NEQ "T" (GOTO MAIN)
echo Installing GameModeLib Main Settings
call "%~dp0Executables\PowerModeOverlay.exe" "ded574b5-45a0-4f42-8737-46345c09c238"
reg import "%~dp0Main.reg"
REM ## Create GameMode Power Plan and Enable Dedicated Graphics for Java and Python ##
call :GETGMEXE
call "!gmexe!" -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe" -PowerPlan -SetPowerPlan
:MAIN

REM ## Start OEM 3d Graphics Settings ##
IF /I "%gmset:~1,1%" NEQ "T" (GOTO GRAPHICS)
echo Install GameModeLib 3D Graphic Settings
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (reg add "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" /t REG_BINARY /d 0300000000000000000000000000000000000000000000000000000002000000 /f)
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
powershell -ExecutionPolicy Bypass -File "%~dp0Executables\WDEnableLowCPU.ps1"
:WDLOWCPU

REM ## Fully Disable Windows Defender Except FireWall ##
IF /I "%gmset:~4,1%" NEQ "T" (GOTO WDDISABLE)
echo Disabling Windows Defender
call :CHKTAMPER
call "%~dp0Executables\WDStaticDisable.bat"
:WDDISABLE

REM ## Disable Sticky Keys ##
IF /I "%gmset:~5,1%" NEQ "T" (GOTO STKYKYS)
echo Disabling Sticky Keys
reg add "HKCU\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f
call "%~dp0Executables\StickyKeysSetFlag.exe" "506"
reg add "HKEY_USERS\.DEFAULT\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f
:STKYKYS

REM ## Start Disabling PalmRejction,PalmCheck, SmartSense and Disable Touchpad While Typing ##
IF /I "%gmset:~6,1%" NEQ "T" (GOTO TOUCHPAD)
set touch=F
reg query "HKCU\SOFTWARE\Elantech" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (GOTO ENDELANTECH)
echo Enabling TouchPad While Key Is Down ElanTech
set touch=T
reg import "%~dp0Elantech.reg"
:ENDELANTECH
reg query "HKLM\SOFTWARE\Synaptics" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (GOTO ENDSYN)
echo Enabling TouchPad While Key Is Down Synaptics
set touch=T
reg import "%~dp0Synaptics.reg"
FOR /F "tokens=* delims=" %%A in ('reg query "HKEY_CURRENT_USER\SOFTWARE\Synaptics\SynTP" /f "TouchPad*" ^| findstr /I /B /C:"HKEY_CURRENT_USER\\SOFTWARE\\Synaptics\\SynTP\\"') DO (
echo Disabling PalmCheck^: %%A
reg query "%%A" /v "PalmDetectConfig_Backup" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (
call :QUERYVAL "%%A" "PalmDetectConfig"
reg add "%%A" /v "PalmDetectConfig_Backup" /t REG_DWORD /d !datval! /f
)
reg add "%%A" /v "PalmDetectConfig" /t REG_DWORD /d 0 /f
reg add "%%A" /v "PalmRejectAlways" /t REG_DWORD /d 0 /f
reg add "%%A" /v "PalmRT" /t REG_DWORD /d 0 /f
)
:ENDSYN
IF "!touch!" NEQ "T" (
echo Enabling TouchPad While Key Is Down Unknown
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\PrecisionTouchPad" /v AAPThreshold /t REG_DWORD /d 0 /f
)
:TOUCHPAD

:END
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

:GETGMEXE
IF /I "!PROCESSOR_ARCHITECTURE!" EQU "ARM64" (
set gmexe=%~dp0GameModeLib-ARM64.exe
exit /b
)
set gmexe=%~dp0GameModeLib-x64.exe
call "!gmexe!" "/?" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (set gmexe=%~dp0GameModeLib-x86.exe)
exit /b
