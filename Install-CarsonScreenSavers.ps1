# Install both CarsonScreenSaver variants into the Windows screen saver directory.
# Run this script as Administrator.

$ErrorActionPreference = "Stop"
$sourceDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$targetDir = Join-Path $env:WINDIR "System32"

if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Host "Please run this script as Administrator." -ForegroundColor Yellow
    exit 1
}

$files = @("CarsonScreenSaver.scr", "CarsonTuxScreenSaver.scr")
foreach ($file in $files) {
    $source = Join-Path $sourceDir $file
    if (-not (Test-Path $source)) { throw "Missing $file in $sourceDir" }
    Copy-Item $source (Join-Path $targetDir $file) -Force
    Write-Host "Installed $file"
}

Write-Host ""
Write-Host "Both Carson screensavers are installed." -ForegroundColor Green
Start-Process "control.exe" -ArgumentList "desk.cpl,,@screensaver"
