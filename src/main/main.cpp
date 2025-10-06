#include <Windows.h>
#include <iostream>
#include <string>
#include "ipc.h"
#include "injector.h"

// Global variables
HWND g_hWnd = nullptr;
bool g_isRunning = true;
HANDLE g_hSharedMemory = nullptr;
SharedFrameData* g_pSharedData = nullptr;
HANDLE g_hFrameReadyEvent = nullptr;
HANDLE g_hMemoryMutex = nullptr;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool InitializeWindow(HINSTANCE hInstance);
bool InitializeIPC();
void CleanupIPC();
void RenderLoop();
void ProcessSelection();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Enable console for debugging
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    
    std::wcout << L"Swiper - DirectX Game Capture & Projection" << std::endl;
    std::wcout << L"==========================================" << std::endl;
    std::wcout << L"Initializing..." << std::endl;
    
    // Initialize IPC
    if (!InitializeIPC()) {
        std::wcerr << L"Failed to initialize IPC" << std::endl;
        return 1;
    }
    
    // Initialize window
    if (!InitializeWindow(hInstance)) {
        std::wcerr << L"Failed to initialize window" << std::endl;
        CleanupIPC();
        return 1;
    }
    
    std::wcout << L"Window created successfully" << std::endl;
    std::wcout << L"\nCommands:" << std::endl;
    std::wcout << L"  1 - List processes" << std::endl;
    std::wcout << L"  2 - Inject DLL into process" << std::endl;
    std::wcout << L"  ESC - Exit application" << std::endl;
    
    // Main message loop
    MSG msg = {};
    while (g_isRunning) {
        // Process Windows messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Check for console input
        if (_kbhit()) {
            int key = _getch();
            if (key == '1') {
                ProcessSelection();
            } else if (key == '2') {
                std::wcout << L"\nEnter process ID to inject: ";
                DWORD pid;
                std::wcin >> pid;
                
                std::wstring dllPath = L"capture.dll";
                // Get full path to DLL
                wchar_t exePath[MAX_PATH];
                GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                std::wstring fullPath(exePath);
                size_t lastSlash = fullPath.find_last_of(L"\\/");
                if (lastSlash != std::wstring::npos) {
                    fullPath = fullPath.substr(0, lastSlash + 1);
                }
                fullPath += dllPath;
                
                if (Injector::InjectDLL(pid, fullPath)) {
                    std::wcout << L"Injection successful!" << std::endl;
                } else {
                    std::wcerr << L"Injection failed!" << std::endl;
                }
            } else if (key == 27) { // ESC
                g_isRunning = false;
            }
        }
        
        // Render frame if available
        RenderLoop();
        
        // Small sleep to prevent CPU spinning
        Sleep(1);
    }
    
    // Cleanup
    CleanupIPC();
    if (g_hWnd) {
        DestroyWindow(g_hWnd);
    }
    
    std::wcout << L"\nSwiper shutting down..." << std::endl;
    FreeConsole();
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            g_isRunning = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                g_isRunning = false;
            }
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Fill background with black
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            
            // Draw text
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            const wchar_t* text = L"Swiper Projection Window\n\nWaiting for captured frames...\n\nPress ESC to exit";
            DrawTextW(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool InitializeWindow(HINSTANCE hInstance) {
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"SwiperWindowClass";
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    
    // Create borderless window
    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST,
        L"SwiperWindowClass",
        L"Swiper Projection",
        WS_POPUP,
        100, 100, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!g_hWnd) {
        return false;
    }
    
    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    
    return true;
}

bool InitializeIPC() {
    // Create shared memory
    g_hSharedMemory = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        sizeof(SharedFrameData),
        SHARED_MEMORY_NAME
    );
    
    if (!g_hSharedMemory) {
        std::wcerr << L"Failed to create shared memory: " << GetLastError() << std::endl;
        return false;
    }
    
    // Map shared memory
    g_pSharedData = (SharedFrameData*)MapViewOfFile(
        g_hSharedMemory,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(SharedFrameData)
    );
    
    if (!g_pSharedData) {
        std::wcerr << L"Failed to map shared memory: " << GetLastError() << std::endl;
        CloseHandle(g_hSharedMemory);
        return false;
    }
    
    // Initialize shared data
    ZeroMemory(g_pSharedData, sizeof(SharedFrameData));
    g_pSharedData->isValid = false;
    
    // Create synchronization objects
    g_hFrameReadyEvent = CreateEventW(nullptr, FALSE, FALSE, FRAME_READY_EVENT_NAME);
    g_hMemoryMutex = CreateMutexW(nullptr, FALSE, MEMORY_MUTEX_NAME);
    
    if (!g_hFrameReadyEvent || !g_hMemoryMutex) {
        std::wcerr << L"Failed to create synchronization objects" << std::endl;
        CleanupIPC();
        return false;
    }
    
    std::wcout << L"IPC initialized successfully" << std::endl;
    return true;
}

void CleanupIPC() {
    if (g_pSharedData) {
        UnmapViewOfFile(g_pSharedData);
        g_pSharedData = nullptr;
    }
    
    if (g_hSharedMemory) {
        CloseHandle(g_hSharedMemory);
        g_hSharedMemory = nullptr;
    }
    
    if (g_hFrameReadyEvent) {
        CloseHandle(g_hFrameReadyEvent);
        g_hFrameReadyEvent = nullptr;
    }
    
    if (g_hMemoryMutex) {
        CloseHandle(g_hMemoryMutex);
        g_hMemoryMutex = nullptr;
    }
}

void RenderLoop() {
    // Wait for frame ready event (non-blocking)
    DWORD result = WaitForSingleObject(g_hFrameReadyEvent, 0);
    if (result != WAIT_OBJECT_0) {
        return;
    }
    
    // Lock shared memory
    if (WaitForSingleObject(g_hMemoryMutex, 100) != WAIT_OBJECT_0) {
        return;
    }
    
    // Check if frame is valid
    if (g_pSharedData && g_pSharedData->isValid) {
        // TODO: Render frame to window
        // For now, just print info
        static uint32_t lastFrame = 0;
        if (g_pSharedData->frameNumber != lastFrame) {
            std::wcout << L"Received frame #" << g_pSharedData->frameNumber 
                      << L" (" << g_pSharedData->width << L"x" << g_pSharedData->height << L")" 
                      << std::endl;
            lastFrame = g_pSharedData->frameNumber;
        }
    }
    
    // Unlock shared memory
    ReleaseMutex(g_hMemoryMutex);
}

void ProcessSelection() {
    std::wcout << L"\nEnumerating processes..." << std::endl;
    std::wcout << L"=========================" << std::endl;
    
    auto processes = Injector::EnumerateProcesses();
    
    int count = 0;
    for (const auto& proc : processes) {
        if (count++ < 20) { // Show first 20 processes
            std::wcout << L"PID: " << proc.processId << L" - " << proc.processName << std::endl;
        }
    }
    
    std::wcout << L"\nTotal processes: " << processes.size() << std::endl;
}
