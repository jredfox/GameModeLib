@ECHO OFF
setlocal enableDelayedExpansion

REM ## Set Vars ##
set Settings=%~1
IF "!Settings!" EQU "" (exit /b)
set ImportGlobal=!Settings:~0,1!
set ImportUSR=!Settings:~1,1!
set Settings=!Settings:~2!
set SIDS=%~2
set u=^<u^>
set g=^<g^>
set gg=!g!
set rc=%~dp0Resources
set rcdef=!rc!\Defaults
set udir=%~dp0Uninstall
set uglobal=%~dp0Uninstall\Global
set logs=%~dp0Logs
set log_amd=!logs!\log_uninstall_amd.txt
set log_wd=!logs!\log_uninstall_wd.txt
set log_wdlowcpu=!logs!\log_uninstall_wdlowcpu.txt
set log_nvidia=!logs!\log_uninstall_nvidia.txt
mkdir "!logs!" >nul 2>&1
call :GETISA
REM Enforce Non Admins Can Only Use Non Admin Reg Files
call :CHKADMIN
IF "!IsAdmin!" NEQ "T" (
set "SIDS="
set ImportGlobal=F
set ImportUSR=T
set gg=^<ACESS_DENIED^>
)

REM ## Set HasDef ##
call "%~dp0\Resources\FindSplitStr.exe" "!SIDS!" ";" "Default" "True"
IF !ERRORLEVEL! NEQ 0 (set HasDef=F) ELSE (set HasDef=T)
REM ## Enable Registry Access ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1

REM ## Main Registry Settings ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO RMAIN)
set regs=!regs!^;!g!Main.reg^;!u!Main.reg^;!u!UGpuEntry.reg
:RMAIN

REM ## Graphics 3D Settings Registry ##
IF /I "!Settings:~1,1!" NEQ "T" (GOTO RGRAPHICS)
set regs=!regs!^;!u!Intel.reg
:RGRAPHICS

REM ## Disable Sticky Keys Via The Registry ##
IF /I "!Settings:~2,1!" NEQ "T" (GOTO RSTKYKYS)
set regs=!regs!^;!u!StickyKeys.reg
IF /I "!ImportGlobal!" EQU "T" (set regs=!regs!^;Users\^.DEFAULT\StickyKeys_gen.reg)
:RSTKYKYS

REM ## TouchPad Disable Palmcheck ##
IF /I "!Settings:~3,1!" NEQ "T" (GOTO RTOUCHPAD)
set regs=!regs!^;!u!TouchPad.reg^;!g!ElanTech.reg^;!u!ElanTech.reg^;!g!Synaptics.reg^;!u!SynapticsUser.reg
:RTOUCHPAD

REM ## Disable Full Screen Optimizations ##
IF /I "!Settings:~4,1!" NEQ "T" (GOTO DISFSO)
set regs=!regs!^;!rcdef!\FSO_1.reg
IF "!HasDef!" NEQ "T" (GOTO DISFSO)
del /F /Q /A "%PROGRAMFILES%\GameModeLib\DisableFSO.bat" >nul 2>&1
rd /Q "%PROGRAMFILES%\GameModeLib" >nul 2>&1
:DISFSO

REM ## Disable Full Screen Optimizations ##
IF /I "!Settings:~5,1!" NEQ "T" (GOTO RPPThrottling)
set regs=!regs!^;!g!PowerThrottling.reg
:RPPThrottling

REM ## NVIDIA PowerPlan If Enabled ##
IF /I "!Settings:~9,1!" NEQ "T" (GOTO RNVPP)
set regs=!regs!^;!rcdef!\NVPP.reg
:RNVPP

REM ## Uninstall GameMode Lib Modules ##
IF "!regs!" NEQ "" (
echo Uninstalling GameModeLib Full Edition
call "!rc!\RegImport.exe" "!ImportGlobal!!ImportUSR!FFT" "!SIDS!" "!udir!;NULL;!regs:~1!" "!gg!=Global/^;!u!=Users/<SID>/" /SkipWithoutSIDS:Default
)

REM ## Main Module Revert Power Plan Settings ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO MAIN)
echo Uninstalling GameModeLib Main
REM ## Revert The Power Plan and Delete Default GameModeLib PowerPlan ##
set gm=b8e6d75e-26e8-5e8f-efef-e94a209a3467
set ppfile=!uglobal!\PowerPlan.txt
IF NOT EXIST "!ppfile!" (GOTO PPUEND)
FOR /F "delims=" %%I IN ('type "!ppfile!"') DO (
set prevpp=%%I
set prevpp=!prevpp: =!
)
IF "!prevpp!" EQU "!gm!" (set prevpp=381b4222-f694-41f0-9685-ff5bb260df2e)
powercfg /SETACTIVE "!prevpp!"
If !ERRORLEVEL! NEQ 0 (powercfg /SETACTIVE "381b4222-f694-41f0-9685-ff5bb260df2e")
powercfg /DELETE "!gm!"
del /F /Q /A "!ppfile!" >nul 2>&1
:PPUEND
REM ## Sync Registry Changes for the PowerMode Overlay ##
call "!rc!\PowerModeOverlay.exe" "sync"
:MAIN

IF /I "!Settings:~1,1!" NEQ "T" (GOTO GRAPHICS)
echo Uninstalling GameModeLib Graphics
IF EXIST "!uglobal!\AMD3DSettings.bat" (
start /MIN cmd /c call "!uglobal!\AMD3DSettings.bat" "T" ^>"!log_amd!" ^2^>^&1
)
set nvidiafile=!uglobal!\NVIDIA.txt
IF NOT EXIST "!nvidiafile!" (GOTO GRAPHICS)
start /MIN cmd /c call "!rc!\NVIDIAUninstall.bat" "!nvidiafile!" ^>"!log_nvidia!" ^2^>^&1
:GRAPHICS

REM ## Uninstall Stickey Keys and Sync Changes ##
IF /I "!Settings:~2,1!" NEQ "T" (GOTO STKYKYS)
call "!rc!\StickyKeysSetFlag.exe" "sync"
:STKYKYS

REM ## NVIDIA Preffered GPU to the Power Plan ##
IF /I "!Settings:~9,1!" NEQ "T" (GOTO NVPP)
taskkill /F /FI "IMAGENAME eq GameModeLibNVPP*"
schtasks /DELETE /tn "GameModeLibNVPP" /F >nul 2>&1
set GameModeLibNVPP=
call "!rc!\GameModeLibNVPP-CLI.exe" "/Uninstall"
del /F /Q /A "!PROGRAMFILES!\GameModeLib\GameModeLibNVPP.exe" >nul 2>&1
REM Del Directory Only If It Is Empty
rd /Q "%PROGRAMFILES%\GameModeLib" >nul 2>&1
:NVPP

REM ## Skip Admin Only Modules ##
IF "!IsAdmin!" NEQ "T" (GOTO END)

REM ## Warn User that BitLocker Has to be Enabled through UI If they Tried Using Uninstall via CLI ##
IF /I "!Settings:~6,1!" EQU "T" (echo ERROR BitLocker Has To Be Manually Re-Installed Through Windows UI)

REM ## Enable Windows Defender Low CPU Priority ##
IF /I "!Settings:~7,1!" NEQ "T" (GOTO WDLOWCPU)
echo Uninstalling GameModeLib Windows Defender Low CPU
call :CHKTAMPER
start /MIN cmd /c call "%~dp0InstallWDLowCPU.bat" "FT" ^>"!log_wdlowcpu!" ^2^>^&1
:WDLOWCPU

REM ## Fully Disable Windows Defender Except FireWall ##
IF /I "!Settings:~8,1!" NEQ "T" (GOTO WDDISABLE)
echo Uninstalling WDDisabler
IF "!chkedtamper!" NEQ "T" (call :CHKTAMPER)
start /MIN cmd /c call "%~dp0InstallWDDisabler.bat" "FT" ^>"!log_wd!" ^2^>^&1
:WDDISABLE

:END
exit /b

:GETISA
set arc=%PROCESSOR_ARCHITEW6432%
IF "!arc!" EQU "" (set arc=%PROCESSOR_ARCHITECTURE%)
IF /I "!arc!" EQU "ARM64" (
set ISA=ARM64
exit /b
)
call "%~dp0GameModeLib-x64.exe" "/?" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (set ISA=x86) ELSE (set ISA=x64)
exit /b

:CHKADMIN
set IsAdmin=F
net session >nul 2>&1
IF !ERRORLEVEL! EQU 0 (set IsAdmin=T)
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