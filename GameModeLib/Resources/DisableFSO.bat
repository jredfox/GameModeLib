@ECHO OFF
setlocal enableDelayedExpansion
set file=%USERPROFILE%\DisableFSO
IF NOT EXIST "%file%" (
echo FirstRun >"%file%"
echo Please Re-Login or Reboot to Disable Full Screen Optimizations
pause
exit /b
)

reg add "HKEY_CURRENT_USER\System\GameConfigStore" /v "GameDVR_FSEBehavior" /t REG_DWORD /d 2 /f
reg add "HKEY_CURRENT_USER\System\GameConfigStore" /v "GameDVR_FSEBehaviorMode" /t REG_DWORD /d 2 /f
reg add "HKEY_CURRENT_USER\System\GameConfigStore" /v "GameDVR_HonorUserFSEBehaviorMode" /t REG_DWORD /d 1 /f
reg add "HKEY_CURRENT_USER\System\GameConfigStore" /v "GameDVR_DXGIHonorFSEWindowsCompatible" /t REG_DWORD /d 1 /f
reg add "HKEY_CURRENT_USER\System\GameConfigStore" /v "GameDVR_EFSEFeatureFlags" /t REG_DWORD /d 0 /f
reg add "HKEY_CURRENT_USER\System\GameConfigStore" /v "GameDVR_DSEBehavior" /t REG_DWORD /d 2 /f
reg delete "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run" /v "GMLDisableFSO" /f
echo Full Screen Optimizations Disabled. Please Re-Login Reboot For it to Take Effect
pause
