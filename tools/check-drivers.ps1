<#

.SYNOPSIS
This checks for the presence of any XDP drivers currently loaded.

#>

[cmdletbinding()]Param(
    [Parameter(Mandatory = $false)]
    [switch]$IgnoreEbpf = $false
)

Set-StrictMode -Version 'Latest'
$ErrorActionPreference = 'Stop'

# Important paths.
$RootDir = Split-Path $PSScriptRoot -Parent

# Cache driverquery output.
$AllDrivers = driverquery /v /fo list

# Checks for the presence of a loaded driver.
function Check-Driver($Driver) {
    $Found = $false
    try {
        $Found = ($AllDrivers | Select-String $Driver).Matches[0].Success
    } catch { }
    $Found
}

# Checks for the presence of a loaded driver and attempts to uninstall it.
function Check-And-Remove-Driver($Driver, $Component) {
    if (Check-Driver $Driver) {
        Write-Host "Detected $Driver is loaded. Uninstalling $Component..."
        & $RootDir\tools\setup.ps1 -Uninstall $Component

        # Update cached driverquery output.
        $AllDrivers = driverquery /v /fo list
    }

    if (Check-Driver $Driver) {
        $AllDrivers | Write-Verbose
        Write-Error "$Driver loaded!"
    }
}

# Check for any XDP drivers.
Check-And-Remove-Driver "xdpfnmp.sys" "xdpfnmp"
Check-And-Remove-Driver "xdpfnlwf.sys" "xdpfnlwf"
Check-And-Remove-Driver "xdpmp.sys" "xdpmp"
Check-And-Remove-Driver "xdp.sys" "xdp"
Check-And-Remove-Driver "fndis.sys" "fndis"

# Check for any eBPF drivers.
if (!$IgnoreEbpf) {
    Check-And-Remove-Driver "ebpfcore.sys" "ebpf"
    Check-And-Remove-Driver "netebpfext.sys" "ebpf"
}

# Yay! No XDP drivers found.
Write-Host "No loaded XDP drivers found!"

# Log driver verifier status
verifier.exe /query
