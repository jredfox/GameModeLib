$ErrorActionPreference = 'SilentlyContinue'
Set-MpPreference -Force -DisableRealtimeMonitoring $true
Set-MpPreference -Force -MAPSReporting Disabled
Set-MpPreference -Force -SubmitSamplesConsent NeverSend