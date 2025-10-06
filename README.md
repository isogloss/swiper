# Swiper - Game Capture & Projection Application

## Project Overview

**Swiper** is a Windows-based C++ application designed to capture gameplay from other applications using DirectX hooking and display it in a fullscreen projection window. The application consists of two main components working together: a main executable (`swiper.exe`) for managing the UI and projection window, and a capture library (`capture.dll`) that hooks into the target game process to intercept rendered frames.

This project is ideal for streaming setups, multi-monitor gaming configurations, or any scenario where you need to capture and display DirectX-rendered content from another application.

## Architecture

### Two-Part Structure

1. **swiper.exe** (Main Application)
   - User interface for process selection and control
   - DLL injection engine
   - Fullscreen projection window with DirectX rendering
   - IPC receiver for captured frames

2. **capture.dll** (Capture Library)
   - Runs inside the target game process
   - DirectX API hooking (D3D9/D3D11/D3D12)
   - Frame capture and transmission via IPC

### Game Capture Flow

```
1. User selects target game process
2. swiper.exe injects capture.dll into game process
3. capture.dll hooks DirectX Present() functions
4. On each frame:
   - Hook intercepts Present() call
   - Frame data is copied from GPU/back buffer
   - Frame is written to shared memory
   - Original Present() is called (game continues normally)
5. swiper.exe reads frames from shared memory
6. Frames are rendered in projection window
```

### IPC Architecture (Shared Memory)

To achieve high-performance frame transfer between the game process and swiper.exe, we use **shared memory**:

- **Shared Memory Buffer**: A named shared memory region stores the latest captured frame
- **Synchronization**: Mutex or semaphore objects ensure thread-safe access
- **Zero-Copy Design**: Frame data is written once by the DLL and read once by the main app
- **Performance**: Capable of handling 60+ FPS with minimal latency

**Memory Layout:**
```
[Frame Header: width, height, format, timestamp]
[Frame Data: RGB/RGBA pixel buffer]
```

## Component Breakdown

### swiper.exe (Main Application)

**Responsibilities:**
- Create and manage the main application window
- Provide UI for selecting target game processes
- Implement DLL injection logic to load capture.dll into the target process
- Create a borderless fullscreen projection window
- Set up IPC shared memory region
- Continuously read captured frames from shared memory
- Render frames using DirectX (D3D11 recommended)

**Key Files:**
- `main.cpp`: Entry point, window creation, main loop
- `injector.h/cpp`: DLL injection implementation
- `ipc.h`: Shared memory structures and constants

### capture.dll (Capture Library)

**Responsibilities:**
- Hook DirectX API functions (Present, ResizeBuffers, etc.)
- Capture frame data from the back buffer
- Write captured frames to shared memory
- Minimal performance impact on the target game

**Key Files:**
- `dllmain.cpp`: DLL entry point, initialization/cleanup
- `hooking.h/cpp`: DirectX hooking implementation

### IPC Channel (Shared Memory)

**Design:**
- Named shared memory object (e.g., "Global\\SwiperCapture")
- Fixed-size buffer for single frame (e.g., 1920x1080x4 = ~8MB)
- Header with metadata (resolution, format, frame counter)
- Synchronization primitives for thread safety

**Advantages:**
- Extremely fast (no kernel transitions for data transfer)
- Low CPU overhead
- Suitable for real-time video streaming

## Build Instructions

### Prerequisites

- **Windows 10/11** (required for DirectX support)
- **Visual Studio 2019 or later** (with C++ desktop development workload)
- **CMake 3.15+**
- **DirectX SDK** (included with Windows SDK in modern VS installations)

### Building the Project

1. **Clone the repository:**
   ```bash
   git clone https://github.com/isogloss/swiper.git
   cd swiper
   ```

2. **Configure with CMake:**
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```

3. **Build:**
   ```bash
   cmake --build . --config Release
   ```

   Or open `swiper.sln` in Visual Studio and build from the IDE.

4. **Output:**
   - `build/Release/swiper.exe`
   - `build/Release/capture.dll`

### Running the Application

1. Launch `swiper.exe`
2. Select a target game process from the UI
3. Click "Start Capture"
4. The projection window will display the captured game output

**Note:** Administrator privileges may be required to inject into some processes.

## Development Roadmap

### Step 1: Basic Window & UI ✓
- Create main application window with basic controls
- Implement process enumeration and selection
- Set up event loop and message handling

### Step 2: DLL Injection
- Implement `CreateRemoteThread` or similar injection technique
- Handle 32-bit vs 64-bit process injection
- Error handling and process privilege management

### Step 3: Hooking Implementation
- Integrate hooking library (MinHook recommended)
- Hook D3D11 `IDXGISwapChain::Present`
- Hook D3D9 `IDirect3DDevice9::Present`
- Hook D3D12 present functions
- Implement hook trampoline and original function calling

### Step 4: IPC with Shared Memory
- Create named shared memory region
- Define frame data structure in `ipc.h`
- Implement producer (DLL) and consumer (main app) logic
- Add mutex/semaphore synchronization

### Step 5: Projection Window
- Create borderless fullscreen window
- Initialize DirectX renderer in swiper.exe
- Implement texture upload from shared memory
- Render loop with VSync control

### Future Enhancements
- Multi-monitor support with window positioning
- Frame rate limiting and performance metrics
- Audio capture support
- Recording functionality
- Compression for remote streaming

## Dependencies

### Required Libraries

1. **MinHook** (Recommended hooking library)
   - URL: https://github.com/TsudaKageyu/minhook
   - License: BSD 2-Clause
   - Purpose: Function hooking for DirectX API interception
   - Integration: Add to `dependencies/` folder or use as a git submodule

2. **Windows API**
   - DirectX 11 (d3d11.lib, dxgi.lib)
   - DirectX 9 (d3d9.lib) - optional
   - DirectX 12 (d3d12.lib, dxgi.lib) - optional

3. **Standard Libraries**
   - User32.lib, Kernel32.lib, etc.

### Adding MinHook

**Option 1: Manual Download**
```bash
cd dependencies
git clone https://github.com/TsudaKageyu/minhook.git
```

**Option 2: Git Submodule**
```bash
git submodule add https://github.com/TsudaKageyu/minhook.git dependencies/minhook
git submodule update --init --recursive
```

Then update `src/capture_dll/CMakeLists.txt` to link against MinHook.

See `dependencies/README.md` for detailed instructions.

## Technical Notes

### DirectX Version Support

- **DirectX 11**: Primary target, most common in modern games
- **DirectX 9**: Legacy support for older games
- **DirectX 12**: Future support, more complex due to low-level API

### Performance Considerations

- Frame capture should take < 1ms per frame
- Shared memory copy should be optimized (consider GPU staging buffers)
- Minimize allocations in hot paths
- Consider asynchronous capture if frame rate drops

### Security & Anti-Cheat

**Warning:** DLL injection and API hooking may trigger anti-cheat systems in online games. This tool is intended for:
- Single-player games
- Games with permission for capture tools
- Development and testing purposes

**Do not use this tool with online games that prohibit such modifications.**

## Contributing

Contributions are welcome! Please follow these guidelines:
- Follow the existing code style
- Test your changes on multiple games/scenarios
- Update documentation for new features
- Submit pull requests with clear descriptions

## License

This project is provided as-is for educational purposes. Users are responsible for ensuring compliance with game EULAs and anti-cheat policies.

## Acknowledgments

- MinHook library by Tsuda Kageyu
- DirectX documentation by Microsoft
- Inspiration from OBS Studio's game capture functionality
