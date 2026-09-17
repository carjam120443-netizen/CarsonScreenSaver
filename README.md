# CarsonScreenSaver

A custom Windows screensaver project built in C.

## Current starter

The project currently contains a minimal screensaver that:

- Builds as a Windows GUI executable and .scr file.
- Supports /s for screensaver mode.
- Supports /c for configuration.
- Supports /p <HWND> for Windows preview mode.
- Is ready to grow into multiple screensavers and visual modes.

## Build locally

Requires CMake and a Windows C compiler such as MSVC.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

The generated screensaver is:

```
build/Release/CarsonScreenSaver.scr
```

## Planned

More screensaver modes, effects, configuration options, assets, and polished Windows integration can be added over time.
