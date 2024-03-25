@ECHO OFF
setlocal enableDelayedExpansion

REM ## Uninstall NVIDIA Settings From File And Delete the File Afterwards ##
set nvidiafile=%~1
REM ## GET NVIDIA Settings From FILE ##
FOR /F "delims=" %%I IN ('type "!nvidiafile!"') DO (
set nvset=%%I
set nvset=!nvset: =!
IF "!nvset!" NEQ "" (GOTO NVBRK)
)
:NVBRK
call "%~dp0NVIDIA3DSettings.exe" "import" "!nvset!"
del /F /Q /A "!nvidiafile!" >nul 2>&1
