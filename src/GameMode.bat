@ECHO OFF
setlocal enableDelayedExpansion
set UnlockBitLocker=%~1
set WDLowCPU=%~2
set WDFullDisable=%~3
REM ## Enable Registry Access ##
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes"
REM ## Generate the Uninstall Script If It Doesn't Exist ##
set uinstall=%~dp0GameModeUninstall.bat
set gpuuinstall=%~dp0GameModeUninstallGPU.bat
IF EXIST "!uinstall!" (
set GenUInstall=F
) ELSE (
echo Generating The Uninstall Script
call "%~dp0GameModeGenUninstall.bat"
set GenUInstall=T
)
echo Installing GameModeLib Full Version
REM ## Enable GameMode ##
reg add "HKCU\Software\Microsoft\GameBar" /v AllowAutoGameMode /t REG_DWORD /d 1 /f
reg add "HKCU\Software\Microsoft\GameBar" /v AutoGameModeEnabled /t REG_DWORD /d 1 /f
REM ## Enable GPU Schedualing ##
reg add "HKLM\SYSTEM\CurrentControlSet\Control\GraphicsDrivers" /v HwSchMode /t REG_DWORD /d 2 /f
REM ## Enable Graphic Settings Variable Refresh Rate Auto HDR and Optimize Windows Games##
reg add "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "DirectXUserGlobalSettings" /t REG_SZ /d "SwapEffectUpgradeEnable=1;AutoHDREnable=1;VRROptimizeEnable=1;" /f
REM ## Enable HDR Settings ##
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\HDR" /v "UseHDR" /t REG_DWORD /d 1 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\HDR" /v "AllowHDRForGames" /t REG_DWORD /d 1 /f
reg add "HKCU\System\GameConfigStore" /v "GameDVR_StreamingEnabled" /t REG_DWORD /d 1 /f
reg add "HKCU\System\GameConfigStore" /v "GameDVR_Enabled" /t REG_DWORD /d 1 /f
REM ## Enable Video Quality Settings ##
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "AllowLowResolution" /t REG_DWORD /d 0 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "DecreaseResolutionOnBattery" /t REG_DWORD /d 0 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "DisableAutoLightingOnBattery" /t REG_DWORD /d 0 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "EnableHDRForPlayback" /t REG_DWORD /d 1 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "HDColorQualityOnBattery" /t REG_DWORD /d 1 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "VideoQualityOnBattery" /t REG_DWORD /d 1 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "DisableOtherEnhancementsOnBattery" /t REG_DWORD /d 0 /f
REM ## Enable HDR On Battery ##
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "DisableHDROnBattery" /t REG_DWORD /d 0 /f
reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" /v "DisableHDRSupportOnBattery" /t REG_DWORD /d 0 /f
REM ## Enable GPU Performance on Java and Python ##
call :ADDGPU "java.exe"
call :ADDGPU "javaw.exe"
call :ADDGPU "py.exe"
call :ADDGPU "pyw.exe"
REM ## WARN USER ##
echo Increase CPU Performance by Disabling Windows Defender Realtime and Tamper Proection while gaming but NEVER While the WEB BROWSER or EMAILS are open^.
echo Increase Disk ^(SSD or HDD^) Performance by disabling bitlocker if you don't need the Security
REM ## Create Game Mode Power Plan ##
call "%~dp0GameModePowerPlan.bat"
REM ## Fix Power Plan Performance ##
powercfg /setacvalueindex SCHEME_CURRENT SUB_ENERGYSAVER ESBATTTHRESHOLD 0
powercfg /setdcvalueindex SCHEME_CURRENT SUB_ENERGYSAVER ESBATTTHRESHOLD 0
set BestPP=ded574b5-45a0-4f42-8737-46345c09c238
call "%~dp0Executables\PowerModeOverlay.exe" "!BestPP!"
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayAcPowerScheme" /t REG_SZ /d "!BestPP!" /f
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayDcPowerScheme" /t REG_SZ /d "!BestPP!" /f
REM ################################### Enable OEM GPU 3D Settings to Game Mode Performance ####################################
::Intel HD Graphics Control Pannel Performance Settings
reg query "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" >nul 2>&1
IF !ERRORLEVEL! EQU 0 (reg add "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" /v "Default" /t REG_BINARY /d 0300000000000000000000000000000000000000000000000000000002000000 /f)

REM ################################### START DISABLING ANOYING THINGS ####################################
REM ## Disable Sticky Keys ##
reg add "HKCU\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f
call "%~dp0Executables\StickyKeysSetFlag.exe" "506"
reg add "HKEY_USERS\.DEFAULT\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f
REM ## Disable BitLocker to Dramtically Improve SSD Performance ##
IF /I "%UnlockBitLocker:~0,1%" EQU "T" (
echo Disabling Bitlocker OS "C:" Drive
manage-bde -unlock C^: >nul 2>&1
manage-bde -off C^: >nul 2>&1
)
REM ## Make Windows Defender Realtime protection Low priority so it doesn't take up more then 15 percent of CPU Usage when running games ##
IF /I "%WDLowCPU:~0,1%" EQU "T" (
echo Enabling Windows Defender Low CPU Priority
call :CHKTAMPER
REM ## EnableLowCpuPriority Win 10 1809 ##
powershell Set-MpPreference -Force -EnableLowCpuPriority ^$true
powershell Set-MpPreference -Force -ScanAvgCPULoadFactor 15
)
REM ## Fully Disable Windows Defender Except FireWall ##
IF /I "%WDFullDisable:~0,1%" EQU "T" (
echo Disabling Windows Defender
call :CHKTAMPER
call "%~dp0Executables\WDStaticDisable.bat"
)

:END
exit /b

:ADDGPU
set exe=%~1
FOR /F "delims=" %%I IN ('where "!exe!" 2^>nul') DO (
IF "%%I" NEQ "" (
reg query "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "%%I" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (
reg add "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "%%I" /t REG_SZ /d "GpuPreference=2;" /f >nul 2>&1
REM ## Update GPU Entry UnInstall Script ##
(
IF "!GenUInstall!" EQU "T" (echo reg delete "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "%%I" /f)
)>>"!gpuuinstall!"
)
)
)
exit /b

:CHKTAMPER
set tameper=F
FOR /F "delims=" %%I IN ('powershell "Get-MpComputerStatus | select IsTamperProtected"') DO (
set a=%%I
set a=!a: =!
IF "!a!" EQU "True" (set tameper=T)
IF "!a!" EQU "true" (set tameper=T)
)
IF "!tameper!" EQU "T" (
cscript /NOLOGO "%~dp0Executables\MSG.vbs" "Disable Tamper Protection"
start windowsdefender://threatsettings/
set /p a="Press ENTER To Continue..."
GOTO CHKTAMPER
)
exit /b