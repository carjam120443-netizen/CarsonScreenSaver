# CarsonScreenSaver

A custom Windows screensaver project built in C.

## Screensavers

The project provides two separate selectable screensavers:

- **CarsonScreenSaver.scr** — Windows 7 Aero-inspired bubbles and glass effect.
- **CarsonTuxScreenSaver.scr** — animated Tux-themed penguin.

Both are separate `.scr` programs. After installation, Windows can list them independently in Screen Saver Settings.

## Install

Download `CarsonScreenSavers.zip` from GitHub Actions and extract it.

Run `Install-CarsonScreenSavers.cmd` as Administrator. It copies both `.scr` files into `%WINDIR%\\System32`, then opens the Windows Screen Saver Settings dialog.

After installation, open:

**Settings → Personalization → Lock screen → Screen saver**

You should be able to select either Carson screensaver from the **Screen saver** dropdown.

Windows uses the `SCRNSAVE.EXE` value under `HKCU\\Control Panel\\Desktop` for the selected screen saver.

## Build locally

Requires CMake and a Windows C compiler such as MSVC.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Generated files:

```text
build/Release/CarsonScreenSaver.scr
build/Release/CarsonTuxScreenSaver.scr
```

## GitHub Actions

The workflow builds both screensavers and packages them into `CarsonScreenSavers.zip`.

The ZIP contains:

```text
CarsonScreenSaver.scr
CarsonTuxScreenSaver.scr
Install-CarsonScreenSavers.ps1
Install-CarsonScreenSavers.cmd
```