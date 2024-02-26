@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enables Windows Defender Notfications ##
reg import "%~dp0WDEnable.reg"
regedit /S "%~dp0WDEnable.reg"
powershell -ExecutionPolicy Bypass -File "%~dp0WDEnable.ps1"
