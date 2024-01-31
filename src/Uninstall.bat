@ECHO OFF
setlocal enableDelayedExpansion
set uset=%~1
set udir=%~dp0Uninstall
IF /I "%uset:~0,1%" NEQ "T" (GOTO MAIN)
REM ## Revert the Power Plan ##
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul
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
call "%~dp0Executables\PowerModeOverlay.exe" "sync"
call :USTALL "UGpuEntry.reg"
:MAIN

IF /I "%uset:~1,1%" EQU "T" (call :USTALL "Intel.reg")
IF /I "%uset:~2,1%" EQU "T" (echo ERR BitLocker Has To Be Manually Re-Installed Through Windows UI)
call :CHKTAMPER
IF /I "%uset:~3,1%" EQU "T" (start /MIN cmd /c call powershell -ExecutionPolicy Bypass -File "%~dp0Executables\WDDisableLowCPU.ps1")
IF /I "%uset:~4,1%" EQU "T" (
echo schtasks /DELETE /tn "WDStaticDisabler" /F
call "!udir!\WDStaticEnable.bat"
call :USTALL "WDNotifications.reg"
)
IF /I "%uset:~5,1%" EQU "T" (
call :USTALL "StickyKeys.reg"
REM call "%%~dp0Executables\StickyKeysSetFlag.exe" "!datval!"
)
IF /I "%uset:~6,1%" EQU "T" (
call :USTALL "TouchPad.reg"
call :USTALL "Elantech.reg"
call :USTALL "Synaptics.reg"
call :USTALL "SynapticsUser.reg"
)
IF /I "%uset:~7,1%" EQU "T" (call :USTALL "DisableFSO.reg")
exit /b

:USTALL
set name=%~1
set ufile=!udir!\!name!
reg import "!ufile!"
REM TODO verify the uninstall works before allowing deletion of reg files
REM del /F /Q /A "!ufile!" >nul 2>&1
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