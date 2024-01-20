@ECHO OFF
setlocal enableDelayedExpansion
call "%~dp0CheckTamper.bat"
powershell -ExecutionPolicy Bypass -File "%~dp0WDEnable.ps1"