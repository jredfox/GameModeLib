@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enable Registry Access ##
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul

REM ## Start Main Installation ##
set gmset=%~1
IF /I "%gmset:~0,1%" NEQ "T" (GOTO MAIN)
echo Installing GameModeLib Main Settings
call "%~dp0Executables\PowerModeOverlay.exe" "ded574b5-45a0-4f42-8737-46345c09c238"
reg import "%~dp0Main.reg"
REM ## Create GameMode Power Plan and Enable Dedicated Graphics for Java and Python ##
call "%~dp0GameModeLib.exe" -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe" -PowerPlan -SetPowerPlan
:MAIN

:END
exit /b
