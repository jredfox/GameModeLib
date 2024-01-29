@ECHO OFF
setlocal enableDelayedExpansion
echo init >nul 2>&1
REM ## Disables Windows Defender Notfications ##
reg import "%~dp0WDDisable.reg" >nul 2>&1
regedit /S "%~dp0WDDisable.reg" >nul 2>&1
REM ## Disable Windows Defender Except Firewall ##
powershell -ExecutionPolicy Bypass -File "%~dp0WDDisable.ps1" >nul 2>&1