@ECHO OFF
setlocal enableDelayedExpansion
REM ## Get the Current AC or DC PowerModeOverlay ##
FOR /F "tokens=3*" %%A IN ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActivePowerScheme"') DO (set PrevPowerPlan=%%A)
FOR /F "tokens=3*" %%A IN ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActiveOverlayAcPowerScheme"') DO (set PrevPPMode=%%A)
set gm=b8e6d75e-26e8-5e8f-efef-e94a209a3467
IF "!PrevPowerPlan!" EQU "!gm!" (set PrevPowerPlan=381b4222-f694-41f0-9685-ff5bb260df2e)
REM Generate uninstalls to GameModeUninstall.bat
(
	echo ^@ECHO OFF
	echo setlocal enableDelayedExpansion
	call :QUERY "HKCU\Software\Microsoft\GameBar" "AllowAutoGameMode"
	call :QUERY "HKCU\Software\Microsoft\GameBar" "AutoGameModeEnabled"
	call :QUERY "HKLM\SYSTEM\CurrentControlSet\Control\GraphicsDrivers" "HwSchMode"
	call :QUERY "HKCU\SOFTWARE\Microsoft\DirectX\UserGpuPreferences" "DirectXUserGlobalSettings"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\HDR" "UseHDR"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\HDR" "AllowHDRForGames"
	call :QUERY "HKCU\System\GameConfigStore" "GameDVR_StreamingEnabled"
	call :QUERY "HKCU\System\GameConfigStore" "GameDVR_Enabled"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "AllowLowResolution"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "DecreaseResolutionOnBattery"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "DisableAutoLightingOnBattery"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "EnableHDRForPlayback"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "HDColorQualityOnBattery"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "VideoQualityOnBattery"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "DisableOtherEnhancementsOnBattery"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "DisableHDROnBattery"
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\VideoSettings" "DisableHDRSupportOnBattery"
	echo powercfg /SETACTIVE "!PrevPowerPlan!"
	echo If %%ERRORLEVEL%% NEQ 0 ^(powercfg /SETACTIVE "381b4222-f694-41f0-9685-ff5bb260df2e"^)
	echo powercfg /DELETE "!gm!"
	echo call "%%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes"
	echo call "%%~dp0Executables\PowerModeOverlay.exe" "!PrevPPMode!"
	call :QUERY "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" "ActiveOverlayAcPowerScheme"
	call :QUERY "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" "ActiveOverlayDcPowerScheme"
	call :QUERY "HKCU\SOFTWARE\Intel\Display\igfxcui\3D" "Default"
	call :QUERY "HKEY_CURRENT_USER\Control Panel\Accessibility\StickyKeys" "Flags"
	echo call "%%~dp0Executables\StickyKeysSetFlag.exe" "!datval!"
	call :QUERY "HKEY_USERS\.DEFAULT\Control Panel\Accessibility\StickyKeys" "Flags"
	IF /I "%WDLowCPU:~0,1%" EQU "T" (
		echo call "%%~dp0Executables\CheckTamper.bat"
		echo powershell Set-MpPreference -Force -EnableLowCpuPriority ^^$false ^>nul 2^>^&1
		call :GETLOADFAC
		echo powershell Set-MpPreference -Force -ScanAvgCPULoadFactor !loadfac! ^>nul 2^>^&1
	)
	IF /I "%WDFullDisable:~0,1%" EQU "T" (
		echo call "%%~dp0Executables\CheckTamper.bat"
		echo call "%%~dp0Executables\WDStaticEnable.bat"
		call :QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows Defender Security Center\Notifications" "DisableNotifications"
		call :QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows Defender Security Center\Notifications" "DisableEnhancedNotifications"
		call :QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows Defender Security Center\Notifications" "DisableNotifications"
		call :QUERY "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows Defender Security Center\Notifications" "DisableEnhancedNotifications"
	)
	echo schtasks /DELETE /tn "WDStaticDisabler" /F
	REM ## Start Mouse and Keyboard ##
	call :QUERY "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\PrecisionTouchPad" "AAPThreshold"
	REM TODO GET OEM Generation of defaults
	REM ## Start Cleanup ##
	echo call "%%~dp0GameModeUninstallGPU.bat"
	echo del /F /Q /A "%%~dp0GameModeUninstallGPU.bat"
	echo ^(goto^) 2^>nul ^& del /F /Q /A "%%~f0" ^& title %%CD%% ^& cmd /c exit /b
)>"%~dp0GameModeUninstall.bat"
REM ## Generate the GPU Preferences Uninstall Script ##
(
	echo ^@ECHO OFF
	echo setlocal enableDelayedExpansion
)>"%~dp0GameModeUninstallGPU.bat"
exit /b

:QUERY
set key=%~1
set val=%~2
reg query "%key%" /v "%val%" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (
echo reg delete "%key%" /v "%val%" /f
exit /b
)
FOR /F "delims=" %%A IN ('reg query "%key%" /v "%val%"') DO (
IF "%%A" NEQ "%key%" (
For /F "tokens=2,3*" %%B IN ("%%A") DO (
	echo reg add "%key%" /v "%val%" /t %%B /d "%%C" /f
	set datval=%%C
)
)
)
exit /b

:GETLOADFAC
FOR /F "delims=" %%I IN ('powershell "Get-MpPreference | select ScanAvgCPULoadFactor"') DO (
set l=%%I
set l=!l: =!
IF "!l!" NEQ "" (set loadfac=!l!)
)
exit /b