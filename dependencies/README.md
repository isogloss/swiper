# Dependencies

This directory is for external libraries required by the Swiper project.

## Required Libraries

### Dear ImGui (Required for GUI)

Dear ImGui is a bloat-free graphical user interface library for C++.

**Download:**
- GitHub: https://github.com/ocornut/imgui
- Download the latest release or clone the repository

**Installation:**
1. Download Dear ImGui source code
2. Create an `imgui` subdirectory here: `dependencies/imgui/`
3. Place the following files:
   ```
   dependencies/imgui/
   ├── imgui.h
   ├── imgui.cpp
   ├── imgui_draw.cpp
   ├── imgui_tables.cpp
   ├── imgui_widgets.cpp
   ├── imgui_internal.h
   ├── imconfig.h
   ├── imstb_rectpack.h
   ├── imstb_textedit.h
   ├── imstb_truetype.h
   └── backends/
       ├── imgui_impl_win32.h
       ├── imgui_impl_win32.cpp
       ├── imgui_impl_dx11.h
       └── imgui_impl_dx11.cpp
   ```

**Note:** CMake will automatically detect and use ImGui if present.

### MinHook (Recommended)

MinHook is a minimalistic x86/x64 API hooking library for Windows.

**Download:**
- GitHub: https://github.com/TsudaKageyu/minhook
- Download the latest release or clone the repository

**Installation:**
1. Download MinHook source code or pre-built binaries
2. Create a `minhook` subdirectory here: `dependencies/minhook/`
3. Place the following files:
   ```
   dependencies/minhook/
   ├── include/
   │   └── MinHook.h
   ├── lib/
   │   ├── libMinHook.x64.lib (for 64-bit)
   │   └── libMinHook.x86.lib (for 32-bit)
   └── bin/
       ├── MinHook.x64.dll (for 64-bit)
       └── MinHook.x86.dll (for 32-bit)
   ```

**Integration with CMake:**

CMake will automatically detect and link MinHook if it is found in the dependencies directory. No manual CMakeLists.txt editing is required.

## Alternative Hooking Libraries

### Microsoft Detours
- Commercial library (requires license for commercial use)
- Very stable and well-tested
- Download: https://github.com/microsoft/Detours

### PolyHook 2
- Modern C++ hooking library
- Supports x86/x64
- Download: https://github.com/stevemk14ebr/PolyHook_2_0

### EasyHook
- Open-source hooking library
- Primarily for .NET but has C++ support
- Download: https://easyhook.github.io/

## DirectX SDK

The DirectX headers are typically included with the Windows SDK. If you need to install them separately:

- Windows SDK 10: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
- DirectX SDK (legacy): https://www.microsoft.com/en-us/download/details.aspx?id=6812

## Build Notes

- This project targets x64 Windows only
- Ensure all libraries match the target architecture (x64)
- Visual Studio 2019 or later recommended
- Windows 10 SDK version 10.0.19041.0 or later

## License Compatibility

When adding dependencies, ensure their licenses are compatible with your project:

- **Dear ImGui**: MIT License (permissive)
- **MinHook**: BSD 2-Clause License (permissive)
- **Detours**: MIT License (commercial use requires separate agreement)
- **PolyHook**: MIT License (permissive)

Check the LICENSE file of each library before use.
