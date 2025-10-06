#include "hooking.h"
#include "../main/ipc.h"
#include <iostream>
#include <d3d11.h>

#ifdef MINHOOK_AVAILABLE
#include <MinHook.h>
#endif

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
    
#ifdef MINHOOK_AVAILABLE
    // Initialize MinHook
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) {
        OutputDebugStringW(L"[Capture DLL] Failed to initialize MinHook\n");
        return false;
    }
#endif
    
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
    
#ifdef MINHOOK_AVAILABLE
    MH_Uninitialize();
#endif
    
    s_initialized = false;
    OutputDebugStringW(L"[Capture DLL] DirectX hooks shut down\n");
}

bool DirectXHook::InstallHooks() {
    if (s_hooksInstalled) {
        return true;
    }
    
    OutputDebugStringW(L"[Capture DLL] Installing hooks...\n");
    
#ifdef MINHOOK_AVAILABLE
    // Create a temporary D3D11 device and swap chain to get the Present function address
    D3D_FEATURE_LEVEL featureLevel;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = GetDesktopWindow();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    
    ID3D11Device* pTempDevice = nullptr;
    ID3D11DeviceContext* pTempContext = nullptr;
    IDXGISwapChain* pTempSwapChain = nullptr;
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &pTempSwapChain,
        &pTempDevice,
        &featureLevel,
        &pTempContext
    );
    
    if (FAILED(hr) || !pTempSwapChain) {
        OutputDebugStringW(L"[Capture DLL] Failed to create temporary D3D11 device\n");
        return false;
    }
    
    // Get the Present function address from the swap chain's vtable
    void** swapChainVTable = *(void***)pTempSwapChain;
    void* presentAddress = swapChainVTable[8]; // Present is at index 8 in IDXGISwapChain vtable
    
    // Create hook for Present
    MH_STATUS status = MH_CreateHook(
        presentAddress,
        &HookedPresent,
        reinterpret_cast<void**>(&s_originalPresent)
    );
    
    if (status != MH_OK) {
        OutputDebugStringW(L"[Capture DLL] Failed to create Present hook\n");
        pTempSwapChain->Release();
        pTempContext->Release();
        pTempDevice->Release();
        return false;
    }
    
    // Enable the hook
    status = MH_EnableHook(presentAddress);
    if (status != MH_OK) {
        OutputDebugStringW(L"[Capture DLL] Failed to enable Present hook\n");
        pTempSwapChain->Release();
        pTempContext->Release();
        pTempDevice->Release();
        return false;
    }
    
    // Cleanup temporary objects
    pTempSwapChain->Release();
    pTempContext->Release();
    pTempDevice->Release();
    
    OutputDebugStringW(L"[Capture DLL] Hooks installed successfully\n");
    s_hooksInstalled = true;
    return true;
#else
    OutputDebugStringW(L"[Capture DLL] MinHook not available, hooks not installed\n");
    return false;
#endif
}

bool DirectXHook::RemoveHooks() {
    if (!s_hooksInstalled) {
        return true;
    }
    
    OutputDebugStringW(L"[Capture DLL] Removing hooks...\n");
    
#ifdef MINHOOK_AVAILABLE
    if (s_originalPresent) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
    }
#endif
    
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
    
    // CRUCIAL: Call original Present to prevent the game from freezing
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
    
    // CRUCIAL: Call original Present1 to prevent the game from freezing
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
        
        // Get device from back buffer
        ID3D11Device* pDevice = nullptr;
        pBackBuffer->GetDevice(&pDevice);
        
        if (pDevice) {
            ID3D11DeviceContext* pContext = nullptr;
            pDevice->GetImmediateContext(&pContext);
            
            if (pContext) {
                // Create staging texture for CPU readback
                D3D11_TEXTURE2D_DESC stagingDesc = desc;
                stagingDesc.Usage = D3D11_USAGE_STAGING;
                stagingDesc.BindFlags = 0;
                stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                stagingDesc.MiscFlags = 0;
                
                ID3D11Texture2D* pStagingTexture = nullptr;
                hr = pDevice->CreateTexture2D(&stagingDesc, nullptr, &pStagingTexture);
                
                if (SUCCEEDED(hr) && pStagingTexture) {
                    // Copy back buffer to staging texture
                    pContext->CopyResource(pStagingTexture, pBackBuffer);
                    
                    // Map the staging texture to read pixel data
                    D3D11_MAPPED_SUBRESOURCE mappedResource;
                    hr = pContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
                    
                    if (SUCCEEDED(hr)) {
                        // Update shared data
                        static uint32_t frameCounter = 0;
                        g_pSharedData->width = desc.Width;
                        g_pSharedData->height = desc.Height;
                        g_pSharedData->format = desc.Format;
                        g_pSharedData->frameNumber = ++frameCounter;
                        g_pSharedData->timestamp = GetTickCount64();
                        g_pSharedData->isValid = true;
                        
                        // Copy pixel data to shared memory (up to MAX_FRAME_SIZE)
                        size_t dataSize = desc.Width * desc.Height * 4; // Assuming RGBA
                        if (dataSize <= MAX_FRAME_SIZE) {
                            const uint8_t* srcData = static_cast<const uint8_t*>(mappedResource.pData);
                            for (UINT row = 0; row < desc.Height; ++row) {
                                memcpy(
                                    g_pSharedData->pixelData + row * desc.Width * 4,
                                    srcData + row * mappedResource.RowPitch,
                                    desc.Width * 4
                                );
                            }
                        }
                        
                        pContext->Unmap(pStagingTexture, 0);
                        
                        // Signal frame ready
                        SetEvent(g_hFrameReadyEvent);
                    }
                    
                    pStagingTexture->Release();
                }
                
                pContext->Release();
            }
            
            pDevice->Release();
        }
        
        pBackBuffer->Release();
    }
    
    // Unlock shared memory
    ReleaseMutex(g_hMemoryMutex);
}
