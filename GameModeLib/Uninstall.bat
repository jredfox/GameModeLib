@ECHO OFF
setlocal enableDelayedExpansion

REM ## Set Vars ##
set Settings=%~1
set ImportGlobal=%Settings:~0,1%
set ImportUSR=%Settings:~1,1%
set Settings=%Settings:~2%
set SIDS=%~2
set u=^<u^>
set g=^<g^>
set rc=%~dp0Resources
set udir=%~dp0Uninstall
set logs=%~dp0Logs\Uninstall
set log_graphics=!logs!\log_graphics.txt
set log_wd=!logs!\log_wd.txt
set log_wdlowcpu=!logs!\log_wdlowcpu.txt
mkdir "!logs!" >nul 2>&1
call :GETISA

REM ## Enable Registry Access ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1

REM ## Main Registry Settings ##
IF /I "%Settings:~0,1%" NEQ "T" (GOTO RMAIN)
set regs=!regs!^;!g!Main.reg^;!u!Main.reg^;!u!UGpuEntry.reg
:RMAIN

REM ## Graphics 3D Settings Registry ##
IF /I "%Settings:~1,1%" NEQ "T" (GOTO RGRAPHICS)
set regs=!regs!^;!u!Intel.reg
:RGRAPHICS

REM ## Disable Sticky Keys Via The Registry ##
IF /I "%Settings:~2,1%" NEQ "T" (GOTO RSTKYKYS)
REM TODO Change uGen_StickyKeys.reg to be .DEFAULT always after confirmed working
IF /I "!ImportGlobal!" EQU "T" (set GenSticky=!u!Gen_StickyKeys.reg)
set regs=!regs!^;!u!StickyKeys.reg^;!GenSticky!
:RSTKYKYS

REM ## TouchPad Disable Palmcheck ##
IF /I "%Settings:~3,1%" NEQ "T" (GOTO RTOUCHPAD)
set regs=!regs!^;!u!TouchPad.reg^;!g!ElanTech.reg^;!u!ElanTech.reg^;!g!Synaptics.reg^;!u!SynapticsUser.reg
:RTOUCHPAD

REM ## Disable Full Screen Optimizations ##
IF /I "%Settings:~4,1%" NEQ "T" (GOTO RDISFSO)
set regs=!regs!^;!g!DisableFSO.reg^;!u!DisableFSO.reg
:RDISFSO

REM ## Disable Full Screen Optimizations ##
IF /I "%Settings:~5,1%" NEQ "T" (GOTO RPPThrottling)
set regs=!regs!^;!g!PowerThrottling.reg
:RPPThrottling

REM ## Uninstall GameMode Lib Modules ##
echo Uninstalling GameModeLib
echo call "!rc!\RegImport.exe" "!ImportGlobal!!ImportUSR!" "!SIDS!" "!udir!;NULL;!regs:~1!" "!g!=Global/^;!u!=Users/<SID>/"

REM echo "!regs:~1!"

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