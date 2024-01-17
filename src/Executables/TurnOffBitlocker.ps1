param(
[Parameter(Mandatory=$true)][string]$Drive,
[Parameter(Mandatory=$true)][string]$Password
)
$SecureString = ConvertTo-SecureString $Password -AsPlainText -Force
Unlock-BitLocker -MountPoint $Drive -Password $SecureString
Disable-BitLocker -MountPoint $Drive