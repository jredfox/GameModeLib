@ECHO OFF
setlocal enableDelayedExpansion

title Enable Windows Defender Low CPU Priority
REM ## Set Vars ##
set WDFile=%~dp0Uninstall\Global\WDCPUStat.txt
set rc=%~dp0Resources
REM ## Check Tamper And then Go to Install or Uninstall ##
set FlagsWD=%~1
IF /I "%FlagsWD:~0,1%" NEQ "F" (call :CHKTAMPER)
IF /I "%FlagsWD:~1,1%" EQU "T" (GOTO UNINSTALL)

:INSTALL
echo Enabling Windows Defender Low CPU Priority
IF NOT EXIST "!WDFile!" (
call :WDCPUSTAT
echo !scanavg! !enablelowcpu!>"!WDFile!"
)
powershell -ExecutionPolicy Bypass -File "!rc!\WDEnableLowCPU.ps1"
exit /b

:UNINSTALL
IF NOT EXIST "!WDFile!" (exit /b)
echo Uninstalling Windows Defender Low CPU Priority
FOR /F "tokens=1-2" %%A IN ('type "!WDFile!"') DO (
set avg=%%A
set lowcpu=%%B
)
del /F /Q /A "!WDFile!" >nul 2>&1
powershell -ExecutionPolicy Bypass -File "!rc!\WDSetLowCPU.ps1" -EnableLowCPU "!lowcpu!" -ScanAvg "!avg!"
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