#ifndef IPC_H
#define IPC_H

#include <Windows.h>
#include <cstdint>

// Maximum frame size for shared memory (1920x1080x4 bytes for RGBA)
constexpr size_t MAX_FRAME_SIZE = 1920 * 1080 * 4;

// Shared memory structure for frame data
struct SharedFrameData {
    uint32_t width;
    uint32_t height;
    uint32_t format;        // DXGI_FORMAT or D3DFMT
    uint32_t frameNumber;
    uint64_t timestamp;
    bool isValid;
    uint8_t pixelData[MAX_FRAME_SIZE];
};

// Names for shared memory objects
constexpr const wchar_t* SHARED_MEMORY_NAME = L"Global\\SwiperSharedMemory";
constexpr const wchar_t* FRAME_READY_EVENT_NAME = L"Global\\SwiperFrameReady";
constexpr const wchar_t* MEMORY_MUTEX_NAME = L"Global\\SwiperMemoryMutex";

#endif // IPC_H
