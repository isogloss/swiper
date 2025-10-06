#include <Windows.h>
#include <iostream>
#include <string>
#include "ipc.h"
#include "injector.h"

#ifdef IMGUI_AVAILABLE
#include "imgui_ui.h"
#else
#include "ui.h"
#endif

// Global variables
HWND g_hWnd = nullptr;
HWND g_hMainWindow = nullptr;
bool g_isRunning = true;

// IPC using RAII wrappers from ipc.h
UniqueHandle g_hSharedMemory;
UniqueHandle g_hFrameReadyEvent;
UniqueHandle g_hMemoryMutex;
std::unique_ptr<MappedView> g_pSharedMemoryView;
SharedFrameData* g_pSharedData = nullptr;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool InitializeWindow(HINSTANCE hInstance);
bool InitializeMainWindow(HINSTANCE hInstance);
bool InitializeIPC();
void CleanupIPC();
void RenderLoop();
void ProcessSelection();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#ifdef IMGUI_AVAILABLE
    // Use ImGui-based GUI
    return RunImGuiApplication(hInstance);
#else
    // Fallback to Win32 UI with console
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
    
    // Initialize main UI window
    if (!InitializeMainWindow(hInstance)) {
        std::wcerr << L"Failed to initialize main window" << std::endl;
        CleanupIPC();
        return 1;
    }
    
    // Initialize projection window
    if (!InitializeWindow(hInstance)) {
        std::wcerr << L"Failed to initialize projection window" << std::endl;
        CleanupIPC();
        return 1;
    }
    
    std::wcout << L"Windows created successfully" << std::endl;
    std::wcout << L"\nUI is ready - use the Settings and Projection tabs" << std::endl;
    
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
        
        // Render frame if available
        RenderLoop();
        
        // Small sleep to prevent CPU spinning
        Sleep(1);
    }
    
    // Cleanup
    CleanupIPC();
    if (g_pUIManager) {
        delete g_pUIManager;
        g_pUIManager = nullptr;
    }
    if (g_hWnd) {
        DestroyWindow(g_hWnd);
    }
    if (g_hMainWindow) {
        DestroyWindow(g_hMainWindow);
    }
    
    std::wcout << L"\nSwiper shutting down..." << std::endl;
    FreeConsole();
    
    return 0;
#endif
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

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            g_isRunning = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_COMMAND:
            if (g_pUIManager) {
                g_pUIManager->OnCommand(wParam, lParam);
            }
            return 0;
            
        case WM_NOTIFY: {
            NMHDR* pNmhdr = (NMHDR*)lParam;
            if (pNmhdr->code == TCN_SELCHANGE && g_pUIManager) {
                int newTab = TabCtrl_GetCurSel(pNmhdr->hwndFrom);
                g_pUIManager->OnTabChanged(newTab);
            }
            return 0;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
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

bool InitializeMainWindow(HINSTANCE hInstance) {
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"SwiperMainWindowClass";
    
    if (!RegisterClassExW(&wc)) {
        return false;
    }
    
    // Create main UI window
    g_hMainWindow = CreateWindowExW(
        0,
        L"SwiperMainWindowClass",
        L"Swiper Control Panel",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 360,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!g_hMainWindow) {
        return false;
    }
    
    // Initialize UI manager
    g_pUIManager = new UIManager();
    if (!g_pUIManager->Initialize(g_hMainWindow, hInstance)) {
        std::wcerr << L"Failed to initialize UI manager" << std::endl;
        delete g_pUIManager;
        g_pUIManager = nullptr;
        return false;
    }
    
    ShowWindow(g_hMainWindow, SW_SHOW);
    UpdateWindow(g_hMainWindow);
    
    return true;
}

bool InitializeIPC() {
    // Create shared memory using RAII wrapper
    g_hSharedMemory.reset(CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        sizeof(SharedFrameData),
        SHARED_MEMORY_NAME
    ));
    
    if (!g_hSharedMemory) {
        std::wcerr << L"Failed to create shared memory: " << GetLastError() << std::endl;
        return false;
    }
    
    // Map shared memory using RAII wrapper
    g_pSharedMemoryView = std::make_unique<MappedView>(
        g_hSharedMemory.get(),
        FILE_MAP_ALL_ACCESS,
        sizeof(SharedFrameData)
    );
    
    if (!g_pSharedMemoryView || !(*g_pSharedMemoryView)) {
        std::wcerr << L"Failed to map shared memory: " << GetLastError() << std::endl;
        g_hSharedMemory.reset();
        return false;
    }
    
    g_pSharedData = g_pSharedMemoryView->as<SharedFrameData>();
    
    // Initialize shared data
    ZeroMemory(g_pSharedData, sizeof(SharedFrameData));
    g_pSharedData->isValid = false;
    
    // Create synchronization objects using RAII wrappers
    g_hFrameReadyEvent.reset(CreateEventW(nullptr, FALSE, FALSE, FRAME_READY_EVENT_NAME));
    g_hMemoryMutex.reset(CreateMutexW(nullptr, FALSE, MEMORY_MUTEX_NAME));
    
    if (!g_hFrameReadyEvent || !g_hMemoryMutex) {
        std::wcerr << L"Failed to create synchronization objects" << std::endl;
        CleanupIPC();
        return false;
    }
    
    std::wcout << L"IPC initialized successfully" << std::endl;
    return true;
}

void CleanupIPC() {
    // RAII wrappers will automatically cleanup on destruction
    g_pSharedData = nullptr;
    g_pSharedMemoryView.reset();
    g_hSharedMemory.reset();
    g_hFrameReadyEvent.reset();
    g_hMemoryMutex.reset();
}

void RenderLoop() {
    // Wait for frame ready event (non-blocking)
    DWORD result = WaitForSingleObject(g_hFrameReadyEvent.get(), 0);
    if (result != WAIT_OBJECT_0) {
        return;
    }
    
    // Lock shared memory
    if (WaitForSingleObject(g_hMemoryMutex.get(), 100) != WAIT_OBJECT_0) {
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
    ReleaseMutex(g_hMemoryMutex.get());
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
