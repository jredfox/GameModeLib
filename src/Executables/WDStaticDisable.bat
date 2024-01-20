@ECHO OFF
setlocal enableDelayedExpansion
REM ## Create Admin Startup of itself ##
schtasks /create /tn "WDStaticDisabler" /ru system /sc onstart /tr "%~f0" /F
REM ## Disables Windows Defender Notfications ##
reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows Defender Security Center\Notifications" /v "DisableNotifications" /t REG_DWORD /d 1 /f
reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows Defender Security Center\Notifications" /v "DisableEnhancedNotifications" /t REG_DWORD /d 1 /f
reg add "HKLM\SOFTWARE\Microsoft\Windows Defender Security Center\Notifications" /v "DisableNotifications" /t REG_DWORD /d 1 /f
reg add "HKLM\SOFTWARE\Microsoft\Windows Defender Security Center\Notifications" /v "DisableEnhancedNotifications" /t REG_DWORD /d 1 /f
REM ## Disable Windows Defender Except Firewall ##
powershell -ExecutionPolicy Bypass -File "%~dp0WDDisable.ps1"