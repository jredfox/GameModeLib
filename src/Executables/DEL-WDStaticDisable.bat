@ECHO OFF
setlocal enableDelayedExpansion
schtasks /DELETE /tn "WDStaticDisabler" /F
