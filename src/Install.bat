@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enable Registry Access ##
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" >nul

REM ## Start Main Installation ##
set gmset=%~1
IF /I "%gmset:~0,1%" NEQ "T" (GOTO MAIN)
echo Installing GameModeLib Main Settings
reg import "%~dp0Main.reg"
REM ## Create GameMode Power Plan and Enable Dedicated Graphics for Java and Python ##
call "%~dp0GameModeLib.exe" -PowerPlan -GPUEntry "java.exe;javaw.exe;py.exe;pyw.exe"
REM ## Fix Power Plan Performance ##
set BestPP=ded574b5-45a0-4f42-8737-46345c09c238
call "%~dp0Executables\PowerModeOverlay.exe" "!BestPP!"
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayAcPowerScheme" /t REG_SZ /d "!BestPP!" /f
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayDcPowerScheme" /t REG_SZ /d "!BestPP!" /f
:MAIN

:END
exit /b
