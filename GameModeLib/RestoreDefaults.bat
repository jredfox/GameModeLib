@ECHO OFF
setlocal enableDelayedExpansion

REM ## Set Vars ##
set Settings=%~1
set SIDS=%~2
set rc=%~dp0Resources

REM ## Enable Registry Access ##
call "!rc!\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul 2>&1

REM ## Main Registry Settings ##
IF /I "!Settings:~0,1!" NEQ "T" (GOTO RMAIN)
set regs=!regs!^;Main.reg
:RMAIN
