param(
[Parameter(Mandatory=$true)][string]$EnableLowCPU,
[Parameter(Mandatory=$true)][string]$ScanAvg
)
if($EnableLowCPU -eq "TRUE") {
    $LowCPU = $true
}
else {
    $LowCPU = $false
}

# $ErrorActionPreference = 'SilentlyContinue'
Set-MpPreference -Force -EnableLowCpuPriority $LowCPU
Set-MpPreference -Force -ScanAvgCPULoadFactor $ScanAvg