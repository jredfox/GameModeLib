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
call "%~dp0GameModeLib.exe" -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe" -PowerPlan -SetPowerPlan
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
