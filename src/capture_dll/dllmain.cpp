#include <windows.h>
#include "hooking.h"

using namespace Swiper;

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // DLL is being loaded into a process
        // Disable thread notifications for performance
        DisableThreadLibraryCalls(hModule);
        
        // Initialize DirectX hooks
        DirectXHook::GetInstance().Initialize();
        break;

    case DLL_PROCESS_DETACH:
        // DLL is being unloaded
        DirectXHook::GetInstance().Shutdown();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // Not used due to DisableThreadLibraryCalls
        break;
    }
    return TRUE;
}
