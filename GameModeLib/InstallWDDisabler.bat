@ECHO OFF
setlocal enableDelayedExpansion
title Disable Windows Defender
echo Disabling Windows Defender
set ustall=%~dp0Uninstall
set rc=%~dp0Resources
REM ## Without Importing, Generate the Uninstall Global Data Not OverWriting Previous Uninstall Data ##
call "!rc!\RegImport.exe" "FFTFF" "" "!rc!;!ustall!;WDDisable.reg"
set wddisabler=!rc!\WDStaticDisable.bat
schtasks /create /tn "WDStaticDisabler" /ru system /sc onstart /tr "!wddisabler!" /F
powershell Add-MpPreference -ExclusionPath "!rc!"
call "!wddisabler!"
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
