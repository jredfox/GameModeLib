@ECHO OFF
setlocal enableDelayedExpansion

REM ## Uninstall NVIDIA Settings From File And Delete the File Afterwards ##
set nvidiafile=%~1
set log_nvidia=%~2
REM ## GET NVIDIA Settings From FILE ##
FOR /F "delims=" %%I IN ('type "!nvidiafile!"') DO (
set nvset=%%I
set nvset=!nvset: =!
IF "!nvset!" NEQ "" (GOTO NVBRK)
)
:NVBRK
"%~dp0NVIDIA3DSettings.exe" "import" "!nvset!" >"!log_nvidia!" 2>&1
del /F /Q /A "!nvidiafile!" >nul 2>&1
