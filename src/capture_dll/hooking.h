#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <d3d9.h>
#include <cstdint>

namespace Swiper {

// DirectX hooking manager
class DirectXHook {
public:
    static DirectXHook& GetInstance();

    // Initialize hooks
    bool Initialize();
    
    // Cleanup hooks
    void Shutdown();

    // Check if hooks are active
    bool IsHooked() const { return m_isHooked; }

private:
    DirectXHook() : m_isHooked(false) {}
    ~DirectXHook();

    DirectXHook(const DirectXHook&) = delete;
    DirectXHook& operator=(const DirectXHook&) = delete;

    bool m_isHooked;

    // Hook DirectX 11
    bool HookD3D11();

    // Hook DirectX 9
    bool HookD3D9();

    // Hooked function pointers (to be implemented with MinHook)
    // These would store the original function addresses
};

// Hooked Present function for D3D11
HRESULT STDMETHODCALLTYPE HookedPresent_D3D11(
    IDXGISwapChain* pSwapChain,
    UINT SyncInterval,
    UINT Flags
);

// Hooked Present function for D3D9
HRESULT STDMETHODCALLTYPE HookedPresent_D3D9(
    IDirect3DDevice9* pDevice,
    CONST RECT* pSourceRect,
    CONST RECT* pDestRect,
    HWND hDestWindowOverride,
    CONST RGNDATA* pDirtyRegion
);

// Helper: Capture frame from D3D11 swap chain
bool CaptureFrame_D3D11(IDXGISwapChain* pSwapChain);

// Helper: Capture frame from D3D9 device
bool CaptureFrame_D3D9(IDirect3DDevice9* pDevice);

} // namespace Swiper
