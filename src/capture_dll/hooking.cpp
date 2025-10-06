#include "hooking.h"
#include "../main/ipc.h"
#include <iostream>
#include <d3d11.h>

// Static member initialization
void* DirectXHook::s_originalPresent = nullptr;
void* DirectXHook::s_originalPresent1 = nullptr;
bool DirectXHook::s_initialized = false;
bool DirectXHook::s_hooksInstalled = false;

// Global IPC handles
extern HANDLE g_hSharedMemory;
extern SharedFrameData* g_pSharedData;
extern HANDLE g_hFrameReadyEvent;
extern HANDLE g_hMemoryMutex;

bool DirectXHook::Initialize() {
    if (s_initialized) {
        return true;
    }
    
    OutputDebugStringW(L"[Capture DLL] Initializing DirectX hooks...\n");
    
    // Detect DirectX version
    if (!DetectDirectXVersion()) {
        OutputDebugStringW(L"[Capture DLL] Failed to detect DirectX version\n");
        return false;
    }
    
    s_initialized = true;
    OutputDebugStringW(L"[Capture DLL] DirectX hooks initialized\n");
    return true;
}

void DirectXHook::Shutdown() {
    if (s_hooksInstalled) {
        RemoveHooks();
    }
    
    s_initialized = false;
    OutputDebugStringW(L"[Capture DLL] DirectX hooks shut down\n");
}

bool DirectXHook::InstallHooks() {
    if (s_hooksInstalled) {
        return true;
    }
    
    OutputDebugStringW(L"[Capture DLL] Installing hooks...\n");
    
    // TODO: Use MinHook or similar library to hook Present functions
    // For now, this is a placeholder
    
    // This would typically involve:
    // 1. Creating a dummy D3D11 device to get the Present function address
    // 2. Using a hooking library to replace the Present function
    // 3. Storing the original function pointer
    
    OutputDebugStringW(L"[Capture DLL] Hooks installed (placeholder)\n");
    s_hooksInstalled = true;
    return true;
}

bool DirectXHook::RemoveHooks() {
    if (!s_hooksInstalled) {
        return true;
    }
    
    OutputDebugStringW(L"[Capture DLL] Removing hooks...\n");
    
    // TODO: Use MinHook to unhook functions
    
    s_hooksInstalled = false;
    OutputDebugStringW(L"[Capture DLL] Hooks removed\n");
    return true;
}

bool DirectXHook::DetectDirectXVersion() {
    // Try to detect DirectX version by checking loaded modules
    HMODULE d3d11 = GetModuleHandleW(L"d3d11.dll");
    HMODULE d3d12 = GetModuleHandleW(L"d3d12.dll");
    HMODULE d3d9 = GetModuleHandleW(L"d3d9.dll");
    
    if (d3d11) {
        OutputDebugStringW(L"[Capture DLL] Detected DirectX 11\n");
        return true;
    }
    
    if (d3d12) {
        OutputDebugStringW(L"[Capture DLL] Detected DirectX 12\n");
        return true;
    }
    
    if (d3d9) {
        OutputDebugStringW(L"[Capture DLL] Detected DirectX 9\n");
        return true;
    }
    
    OutputDebugStringW(L"[Capture DLL] No DirectX version detected\n");
    return false;
}

HRESULT STDMETHODCALLTYPE DirectXHook::HookedPresent(
    IDXGISwapChain* pSwapChain,
    UINT SyncInterval,
    UINT Flags
) {
    // Capture the frame
    CaptureFrame(pSwapChain);
    
    // Call original Present
    typedef HRESULT(STDMETHODCALLTYPE* PresentFunc)(IDXGISwapChain*, UINT, UINT);
    PresentFunc originalFunc = (PresentFunc)s_originalPresent;
    
    if (originalFunc) {
        return originalFunc(pSwapChain, SyncInterval, Flags);
    }
    
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE DirectXHook::HookedPresent1(
    IDXGISwapChain1* pSwapChain,
    UINT SyncInterval,
    UINT Flags,
    const DXGI_PRESENT_PARAMETERS* pPresentParameters
) {
    // Capture the frame
    CaptureFrame(pSwapChain);
    
    // Call original Present1
    typedef HRESULT(STDMETHODCALLTYPE* Present1Func)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
    Present1Func originalFunc = (Present1Func)s_originalPresent1;
    
    if (originalFunc) {
        return originalFunc(pSwapChain, SyncInterval, Flags, pPresentParameters);
    }
    
    return E_FAIL;
}

void DirectXHook::CaptureFrame(IDXGISwapChain* pSwapChain) {
    if (!pSwapChain || !g_pSharedData) {
        return;
    }
    
    // Lock shared memory
    if (WaitForSingleObject(g_hMemoryMutex, 100) != WAIT_OBJECT_0) {
        return;
    }
    
    // Get back buffer
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    
    if (SUCCEEDED(hr) && pBackBuffer) {
        D3D11_TEXTURE2D_DESC desc;
        pBackBuffer->GetDesc(&desc);
        
        // Update shared data
        static uint32_t frameCounter = 0;
        g_pSharedData->width = desc.Width;
        g_pSharedData->height = desc.Height;
        g_pSharedData->format = desc.Format;
        g_pSharedData->frameNumber = ++frameCounter;
        g_pSharedData->timestamp = GetTickCount64();
        g_pSharedData->isValid = true;
        
        // TODO: Actually copy texture data to shared memory
        // This would involve creating a staging texture and copying data
        
        pBackBuffer->Release();
        
        // Signal frame ready
        SetEvent(g_hFrameReadyEvent);
    }
    
    // Unlock shared memory
    ReleaseMutex(g_hMemoryMutex);
}
