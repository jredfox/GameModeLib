$ErrorActionPreference = 'SilentlyContinue'
Set-MpPreference -Force -EnableLowCpuPriority $false
Set-MpPreference -Force -ScanAvgCPULoadFactor 50
set-MpPreference -Force -PerformanceModeStatus Enabled