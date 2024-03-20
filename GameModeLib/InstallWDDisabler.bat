@ECHO OFF
setlocal enableDelayedExpansion

title Disable Windows Defender
REM ## Set Vars ##
set ustall=%~dp0Uninstall
set rc=%~dp0Resources
mkdir "%~dp0Uninstall\Global" >nul 2>&1
REM ## Check Tamper And then Go to Install or Uninstall ##
set FlagsWD=%~1
IF "!FlagsWD!" EQU "" (set "FlagsWD= ")
IF /I "!FlagsWD:~0,1!" NEQ "F" (call :CHKTAMPER)
IF /I "!FlagsWD:~1,1!" EQU "T" (GOTO UNINSTALL)

:INSTALL
echo Disabling Windows Defender
call "!rc!\RegImport.exe" "FFT" "" "!rc!;!ustall!;WDDisable.reg"
set wddisabler=!rc!\WDStaticDisable.bat
schtasks /create /tn "WDStaticDisabler" /ru system /sc onstart /tr "!wddisabler!" /F
powershell Add-MpPreference -ExclusionPath "!rc!"
call "!wddisabler!"
exit /b

:UNINSTALL
echo Enabling Windows Defender
set WDFile=%~dp0Uninstall\Global\WDDisable.reg
REM ## Use Reg and Regedit as it Works Better with Windows Defender Registry Notifications ##
IF EXIST "!WDFile!" (
  reg import "!WDFile!"
  regedit /S "!WDFile!"
  del /F /Q /A "!WDFile!" >nul 2>&1
)
schtasks /DELETE /tn "WDStaticDisabler" /F
powershell -ExecutionPolicy Bypass -File "!rc!\WDEnable.ps1"
powershell Remove-MpPreference -ExclusionPath "!rc!"
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
exit /b
