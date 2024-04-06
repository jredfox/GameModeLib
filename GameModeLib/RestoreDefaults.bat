@ECHO OFF
setlocal enableDelayedExpansion

REM ## Set Vars ##
set Settings=%~1
set SIDS=%~2
set rc=%~dp0Resources
set dirdef=!rc!\Defaults
set itmp=%~dp0TMP\Install
set uglobal=%~dp0Uninstall\Global
set log_amd=!logs!\log_uninstall_amd.txt

REM ## INIT ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1
call :CLNUP
mkdir "!logs!" >nul 2>&1

REM ## Main Registry Settings ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO RMAIN)
set regs=!regs!^;Main.reg
:RMAIN

REM ## Graphics 3D Settings Registry ##
IF /I "!Settings:~1,1!" NEQ "T" (GOTO RGRAPHICS)
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;Intel.reg)
:RGRAPHICS

REM ## Disable Sticky Keys Via The Registry ##
IF /I "!Settings:~2,1!" NEQ "T" (GOTO RSTKYKYS)
set regs=!regs!^;StickyKeys.reg
:RSTKYKYS

REM ## TouchPad Disable Palmcheck ##
IF /I "!Settings:~3,1!" NEQ "T" (GOTO RTOUCHPAD)
set regs=!regs!^;TouchPad.reg
REM ## Synaptics ##
set syntp=!itmp!\SynapticsUser.reg
call "!rc!\GenSynaptics.exe" "/RestoreDefaults" "!syntp!"
IF !ERRORLEVEL! EQU 0 (set regs=!regs!^;!syntp!)
:RTOUCHPAD

REM ## Main Registry Settings ##
IF /I "!Settings:~4,1!" NEQ "T" (GOTO RFSO)
set regs=!regs!^;FSO.reg^;FSO_1.reg
:RFSO

REM ## WD Default Notification Settings ##
IF /I "!Settings:~8,1!" NEQ "T" (GOTO RWD)
call :CHKTAMPER
set regs=!regs!^;WD.reg
:RWD

REM ## NVIDIA Preffered GPU to the Power Plan ##
IF /I "!Settings:~9,1!" NEQ "T" (GOTO RNVPP)
set regs=!regs!^;NVPP.reg
:RNVPP

REM ## Install Registry Settings of all Enabled Modules ##
IF "!regs!" NEQ "" (
echo Restoring Windows Default Settings
call "!rc!\RegImport.exe" "TT" "!SIDS!" "!dirdef!;NULL;!regs:~1!"
)

REM ## Graphics 3D Settings Registry ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO MAIN)
powercfg /SETACTIVE "381b4222-f694-41f0-9685-ff5bb260df2e"
powercfg /DELETE "b8e6d75e-26e8-5e8f-efef-e94a209a3467"
REM call "!rc!\PowerModeOverlay.exe" "sync" TODO After Power Plan Overlay Gets Restored Uncomment this
:MAIN

REM ## Graphics 3D Settings Registry ##
IF /I "!Settings:~1,1!" NEQ "T" (GOTO GRAPHICS)
REM We can only uninstall AMD3DSettings if Uninstall Gen Data was created CNCMD.exe if it does support a restore command it's not documented
IF EXIST "!uglobal!\AMD3DSettings.bat" (
start /MIN cmd /c call "!uglobal!\AMD3DSettings.bat" "F" ^>"!log_amd!" ^2^>^&1
)
call "!rc!\NVIDIA3DSettings.exe" import "0x00A879CF=Default;0x1057EB71=Default;0x10F9DC80=Default;0x10F9DC81=Default;0x10F9DC84=Default" >nul 2>&1
:GRAPHICS

REM ## Update Sticky Keys ##
IF /I "!Settings:~2,1!" NEQ "T" (GOTO STKYKYS)
call "!rc!\StickyKeysSetFlag.exe" "sync"
:STKYKYS

REM ## Full Screen Optimizations ##
IF /I "!Settings:~4,1!" NEQ "T" (GOTO FSO)
del /F /Q /A "%PROGRAMFILES%\GameModeLib\DisableFSO.bat" >nul 2>&1
rd /Q "%PROGRAMFILES%\GameModeLib" >nul 2>&1
:FSO

REM ## NVIDIA Preffered GPU to the Power Plan ##
IF /I "!Settings:~9,1!" NEQ "T" (GOTO NVPP)
taskkill /F /FI "IMAGENAME eq GameModeLibNVPP*"
schtasks /DELETE /tn "GameModeLibNVPP" /F
set GameModeLibNVPP=
call "!rc!\GameModeLibNVPP-CLI.exe" "/Uninstall" >nul 2>&1
REM Del Old Files
del /F /Q /A "!PROGRAMFILES!\GameModeLib\GameModeLibNVPP.exe" >nul 2>&1
rd /Q "%PROGRAMFILES%\GameModeLib" >nul 2>&1
:NVPP

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "!Settings:~7,1!" NEQ "T" (GOTO WDLOWCPU)
call :CHKTAMPER
powershell -ExecutionPolicy Bypass -File "!rc!\WDDisableLowCPU.ps1"
:WDLOWCPU

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "!Settings:~8,1!" NEQ "T" (GOTO WDENABLE)
IF "!chkedtamper!" NEQ "T" (call :CHKTAMPER)
schtasks /DELETE /tn "WDStaticDisabler" /F
powershell -ExecutionPolicy Bypass -File "!rc!\WDEnable.ps1"
powershell Remove-MpPreference -ExclusionPath "!rc!"
:WDENABLE

:END
call :CLNUP
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

:HASNV
call "!rc!\GameModeLibNVPP-CLI.exe" "/?" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set HasNV=T) ELSE (set HasNV=F)
set HasNV=T
exit /b
