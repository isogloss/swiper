#ifndef HOOKING_H
#define HOOKING_H

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

// Hooking management
class DirectXHook {
public:
    static bool Initialize();
    static void Shutdown();
    
    // Hook installation
    static bool InstallHooks();
    static bool RemoveHooks();
    
    // DirectX version detection
    static bool DetectDirectXVersion();
    
private:
    // Hook handlers
    static HRESULT STDMETHODCALLTYPE HookedPresent(
        IDXGISwapChain* pSwapChain,
        UINT SyncInterval,
        UINT Flags
    );
    
    static HRESULT STDMETHODCALLTYPE HookedPresent1(
        IDXGISwapChain1* pSwapChain,
        UINT SyncInterval,
        UINT Flags,
        const DXGI_PRESENT_PARAMETERS* pPresentParameters
    );
    
    // Frame capture
    static void CaptureFrame(IDXGISwapChain* pSwapChain);
    
    // Original function pointers
    static void* s_originalPresent;
    static void* s_originalPresent1;
    
    static bool s_initialized;
    static bool s_hooksInstalled;
};

#endif // HOOKING_H
