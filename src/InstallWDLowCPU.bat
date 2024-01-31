@ECHO OFF
setlocal enableDelayedExpansion
echo Enabling Windows Defender Low CPU Priority
title Enable Windows Defender Low CPU Priority
IF /I "%~1" NEQ "F" (call :CHKTAMPER)
set umain=%~dp0Uninstall\WDCPUStat.txt
IF NOT EXIST "!umain!" (
call :WDCPUSTAT
echo !scanavg! !enablelowcpu!>"!umain!"
)
powershell -ExecutionPolicy Bypass -File "%~dp0Executables\WDEnableLowCPU.ps1"
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

:WDCPUSTAT
set scanavg=NULL
set enablelowcpu=NULL
FOR /F "delims=" %%I IN ('powershell "Get-MpPreference | select ScanAvgCPULoadFactor, EnableLowCpuPriority"') DO (
set l=%%I
set l=!l: =!
IF "!l!" NEQ "" (set lineavg=%%I)
)
FOR /F "tokens=1,2 delims= " %%A IN ("!lineavg!") DO (
IF "%%A" NEQ "" (set scanavg=%%A)
IF "%%B" NEQ "" (set enablelowcpu=%%B)
)
exit /b