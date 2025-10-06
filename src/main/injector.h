#pragma once

#include <windows.h>
#include <string>

namespace Swiper {

class Injector {
public:
    // Inject a DLL into a target process
    static bool InjectDLL(DWORD processId, const std::wstring& dllPath);

    // Eject a DLL from a target process (optional, for cleanup)
    static bool EjectDLL(DWORD processId, const std::wstring& dllPath);

    // Helper: Get full path to the capture DLL
    static std::wstring GetCaptureDllPath();

    // Helper: Check if process is 64-bit
    static bool IsProcess64Bit(DWORD processId);

private:
    // Remote thread injection method
    static bool InjectViaCreateRemoteThread(HANDLE hProcess, const std::wstring& dllPath);
};

} // namespace Swiper
