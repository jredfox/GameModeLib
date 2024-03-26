@ECHO OFF
setlocal enableDelayedExpansion

set nvfile=%~1
set log_nvidia=%~2
IF /I "%~3" EQU "T" (set NoVSYNC=/NoVSYNC) ELSE (set "NoVSYNC=")
REM ## Install NVIDIA Preffered Graphics Processor to High Performance and Gen Uninstall Data ##
IF EXIST "!nvfile!" (
call "!rc!\NVIDIA3DSettings.exe" !NoVSYNC! 2>"!log_nvidia!"
exit /b
)
call "!rc!\NVIDIA3DSettings.exe" !NoVSYNC! >"!nvfile!" 2>"!log_nvidia!"
REM ## Delete Blank NVIDIA.txt ##
call :ISBLANK "!nvfile!"
IF "!isBlank!" EQU "T" (del /F /Q /A "!nvfile!" >nul 2>&1)
exit /b

:ISBLANK
set isBlank=T
set file=%~1
FOR /F "delims=" %%A IN ('type "%file%"') DO (
set line=%%A
set line=!line: =!
IF "!line!" NEQ "" (
set isBlank=F
exit /b
)
)
exit /b
