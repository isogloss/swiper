#include "injector.h"
#include <iostream>
#include <vector>
#include <tlhelp32.h>

namespace Swiper {

bool Injector::InjectDLL(DWORD processId, const std::wstring& dllPath) {
    // Open target process with required access rights
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | 
        PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        FALSE,
        processId
    );

    if (hProcess == nullptr) {
        std::wcerr << L"Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }

    bool result = InjectViaCreateRemoteThread(hProcess, dllPath);
    
    CloseHandle(hProcess);
    return result;
}

bool Injector::InjectViaCreateRemoteThread(HANDLE hProcess, const std::wstring& dllPath) {
    // Calculate size needed for DLL path
    size_t pathSize = (dllPath.length() + 1) * sizeof(wchar_t);

    // Allocate memory in target process
    LPVOID pRemotePath = VirtualAllocEx(
        hProcess,
        nullptr,
        pathSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (pRemotePath == nullptr) {
        std::wcerr << L"Failed to allocate memory in target process. Error: " 
                   << GetLastError() << std::endl;
        return false;
    }

    // Write DLL path to target process
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(hProcess, pRemotePath, dllPath.c_str(), 
                           pathSize, &bytesWritten)) {
        std::wcerr << L"Failed to write DLL path to target process. Error: " 
                   << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    // Get address of LoadLibraryW
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32 == nullptr) {
        std::wcerr << L"Failed to get kernel32.dll handle" << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    LPVOID pLoadLibrary = reinterpret_cast<LPVOID>(
        GetProcAddress(hKernel32, "LoadLibraryW")
    );

    if (pLoadLibrary == nullptr) {
        std::wcerr << L"Failed to get LoadLibraryW address" << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    // Create remote thread to load the DLL
    HANDLE hThread = CreateRemoteThread(
        hProcess,
        nullptr,
        0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibrary),
        pRemotePath,
        0,
        nullptr
    );

    if (hThread == nullptr) {
        std::wcerr << L"Failed to create remote thread. Error: " 
                   << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        return false;
    }

    // Wait for thread to complete
    WaitForSingleObject(hThread, INFINITE);

    // Get thread exit code (HMODULE of loaded DLL)
    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);

    if (exitCode == 0) {
        std::wcerr << L"LoadLibraryW returned NULL in target process" << std::endl;
        return false;
    }

    std::wcout << L"DLL injected successfully. Module handle: 0x" 
               << std::hex << exitCode << std::dec << std::endl;
    return true;
}

bool Injector::EjectDLL(DWORD processId, const std::wstring& dllPath) {
    // TODO: Implement DLL ejection using FreeLibrary
    // This is optional and can be added later for cleanup
    (void)processId;
    (void)dllPath;
    return false;
}

std::wstring Injector::GetCaptureDllPath() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    
    std::wstring path(exePath);
    size_t lastSlash = path.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        path = path.substr(0, lastSlash + 1);
    }
    
    path += L"capture.dll";
    return path;
}

bool Injector::IsProcess64Bit(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == nullptr) {
        return false;
    }

    BOOL isWow64 = FALSE;
    
#ifdef _WIN64
    // On 64-bit Windows, check if process is running under WOW64
    IsWow64Process(hProcess, &isWow64);
    CloseHandle(hProcess);
    return !isWow64;  // If not WOW64, it's 64-bit
#else
    // On 32-bit Windows, all processes are 32-bit
    CloseHandle(hProcess);
    return false;
#endif
}

} // namespace Swiper
