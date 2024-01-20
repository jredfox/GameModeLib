@ECHO OFF
setlocal enableDelayedExpansion
REM ## Create Admin Startup of itself ##
schtasks /create /tn "WDStaticDisabler" /ru system /sc onstart /tr "%~f0" /F
powershell -ExecutionPolicy Bypass -File "%~dp0WDDisable.ps1"