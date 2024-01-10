@ECHO OFF
setlocal enableDelayedExpansion
REM ## Enable GameMode ##
reg add "HKCU\Software\Microsoft\GameBar" /v AllowAutoGameMode /t REG_DWORD /d 1 /f
reg add "HKCU\Software\Microsoft\GameBar" /v AutoGameModeEnabled /t REG_DWORD /d 1 /f
REM ## Enable GPU Schedualing ##
reg add "HKLM\SYSTEM\CurrentControlSet\Control\GraphicsDrivers" /v HwSchMode /t REG_DWORD /d 2 /f
REM ## Enable HDR Settings WIP ##
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

REM ################################### START DISABLING ANOYING THINGS ####################################
REM ## Disable Sticky Keys REQUIRES Logging Off and Back On ##
reg add "HKCU\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f

REM ## Enable Default Graphic Settings ##

REM ## Enable Variable Refresh Rate ## HKEY_CURRENT_USER\SOFTWARE\Microsoft\DirectX\UserGpuPreferences\DirectXUserGlobalSettings && HKEY_CURRENT_USER\Software\Microsoft\DirectX\UserGpuPreferences
REM ## VALUES = SwapEffectUpgradeEnable=1;AutoHDREnable=1;VRROptimizeEnable=1;
FOR /F "tokens=3*" %%a in ('reg query "HKEY_CURRENT_USER\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "DirectXUserGlobalSettings" 2^>nul') DO (set VRR=%%a)
echo !VRR!
pause
exit /b