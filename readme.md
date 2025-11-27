# Lycheepad - Windows CE Palm-size PC Launcher

## Overview

Lycheepad is a custom application launcher designed specifically for legacy Windows CE Palm-size PC 2.01 devices. It provides an icon-based interface for quickly launching applications on these classic handheld devices.

## Features

- **Icon Grid Interface**: 12 application slots arranged in a 3x4 grid layout
- **Customizable Slots**: Each slot can be configured with:
  - Application path (.exe file)
  - Custom label text
  - Application icon
- **Configuration Persistence**: Save and load application configurations to/from file
- **Single Instance**: Prevents multiple instances from running simultaneously
- **Windows CE Compatible**: Built specifically for Windows CE 2.01 platform

## Technical Details

- **Language**: C Language
- **API**: Win32 API
- **Platform**: Windows CE 2.01 (Palm-size PC).

## Build Instructions

### Prerequisites
- Microsoft Visual C++ 6.0
- Windows CE Toolkit for Visual C++ 6.0
- Target Platform: Windows CE 2.01 (Palm-size PC)

### Project Files
- `LycheeMain.c` - Main application window and logic
- `DialogAppList.c` - Application selection dialog
- `DialogFolderViewer.c` - Folder browser dialog
- `DialogSelectExe.c` - Executable selection dialog
- `Utils.c` - Utility functions
- `Lycheepad.rc` - Resource file containing dialogs and menus
- `resource.h` - Resource definitions

## Usage

1. **Adding Applications**: Use the "App List" menu option to browse and select applications
2. **Configuration**: The application automatically manages configuration files
3. **Launching**: Click any configured slot to launch the associated application

## Compatibility

This application is specifically designed for:
- Windows CE 2.01
- Palm-size PC form factor
- 240x320 screen resolution (typical for these devices)

## Notes

- The application uses Windows CE-specific APIs and may not run on desktop Windows
- Memory management is optimized for the limited resources of Palm-size PC devices
- The interface is designed for touchscreen input with stylus
