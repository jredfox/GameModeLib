@ECHO OFF
setlocal enableDelayedExpansion
echo Disabling Windows Defender
title Disable Windows Defender
call :GETISA
set dregquery=%~dp0Executables\DotRegQuery-!ISA!^.exe
IF /I "%~1" NEQ "F" (call :CHKTAMPER)
set wddisabler=%~dp0Executables\WDStaticDisable.bat
set umain=%~dp0Uninstall\WDEnable.reg
IF NOT EXIST "!umain!" (call "!dregquery!" "%~dp0Executables\WDDisable.reg" "%~dp0Uninstall" >"!umain!")
schtasks /create /tn "WDStaticDisabler" /ru system /sc onstart /tr "!wddisabler!" /F
powershell Add-MpPreference -ExclusionPath "%~dp0Executables"
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
cscript /NOLOGO "%~dp0Executables\MSG.vbs" "Disable Tamper Protection"
start windowsdefender://threatsettings/
set /p a="Press ENTER To Continue..."
GOTO CHKTAMPER
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