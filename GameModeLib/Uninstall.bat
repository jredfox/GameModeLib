@ECHO OFF
setlocal enableDelayedExpansion
REM ## Set Global Variables ##
set uset=%~1
set udir=%~dp0Uninstall
set rc=%~dp0Resources

REM ## Start Main Uninstall ##
IF /I "%uset:~0,1%" NEQ "T" (GOTO MAIN)
echo Uninstalling GameModeLib Main Settings
REM ## Revert the Power Plan ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul
set gm=b8e6d75e-26e8-5e8f-efef-e94a209a3467
FOR /F "delims=" %%I IN ('type "!udir!\PowerPlan.txt"') DO (
set prevpp=%%I
set prevpp=!prevpp: =!
)
IF "!prevpp!" EQU "!gm!" (set prevpp=381b4222-f694-41f0-9685-ff5bb260df2e)
powercfg /SETACTIVE "!prevpp!"
If !ERRORLEVEL! NEQ 0 (powercfg /SETACTIVE "381b4222-f694-41f0-9685-ff5bb260df2e")
powercfg /DELETE "!gm!"
del /F /Q /A "!udir!\PowerPlan.txt" >nul 2>&1
call :USTALL "Main.reg"
call "!rc!\PowerModeOverlay.exe" "sync"
call :USTALL "UGpuEntry.reg"
:MAIN

IF /I "%uset:~1,1%" NEQ "T" (GOTO GRAPHICS)
echo Uninstalling GameModeLib 3D Graphic Settings
IF EXIST "!udir!\Intel.reg" (call :USTALL "Intel.reg")
IF EXIST "!udir!\AMD3DSettings.bat" (
call "!udir!\AMD3DSettings.bat"
del /F /Q /A "!udir!\AMD3DSettings.bat"
del /F /Q /A "!udir!\AMD3DSettings.zip"
)
:GRAPHICS

IF /I "%uset:~2,1%" EQU "T" (echo ERR BitLocker Has To Be Manually Re-Installed Through Windows UI)

IF /I "%uset:~3,1%" NEQ "T" (GOTO WDLOWCPU)
echo Uninstalling GameModeLib Windows Defender Low CPU
call :CHKTAMPER
FOR /F "tokens=1-2" %%A IN ('type "!udir!\WDCPUStat.txt"') DO (
set avg=%%A
set lowcpu=%%B
)
del /F /Q /A "!udir!\WDCPUStat.txt" >nul 2>&1
powershell -ExecutionPolicy Bypass -File "!rc!\WDSetLowCPU.ps1" -EnableLowCPU "!lowcpu!" -ScanAvg "!avg!"
:WDLOWCPU

IF /I "%uset:~4,1%" NEQ "T" (GOTO WDDISABLE)
echo Uninstalling GameModeLib Windows Defender Disabler
IF "!chkedtamper!" NEQ "T" (call :CHKTAMPER)
schtasks /DELETE /tn "WDStaticDisabler" /F
call "!rc!\WDStaticEnable.bat"
powershell Remove-MpPreference -ExclusionPath "!rc!"
call :USTALL "WDEnable.reg"
:WDDISABLE

REM ## Uninstall Stickey Keys and Sync Changes ##
IF /I "%uset:~5,1%" NEQ "T" (GOTO STKYKYS)
echo Uninstalling GameModeLib Sticky Keys
call :USTALL "StickyKeys.reg"
call "!rc!\StickyKeysSetFlag.exe" "sync"
:STKYKYS

REM ## Uninstall Disabling of PalmCheck ##
IF /I "%uset:~6,1%" NEQ "T" (GOTO TOUCHPAD)
echo Uninstalling PalmCheck Settings
call :USTALL "TouchPad.reg"
call :USTALL "Elantech.reg"
call :USTALL "Synaptics.reg"
REM delete registry key and apply backup for clean uninstall of synaptic edits
reg delete "HKEY_CURRENT_USER\SOFTWARE\Synaptics" /f
call :USTALL "SynapticsUser.reg"
:TOUCHPAD

REM ## Uninstall Disable Full Screen Optimizations ##
IF /I "%uset:~7,1%" NEQ "T" (GOTO DISFSO)
echo Uninstalling Full Screen Optimizations
call :USTALL "DisableFSO.reg"
:DISFSO

REM ## Uninstall Power Plan Throttling ##
IF /I "%uset:~8,1%" NEQ "T" (GOTO PPThrottling)
echo Uninstalling Power Plan Throttling
call :USTALL "PowerThrottling.reg"
:PPThrottling

:END
exit /b

:USTALL
set name=%~1
set ufile=!udir!\!name!
reg import "!ufile!"
del /F /Q /A "!ufile!" >nul 2>&1
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
