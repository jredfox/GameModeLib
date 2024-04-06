@ECHO OFF
setlocal enableDelayedExpansion

REM ## Set Vars ##
set Settings=%~1
set SIDS=%~2
set rc=%~dp0Resources
set itmp=%~dp0TMP\Install
set dirdef=!rc!\Defaults

REM ## Enable Registry Access ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1
call :CLNUP

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

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "!Settings:~7,1!" NEQ "T" (GOTO WDLOWCPU)
call :CHKTAMPER
echo powershell -ExecutionPolicy Bypass -File "!rc!\WDDisableLowCPU.ps1"
:WDLOWCPU

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "!Settings:~8,1!" NEQ "T" (GOTO WDENABLE)
call :CHKTAMPER
echo powershell -ExecutionPolicy Bypass -File "!rc!\WDEnable.ps1"
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