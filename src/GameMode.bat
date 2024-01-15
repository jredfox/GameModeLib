@ECHO OFF
setlocal enableDelayedExpansion
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
echo Increase CPU usage by Disabling Windows Defender Realtime and Tamper Proection while gaming but NEVER While the WEB BROWSER or EMAILS are open^.
REM ## Create Game Mode Power Plan ##
call "%~dp0GameModePowerPlan.bat"
REM ## Fix Power Plan Performance ##
powercfg /setacvalueindex SCHEME_CURRENT SUB_ENERGYSAVER ESBATTTHRESHOLD 0
powercfg /setdcvalueindex SCHEME_CURRENT SUB_ENERGYSAVER ESBATTTHRESHOLD 0
set BestPP=ded574b5-45a0-4f42-8737-46345c09c238
call "%~dp0Executables\PowerModeOverlay.exe" "!BestPP!"
call "%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes"
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayAcPowerScheme" /t REG_SZ /d "!BestPP!" /f
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayDcPowerScheme" /t REG_SZ /d "!BestPP!" /f

REM ################################### START DISABLING ANOYING THINGS ####################################
REM ## Disable Sticky Keys ##
reg add "HKCU\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f
call "%~dp0Executables\DisableStickyKeys.exe"
reg add "HKEY_USERS\.DEFAULT\Control Panel\Accessibility\StickyKeys" /v "Flags" /t REG_SZ /d "506" /f

:END
exit /b

:ADDGPU
set exe=%~1
FOR /F "delims=" %%I IN ('where "!exe!" 2^>nul') DO (
IF "%%I" NEQ "" (
reg query "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "%%I" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (
reg add "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" /v "%%I" /t REG_SZ /d "GpuPreference=2;" /f >nul 2>&1
)
)
)
exit /b