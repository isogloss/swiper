# Dependencies

This directory contains third-party libraries required by the Swiper project.

## Required Dependencies

### MinHook

MinHook is a minimalistic x86/x64 API hooking library for Windows.

- **Repository**: https://github.com/TsudaKageyu/minhook
- **License**: BSD 2-Clause License
- **Purpose**: Used to hook DirectX API functions for game capture

#### Installation Instructions

**Option 1: Git Submodule (Recommended)**

```bash
cd /path/to/swiper
git submodule add https://github.com/TsudaKageyu/minhook.git dependencies/minhook
git submodule update --init --recursive
```

**Option 2: Manual Download**

1. Download MinHook from: https://github.com/TsudaKageyu/minhook/releases
2. Extract to `dependencies/minhook/`
3. Ensure the following structure exists:
   ```
   dependencies/minhook/
   ├── include/
   │   └── MinHook.h
   └── src/
       └── (source files)
   ```

**Option 3: Build from Source**

```bash
cd dependencies
git clone https://github.com/TsudaKageyu/minhook.git
cd minhook
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

#### Integration with CMake

Once MinHook is added to the `dependencies/` folder, uncomment the following lines in `src/capture_dll/CMakeLists.txt`:

```cmake
target_include_directories(capture_dll PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/minhook/include)
target_link_libraries(capture_dll PRIVATE minhook)
```

You may also need to add MinHook as a subdirectory in the root `CMakeLists.txt` or link against the pre-built library.

## Future Dependencies

### DirectX SDK

The DirectX SDK is included with modern Windows SDK installations (Windows 10 SDK and later). If you're using Visual Studio 2019 or later with the C++ desktop development workload, you should already have the necessary headers and libraries.

If you encounter missing DirectX headers:
1. Install the Windows SDK from Visual Studio Installer
2. Or download the legacy DirectX SDK from Microsoft

### Optional: Microsoft Detours (Alternative to MinHook)

If you prefer to use Microsoft Detours instead of MinHook:

- **Website**: https://github.com/microsoft/Detours
- **License**: MIT License
- **Note**: Requires separate license for commercial use

## Verifying Dependencies

After adding MinHook, verify the directory structure:

```
dependencies/
├── README.md
└── minhook/
    ├── include/
    │   └── MinHook.h
    ├── src/
    └── CMakeLists.txt (if available)
```

## Troubleshooting

### MinHook not found during build

- Ensure the `minhook` directory exists in `dependencies/`
- Check that `MinHook.h` is present in `dependencies/minhook/include/`
- Verify CMake configuration with `cmake .. -DCMAKE_PREFIX_PATH=/path/to/dependencies`

### Linking errors

- Make sure you're building for the correct architecture (x64 vs x86)
- Verify that MinHook is built for the same architecture as your project
- Check that the MinHook library file exists in the expected location

## Contributing

When adding new dependencies:
1. Document them in this file
2. Provide clear installation instructions
3. Include license information
4. Update the root CMakeLists.txt if necessary
