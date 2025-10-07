#include "injector.h"
#include <TlHelp32.h>
#include <iostream>
#include <memory>
#include <filesystem>

// RAII wrapper for Windows HANDLE
struct HandleDeleter {
    void operator()(HANDLE handle) const {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
};

using UniqueHandle = std::unique_ptr<void, HandleDeleter>;

// RAII wrapper for remote memory allocation
class RemoteMemory {
public:
    RemoteMemory(HANDLE process, size_t size)
        : m_process(process)
        , m_memory(nullptr) {
        m_memory = VirtualAllocEx(
            process, nullptr, size,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
        );
    }
    
    ~RemoteMemory() {
        if (m_memory) {
            VirtualFreeEx(m_process, m_memory, 0, MEM_RELEASE);
        }
    }
    
    LPVOID get() const { return m_memory; }
    explicit operator bool() const { return m_memory != nullptr; }
    
    // Prevent copying
    RemoteMemory(const RemoteMemory&) = delete;
    RemoteMemory& operator=(const RemoteMemory&) = delete;
    
private:
    HANDLE m_process;
    LPVOID m_memory;
};

std::vector<ProcessInfo> Injector::EnumerateProcesses() {
    std::vector<ProcessInfo> processes;
    
    UniqueHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!snapshot || snapshot.get() == INVALID_HANDLE_VALUE) {
        return processes;
    }
    
    PROCESSENTRY32W processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(snapshot.get(), &processEntry)) {
        do {
            ProcessInfo info;
            info.processId = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
            info.windowTitle = L""; // Would need EnumWindows to get actual titles
            
            processes.push_back(info);
        } while (Process32NextW(snapshot.get(), &processEntry));
    }
    
    return processes;
}

bool Injector::InjectDLL(DWORD processId, const std::wstring& dllPath) {
    // Open target process
    UniqueHandle hProcess(OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | 
        PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, processId
    ));
    
    if (!hProcess) {
        std::wcerr << L"Failed to open process: " << GetLastError() << std::endl;
        return false;
    }
    
    // Allocate memory in target process for DLL path
    size_t pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    RemoteMemory remoteMemory(hProcess.get(), pathSize);
    
    if (!remoteMemory) {
        std::wcerr << L"Failed to allocate memory: " << GetLastError() << std::endl;
        return false;
    }
    
    // Write DLL path to target process memory
    if (!WriteProcessMemory(hProcess.get(), remoteMemory.get(), dllPath.c_str(), pathSize, nullptr)) {
        std::wcerr << L"Failed to write memory: " << GetLastError() << std::endl;
        return false;
    }
    
    // Get address of LoadLibraryW
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    LPTHREAD_START_ROUTINE loadLibraryAddr = 
        (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
    
    if (!loadLibraryAddr) {
        std::wcerr << L"Failed to get LoadLibraryW address" << std::endl;
        return false;
    }
    
    // Create remote thread to load DLL
    UniqueHandle hThread(CreateRemoteThread(
        hProcess.get(), nullptr, 0,
        loadLibraryAddr, remoteMemory.get(),
        0, nullptr
    ));
    
    if (!hThread) {
        std::wcerr << L"Failed to create remote thread: " << GetLastError() << std::endl;
        return false;
    }
    
    // Wait for thread to complete
    WaitForSingleObject(hThread.get(), INFINITE);
    
    std::wcout << L"DLL injected successfully into process " << processId << std::endl;
    return true;
}

bool Injector::EjectDLL(DWORD processId, const std::wstring& dllPath) {
    // Implementation for ejecting DLL (finding module and calling FreeLibrary)
    // Not implemented in this basic version
    return false;
}

bool Injector::IsProcess64Bit(DWORD processId) {
    UniqueHandle hProcess(OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId));
    if (!hProcess) {
        return false;
    }
    
    BOOL isWow64 = FALSE;
    IsWow64Process(hProcess.get(), &isWow64);
    
    // If process is WOW64, it's 32-bit running on 64-bit Windows
    // If not WOW64 and we're on 64-bit Windows, process is 64-bit
    #ifdef _WIN64
    return !isWow64;
    #else
    return false;
    #endif
}

std::wstring Injector::GetDLLPath(const std::wstring& dllName) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    
    // Use C++17 filesystem for path manipulation
    std::filesystem::path executablePath(exePath);
    std::filesystem::path directory = executablePath.parent_path();
    std::filesystem::path dllFullPath = directory / dllName;
    
    return dllFullPath.wstring();
}
