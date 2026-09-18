# Install CarsonScreenSaver v2 screensavers.
# Run this script as Administrator.

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$system32 = Join-Path $env:WINDIR "System32"

$screensavers = @(
    "CarsonScreenSaver.scr",
    "CarsonTuxScreenSaver.scr",
    "CarsonWindows7ScreenSaver.scr"
)

foreach ($file in $screensavers) {
    $source = Join-Path $scriptDir $file
    if (-not (Test-Path $source)) { throw "Missing screensaver: $file" }
    Copy-Item $source (Join-Path $system32 $file) -Force
}

Start-Process control.exe "desk.cpl,,@screensaver"
