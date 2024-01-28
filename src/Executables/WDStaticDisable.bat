@ECHO OFF
setlocal enableDelayedExpansion
REM ## Disables Windows Defender Notfications ##
reg import "%~dp0WDDisable.reg"
REM ## Disable Windows Defender Except Firewall ##
powershell -ExecutionPolicy Bypass -File "%~dp0WDDisable.ps1"
