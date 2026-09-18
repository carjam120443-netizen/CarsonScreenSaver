# CarsonScreenSaver

A custom Windows screensaver project built in C.

## Screensavers

The project provides three separate selectable screensavers:

- CarsonScreenSaver.scr — Windows 7 Aero-inspired bubbles and glass effect.
- CarsonTuxScreenSaver.scr — animated Tux-themed penguin.
- CarsonWindows7ScreenSaver.scr — animated Windows 7-style four-color logo.

The Windows 7-style screensaver draws the logo programmatically, so the repository does not need to ship a copied image asset. It uses the same per-monitor design as the other screensavers, so normal mode displays independently across multiple monitors.

All three are separate .scr programs. After installation, Windows can list them independently in Screen Saver Settings.

## Install

Download CarsonScreenSavers.zip from GitHub Actions and extract it.

For the current three-screensaver package, run Install-CarsonScreenSavers-v2.cmd as Administrator. It copies all three .scr files into %WINDIR%\System32, then opens the Windows Screen Saver Settings dialog.

The original Install-CarsonScreenSavers.cmd / .ps1 pair is intentionally preserved and still installs the original two screensavers.

After installation, open:

Settings → Personalization → Lock screen → Screen saver

You should be able to select any of the Carson screensavers from the Screen saver dropdown.

## Build locally

Requires CMake and a Windows C compiler such as MSVC.

    cmake -S . -B build -A x64
    cmake --build build --config Release

Generated files:

    build/Release/CarsonScreenSaver.scr
    build/Release/CarsonTuxScreenSaver.scr
    build/Release/CarsonWindows7ScreenSaver.scr

## GitHub Actions

The workflow builds all three screensavers and packages them into CarsonScreenSavers.zip.

The ZIP contains:

    CarsonScreenSaver.scr
    CarsonTuxScreenSaver.scr
    CarsonWindows7ScreenSaver.scr
    Install-CarsonScreenSavers.ps1
    Install-CarsonScreenSavers.cmd
    Install-CarsonScreenSavers-v2.ps1
    Install-CarsonScreenSavers-v2.cmd
