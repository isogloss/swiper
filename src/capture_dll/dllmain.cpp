#include <Windows.h>
#include "hooking.h"
#include "../main/ipc.h"

// Global IPC handles
HANDLE g_hSharedMemory = nullptr;
SharedFrameData* g_pSharedData = nullptr;
HANDLE g_hFrameReadyEvent = nullptr;
HANDLE g_hMemoryMutex = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            OutputDebugStringW(L"[Capture DLL] DLL_PROCESS_ATTACH\n");
            DisableThreadLibraryCalls(hModule);
            
            // Open shared memory (created by main app)
            g_hSharedMemory = OpenFileMappingW(
                FILE_MAP_ALL_ACCESS,
                FALSE,
                SHARED_MEMORY_NAME
            );
            
            if (!g_hSharedMemory) {
                OutputDebugStringW(L"[Capture DLL] Failed to open shared memory\n");
                return FALSE;
            }
            
            // Map shared memory
            g_pSharedData = (SharedFrameData*)MapViewOfFile(
                g_hSharedMemory,
                FILE_MAP_ALL_ACCESS,
                0, 0,
                sizeof(SharedFrameData)
            );
            
            if (!g_pSharedData) {
                OutputDebugStringW(L"[Capture DLL] Failed to map shared memory\n");
                CloseHandle(g_hSharedMemory);
                return FALSE;
            }
            
            // Open synchronization objects
            g_hFrameReadyEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, FRAME_READY_EVENT_NAME);
            g_hMemoryMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, MEMORY_MUTEX_NAME);
            
            if (!g_hFrameReadyEvent || !g_hMemoryMutex) {
                OutputDebugStringW(L"[Capture DLL] Failed to open synchronization objects\n");
                return FALSE;
            }
            
            OutputDebugStringW(L"[Capture DLL] IPC initialized successfully\n");
            
            // Initialize DirectX hooks
            if (!DirectXHook::Initialize()) {
                OutputDebugStringW(L"[Capture DLL] Failed to initialize hooks\n");
                return FALSE;
            }
            
            // Install hooks
            if (!DirectXHook::InstallHooks()) {
                OutputDebugStringW(L"[Capture DLL] Failed to install hooks\n");
                return FALSE;
            }
            
            OutputDebugStringW(L"[Capture DLL] Capture DLL loaded successfully\n");
            break;
            
        case DLL_THREAD_ATTACH:
            break;
            
        case DLL_THREAD_DETACH:
            break;
            
        case DLL_PROCESS_DETACH:
            OutputDebugStringW(L"[Capture DLL] DLL_PROCESS_DETACH\n");
            
            // Shutdown hooks
            DirectXHook::Shutdown();
            
            // Cleanup IPC
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
            
            OutputDebugStringW(L"[Capture DLL] Capture DLL unloaded\n");
            break;
    }
    
    return TRUE;
}
