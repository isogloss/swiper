#pragma once

#include <cstdint>
#include <windows.h>

namespace Swiper {

// Shared memory configuration
constexpr const char* SHARED_MEMORY_NAME = "Global\\SwiperCaptureBuffer";
constexpr size_t MAX_FRAME_WIDTH = 3840;  // 4K width
constexpr size_t MAX_FRAME_HEIGHT = 2160; // 4K height
constexpr size_t MAX_FRAME_SIZE = MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 4; // RGBA

// Frame format enum
enum class FrameFormat : uint32_t {
    UNKNOWN = 0,
    BGRA8 = 1,  // Most common format from DirectX
    RGBA8 = 2,
    RGB8 = 3
};

// Frame header structure
struct FrameHeader {
    uint32_t width;
    uint32_t height;
    FrameFormat format;
    uint64_t timestamp;      // System time when captured
    uint64_t frameNumber;    // Sequential frame counter
    uint32_t dataSize;       // Size of frame data in bytes
    uint32_t padding;        // Alignment padding
};

// Shared memory layout
struct SharedMemoryData {
    FrameHeader header;
    uint8_t frameData[MAX_FRAME_SIZE];
};

// Synchronization objects
constexpr const char* MUTEX_NAME = "Global\\SwiperCaptureMutex";
constexpr const char* EVENT_NEW_FRAME = "Global\\SwiperNewFrameEvent";

// Calculate total shared memory size
constexpr size_t SHARED_MEMORY_SIZE = sizeof(SharedMemoryData);

// Helper class for shared memory management
class SharedMemoryManager {
public:
    SharedMemoryManager() 
        : hMapFile(nullptr)
        , pBuffer(nullptr)
        , hMutex(nullptr)
        , hNewFrameEvent(nullptr)
    {}

    ~SharedMemoryManager() {
        Close();
    }

    // Create shared memory (for main application)
    bool Create() {
        hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            0,
            static_cast<DWORD>(SHARED_MEMORY_SIZE),
            SHARED_MEMORY_NAME
        );

        if (hMapFile == nullptr) {
            return false;
        }

        pBuffer = static_cast<SharedMemoryData*>(MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            SHARED_MEMORY_SIZE
        ));

        if (pBuffer == nullptr) {
            CloseHandle(hMapFile);
            hMapFile = nullptr;
            return false;
        }

        // Create synchronization objects
        hMutex = CreateMutexA(nullptr, FALSE, MUTEX_NAME);
        hNewFrameEvent = CreateEventA(nullptr, FALSE, FALSE, EVENT_NEW_FRAME);

        // Initialize header
        ZeroMemory(pBuffer, sizeof(FrameHeader));

        return true;
    }

    // Open existing shared memory (for capture DLL)
    bool Open() {
        hMapFile = OpenFileMappingA(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            SHARED_MEMORY_NAME
        );

        if (hMapFile == nullptr) {
            return false;
        }

        pBuffer = static_cast<SharedMemoryData*>(MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            SHARED_MEMORY_SIZE
        ));

        if (pBuffer == nullptr) {
            CloseHandle(hMapFile);
            hMapFile = nullptr;
            return false;
        }

        // Open synchronization objects
        hMutex = OpenMutexA(SYNCHRONIZE, FALSE, MUTEX_NAME);
        hNewFrameEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, EVENT_NEW_FRAME);

        return true;
    }

    void Close() {
        if (pBuffer != nullptr) {
            UnmapViewOfFile(pBuffer);
            pBuffer = nullptr;
        }

        if (hMapFile != nullptr) {
            CloseHandle(hMapFile);
            hMapFile = nullptr;
        }

        if (hMutex != nullptr) {
            CloseHandle(hMutex);
            hMutex = nullptr;
        }

        if (hNewFrameEvent != nullptr) {
            CloseHandle(hNewFrameEvent);
            hNewFrameEvent = nullptr;
        }
    }

    // Lock for thread-safe access
    bool Lock(DWORD timeout = INFINITE) {
        if (hMutex == nullptr) return false;
        return WaitForSingleObject(hMutex, timeout) == WAIT_OBJECT_0;
    }

    void Unlock() {
        if (hMutex != nullptr) {
            ReleaseMutex(hMutex);
        }
    }

    // Signal that a new frame is available
    void SignalNewFrame() {
        if (hNewFrameEvent != nullptr) {
            SetEvent(hNewFrameEvent);
        }
    }

    // Wait for new frame signal
    bool WaitForNewFrame(DWORD timeout = INFINITE) {
        if (hNewFrameEvent == nullptr) return false;
        return WaitForSingleObject(hNewFrameEvent, timeout) == WAIT_OBJECT_0;
    }

    SharedMemoryData* GetBuffer() { return pBuffer; }

private:
    HANDLE hMapFile;
    SharedMemoryData* pBuffer;
    HANDLE hMutex;
    HANDLE hNewFrameEvent;
};

} // namespace Swiper
