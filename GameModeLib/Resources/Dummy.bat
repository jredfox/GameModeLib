@ECHO OFF
setlocal enableDelayedExpansion

REM ## Enable Delayed Expansions And Work Around Windows Bug Where First Scripts Ever Running on Windows Fails ##
call echo Hello CMD >nul 2>&1
powershell -ExecutionPolicy Bypass -File "%~dp0\Dummy.ps1" >nul 2>&1
exit /b
