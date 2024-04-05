@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enables Windows Defender Notfications ##
reg import "%~dp0Defaults/WD.reg"
regedit /S "%~dp0Defaults/WD.reg"
powershell -ExecutionPolicy Bypass -File "%~dp0WDEnable.ps1"
