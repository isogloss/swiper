#ifndef IPC_H
#define IPC_H

#include <Windows.h>
#include <cstdint>
#include <memory>

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

// RAII wrapper for Windows HANDLE
struct HandleDeleter {
    void operator()(HANDLE handle) const {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
};

using UniqueHandle = std::unique_ptr<void, HandleDeleter>;

// RAII wrapper for mapped view of file
class MappedView {
public:
    MappedView(HANDLE hFileMapping, DWORD desiredAccess, size_t size)
        : m_view(nullptr) {
        m_view = MapViewOfFile(hFileMapping, desiredAccess, 0, 0, size);
    }
    
    ~MappedView() {
        if (m_view) {
            UnmapViewOfFile(m_view);
        }
    }
    
    void* get() const { return m_view; }
    explicit operator bool() const { return m_view != nullptr; }
    
    template<typename T>
    T* as() const { return static_cast<T*>(m_view); }
    
    // Prevent copying
    MappedView(const MappedView&) = delete;
    MappedView& operator=(const MappedView&) = delete;
    
private:
    void* m_view;
};

#endif // IPC_H
