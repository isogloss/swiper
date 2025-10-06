#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include "injector.h"
#include "ipc.h"

using namespace Swiper;

// Forward declarations
void PrintUsage();
void DisplayCapturedFrames(SharedMemoryManager& shmManager);

int main(int argc, char* argv[]) {
    std::cout << "=== Swiper Game Capture & Projection ===" << std::endl;
    std::cout << "Version 1.0.0" << std::endl;
    std::cout << std::endl;

    // Parse command line arguments
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    // Get target process ID
    DWORD targetPid = static_cast<DWORD>(std::atoi(argv[1]));
    if (targetPid == 0) {
        std::cerr << "Invalid process ID: " << argv[1] << std::endl;
        return 1;
    }

    std::cout << "Target Process ID: " << targetPid << std::endl;

    // Check if process is 64-bit
    if (Injector::IsProcess64Bit(targetPid)) {
        std::cout << "Target process is 64-bit" << std::endl;
#ifndef _WIN64
        std::cerr << "ERROR: Cannot inject 64-bit DLL from 32-bit process!" << std::endl;
        std::cerr << "Please use the 64-bit version of swiper.exe" << std::endl;
        return 1;
#endif
    } else {
        std::cout << "Target process is 32-bit" << std::endl;
#ifdef _WIN64
        std::cerr << "WARNING: Injecting 64-bit DLL into 32-bit process may fail!" << std::endl;
#endif
    }

    // Initialize shared memory
    std::cout << std::endl << "Initializing shared memory..." << std::endl;
    SharedMemoryManager shmManager;
    if (!shmManager.Create()) {
        std::cerr << "Failed to create shared memory. Error: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "Shared memory initialized successfully." << std::endl;

    // Get path to capture DLL
    std::wstring dllPath = Injector::GetCaptureDllPath();
    std::wcout << L"Capture DLL path: " << dllPath << std::endl;

    // Check if DLL exists
    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wcerr << L"ERROR: capture.dll not found at: " << dllPath << std::endl;
        std::wcerr << L"Please ensure capture.dll is in the same directory as swiper.exe" << std::endl;
        return 1;
    }

    // Inject DLL into target process
    std::cout << std::endl << "Injecting capture.dll into target process..." << std::endl;
    if (!Injector::InjectDLL(targetPid, dllPath)) {
        std::cerr << "Failed to inject DLL. Make sure you have administrator privileges." << std::endl;
        return 1;
    }

    std::cout << "Injection successful!" << std::endl;
    std::cout << std::endl << "Waiting for captured frames... (Press Ctrl+C to exit)" << std::endl;
    std::cout << std::endl;

    // Main loop: display captured frames
    DisplayCapturedFrames(shmManager);

    return 0;
}

void PrintUsage() {
    std::cout << "Usage: swiper.exe <process_id>" << std::endl;
    std::cout << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  process_id    The Process ID of the game to capture" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  swiper.exe 1234" << std::endl;
    std::cout << std::endl;
    std::cout << "To find a process ID:" << std::endl;
    std::cout << "  1. Open Task Manager (Ctrl+Shift+Esc)" << std::endl;
    std::cout << "  2. Go to Details tab" << std::endl;
    std::cout << "  3. Find your game process and note the PID column" << std::endl;
}

void DisplayCapturedFrames(SharedMemoryManager& shmManager) {
    uint64_t lastFrameNumber = 0;
    uint32_t frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        // Wait for new frame (with timeout to allow Ctrl+C)
        if (!shmManager.WaitForNewFrame(1000)) {
            continue;
        }

        // Lock shared memory
        if (!shmManager.Lock(100)) {
            continue;
        }

        SharedMemoryData* data = shmManager.GetBuffer();
        if (data != nullptr && data->header.frameNumber > lastFrameNumber) {
            lastFrameNumber = data->header.frameNumber;
            frameCount++;

            // Display frame info
            std::cout << "Frame #" << data->header.frameNumber
                      << " | Resolution: " << data->header.width << "x" << data->header.height
                      << " | Format: " << static_cast<int>(data->header.format)
                      << " | Size: " << data->header.dataSize << " bytes"
                      << std::endl;

            // Calculate FPS
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                currentTime - startTime
            ).count();

            if (elapsed > 0 && frameCount % 60 == 0) {
                double fps = static_cast<double>(frameCount) / elapsed;
                std::cout << "  Average FPS: " << fps << std::endl;
            }
        }

        shmManager.Unlock();
    }
}
