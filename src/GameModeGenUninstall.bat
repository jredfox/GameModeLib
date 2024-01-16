@ECHO OFF
setlocal enableDelayedExpansion
REM ## Get the Current AC or DC PowerModeOverlay ##
FOR /F "tokens=3*" %%A IN ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActivePowerScheme"') DO (set PrevPowerPlan=%%A)
powercfg /SETACTIVE "381b4222-f694-41f0-9685-ff5bb260df2e"
cscript /NOLOGO "%~dp0Executables\Sleep.vbs" "250"
FOR /F "tokens=1*" %%A IN ('call "%~dp0Executables\PowerModeOverlay.exe" "GET"') DO (set PrevPPMode=%%A)
powercfg /SETACTIVE "!PrevPowerPlan!"
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
	echo powercfg /DELETE "b8e6d75e-26e8-5e8f-efef-e94a209a3467"
	echo call "%%~dp0Executables\PowerModeOverlay.exe" "!PrevPPMode!"
	echo call "%%~dp0Executables\RegGrant.exe" "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes"
	call :QUERY "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" "ActiveOverlayAcPowerScheme"
	call :QUERY "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" "ActiveOverlayDcPowerScheme"
	call :QUERY "HKCU\Control Panel\Accessibility\StickyKeys" "Flags"
	echo call "%%~dp0Executables\StickyKeysSetFlag.exe" "!datval!"
	call :QUERY "HKEY_USERS\.DEFAULT\Control Panel\Accessibility\StickyKeys" "Flags"
)>"%~dp0GameModeUninstall.bat"
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