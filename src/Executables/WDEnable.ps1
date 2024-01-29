$ErrorActionPreference = 'SilentlyContinue'
Set-MpPreference -Force -DisableRealtimeMonitoring $false
Set-MpPreference -Force -MAPSReporting Advanced
Set-MpPreference -Force -SubmitSamplesConsent Always
Set-MpPreference -Force -SubmitSamplesConsent SendSafeSamples