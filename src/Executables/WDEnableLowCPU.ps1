$ErrorActionPreference = 'SilentlyContinue'
Set-MpPreference -Force -EnableLowCpuPriority $true
Set-MpPreference -Force -ScanAvgCPULoadFactor 15