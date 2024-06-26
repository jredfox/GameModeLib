param(
[Parameter(Mandatory=$true)][string]$EnableLowCPU,
[Parameter(Mandatory=$true)][string]$ScanAvg,
[Parameter(Mandatory=$true)][string]$PerfDrive
)
if ($EnableLowCPU -eq "TRUE") {
    $LowCPU = $true
}
elseif ($EnableLowCPU -eq "1") {
    $LowCPU = $true
}
else {
    $LowCPU = $false
}
if ($PerfDrive -eq "1") {
    $PerfDrive = "Enabled"
}
elseif ($PerfDrive -eq "TRUE") {
    $PerfDrive = "Enabled"
}
elseif ($PerfDrive -eq "Enabled") {
    $PerfDrive = "Enabled"
}
else {
    $PerfDrive = "Disabled"
}

$ErrorActionPreference = 'SilentlyContinue'
Set-MpPreference -Force -EnableLowCpuPriority $LowCPU
Set-MpPreference -Force -ScanAvgCPULoadFactor $ScanAvg
Set-MpPreference -Force -PerformanceModeStatus $PerfDrive