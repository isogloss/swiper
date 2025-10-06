# Swiper - DirectX Game Capture & Projection

## Project Overview

**Swiper** is a Windows-based C++ application that captures gameplay from other applications using DirectX hooking and displays it in a fullscreen projection window. The project consists of two main components:

1. **swiper.exe** - The main application that manages injection, IPC, and renders the projection window
2. **capture.dll** - A dynamic library that gets injected into the target game process to hook DirectX and capture frames

This enables users to capture and project gameplay to any monitor, useful for streaming, recording, or multi-monitor gaming setups.

## Architecture

### Two-Part Structure

#### 1. Main Application (`swiper.exe`)
The main executable is responsible for:
- User interface for process selection
- DLL injection into target game process
- Creating and managing the projection window
- Receiving captured frames via IPC
- Rendering frames in the borderless fullscreen window

#### 2. Capture DLL (`capture.dll`)
The capture library runs inside the target game's process and:
- Hooks DirectX Present functions (D3D9, D3D11, D3D12)
- Intercepts frame data at render time
- Copies frame buffers to shared memory
- Manages synchronization with the main application

### Game Capture Process Flow

```
1. User selects target game process in swiper.exe
2. swiper.exe injects capture.dll into target process
3. capture.dll hooks DirectX Present function
4. On each frame:
   a. Hook intercepts Present call
   b. Frame buffer is copied to shared memory
   c. Synchronization event signals new frame
   d. Original Present is called (game continues normally)
5. swiper.exe reads from shared memory
6. Frame is rendered in projection window
```

### Fullscreen Projection Process

```
1. swiper.exe creates a borderless window
2. Window can be positioned on any monitor
3. Render loop continuously:
   a. Waits for frame notification
   b. Reads frame data from shared memory
   c. Renders to DirectX/OpenGL surface
   d. Presents to window
```

## Component Breakdown

### swiper.exe Components

**main.cpp**
- Application entry point
- Window creation and management
- Process selection UI
- Render loop for projection window
- Frame rendering logic

**injector.h / injector.cpp**
- DLL injection implementation
- Process enumeration and selection
- Remote thread creation for injection
- Error handling and validation

**ipc.h**
- Shared header defining IPC structures
- Shared memory layout
- Synchronization primitives (events, mutexes)
- Frame metadata structures

### capture.dll Components

**dllmain.cpp**
- DLL entry point (DllMain)
- Initialization and cleanup
- Hook installation/removal
- IPC connection establishment

**hooking.h / hooking.cpp**
- DirectX API hooking logic
- Support for D3D9/D3D11/D3D12
- Present function hooks
- Frame buffer extraction
- Shared memory write operations

### IPC Channel

The application uses **Shared Memory** for high-performance inter-process communication:

**Structure:**
```cpp
struct SharedFrameData {
    uint32_t width;
    uint32_t height;
    uint32_t format;      // DXGI_FORMAT or D3DFMT
    uint32_t frameNumber;
    uint64_t timestamp;
    uint8_t pixelData[MAX_FRAME_SIZE];
};
```

**Synchronization:**
- Named Event for frame ready notification
- Mutex for shared memory access control
- Double buffering to prevent tearing

**Benefits:**
- Zero-copy frame transfer (memory mapping)
- Low latency (microseconds)
- No serialization overhead
- Direct memory access from both processes

## Build Instructions

### Prerequisites
- Windows 10/11
- CMake 3.15 or higher
- Visual Studio 2019 or higher (with C++ Desktop Development workload)
- DirectX SDK (included with Windows SDK)

### Building the Project

1. Clone the repository:
```bash
git clone https://github.com/isogloss/swiper.git
cd swiper
```

2. Generate build files:
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
```

3. Build the project:
```bash
cmake --build . --config Release
```

4. Output files will be in:
   - `build/bin/Release/swiper.exe`
   - `build/lib/Release/capture.dll`

### Alternative: Using Visual Studio directly
```bash
cmake .. -G "Visual Studio 16 2019" -A x64
```
Then open `Swiper.sln` in Visual Studio and build.

## Development Roadmap

### Step 1: Basic Window & UI ✓
- [x] Create main application window
- [x] Implement basic Win32 message loop
- [x] Add minimal UI for process selection
- [ ] Test window creation and event handling

### Step 2: DLL Injection
- [ ] Implement `CreateRemoteThread` injection method
- [ ] Add process enumeration (Toolhelp32 API)
- [ ] Validate target process architecture (x64 only)
- [ ] Error handling and logging
- [ ] Test injection with simple test application

### Step 3: Hooking Implementation
- [ ] Integrate hooking library (MinHook recommended)
- [ ] Detect DirectX version in target process
- [ ] Hook D3D11 Present function
- [ ] Hook D3D12 Present function
- [ ] Hook D3D9 Present function (optional)
- [ ] Implement hook trampoline functions
- [ ] Test hooks with sample DirectX applications

### Step 4: IPC with Shared Memory
- [ ] Define shared memory structure
- [ ] Implement shared memory creation (main app)
- [ ] Implement shared memory mapping (DLL)
- [ ] Add synchronization events
- [ ] Implement frame write (DLL side)
- [ ] Implement frame read (main app side)
- [ ] Test IPC throughput and latency

### Step 5: Frame Capture
- [ ] Extract texture from DirectX Present call
- [ ] Copy texture to CPU-accessible buffer
- [ ] Handle different texture formats
- [ ] Write frame data to shared memory
- [ ] Optimize for performance (GPU->CPU copy)
- [ ] Test with various games

### Step 6: Projection Window
- [ ] Create borderless window
- [ ] Initialize DirectX 11 rendering context
- [ ] Implement texture upload from shared memory
- [ ] Create render pipeline (vertex shader, pixel shader)
- [ ] Render texture to fullscreen quad
- [ ] Handle window resize and repositioning
- [ ] Add fullscreen toggle
- [ ] Test rendering performance

### Step 7: Polish & Optimization
- [ ] Add error messages and logging
- [ ] Implement graceful shutdown
- [ ] Optimize frame copy performance
- [ ] Add configuration file support
- [ ] Memory leak testing
- [ ] Multi-monitor support testing

## Dependencies

### Required Libraries

**MinHook** (Recommended for hooking)
- Minimalistic x86/x64 API hooking library
- Easy to integrate and use
- Repository: https://github.com/TsudaKageyu/minhook
- License: BSD 2-Clause

**Instructions:**
1. Download MinHook source or binary
2. Place in `dependencies/minhook/`
3. Update CMakeLists.txt to include MinHook

### Alternative Hooking Libraries
- **Microsoft Detours**: Commercial library (requires license)
- **EasyHook**: .NET-focused but has C++ support
- **PolyHook**: Modern C++ hooking library

### Windows SDK Components
- D3D11.h, D3D12.h (DirectX headers)
- dxgi.h (DirectX Graphics Infrastructure)
- Windows.h (Win32 API)

## Usage

1. Launch `swiper.exe`
2. Select target game process from list
3. Click "Inject & Capture"
4. Projection window will appear with captured game frames
5. Move window to desired monitor
6. Press F11 for fullscreen toggle
7. Press ESC to stop capture and close

## Technical Notes

### Performance Considerations
- Frame capture uses GPU->CPU copy (can be slow)
- Shared memory minimizes IPC overhead
- Double buffering prevents frame tearing
- Hook overhead: ~0.1-0.5ms per frame

### Compatibility
- Windows 10/11 (x64 only)
- DirectX 11/12 games (D3D9 optional)
- Anti-cheat may block injection
- Admin rights required for injection

### Security
- Code runs in game process (kernel-level anti-cheat may detect)
- Use only with games you own
- Not intended for online competitive games

## Troubleshooting

**DLL Injection Fails**
- Run swiper.exe as Administrator
- Check if target is 64-bit process
- Verify capture.dll is in same directory

**Black Screen in Projection Window**
- Check if game uses DirectX 11/12
- Verify shared memory is created
- Check Windows Event Viewer for errors

**Low FPS / Stuttering**
- GPU->CPU copy is expensive
- Reduce game resolution
- Close other GPU-intensive applications

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Please follow the development roadmap and maintain code quality.

1. Fork the repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Create Pull Request

## Disclaimer

This tool is for educational purposes. Use responsibly and only with software you have the right to modify. The authors are not responsible for any misuse or violations of terms of service.
