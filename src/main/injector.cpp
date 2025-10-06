#include "injector.h"
#include <TlHelp32.h>
#include <iostream>

std::vector<ProcessInfo> Injector::EnumerateProcesses() {
    std::vector<ProcessInfo> processes;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return processes;
    }
    
    PROCESSENTRY32W processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(snapshot, &processEntry)) {
        do {
            ProcessInfo info;
            info.processId = processEntry.th32ProcessID;
            info.processName = processEntry.szExeFile;
            info.windowTitle = L""; // Would need EnumWindows to get actual titles
            
            processes.push_back(info);
        } while (Process32NextW(snapshot, &processEntry));
    }
    
    CloseHandle(snapshot);
    return processes;
}

bool Injector::InjectDLL(DWORD processId, const std::wstring& dllPath) {
    // Open target process
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | 
        PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, processId
    );
    
    if (!hProcess) {
        std::wcerr << L"Failed to open process: " << GetLastError() << std::endl;
        return false;
    }
    
    // Allocate memory in target process for DLL path
    size_t pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID remoteMemory = VirtualAllocEx(
        hProcess, nullptr, pathSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
    
    if (!remoteMemory) {
        std::wcerr << L"Failed to allocate memory: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }
    
    // Write DLL path to target process memory
    if (!WriteProcessMemory(hProcess, remoteMemory, dllPath.c_str(), pathSize, nullptr)) {
        std::wcerr << L"Failed to write memory: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // Get address of LoadLibraryW
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    LPTHREAD_START_ROUTINE loadLibraryAddr = 
        (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
    
    if (!loadLibraryAddr) {
        std::wcerr << L"Failed to get LoadLibraryW address" << std::endl;
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // Create remote thread to load DLL
    HANDLE hThread = CreateRemoteThread(
        hProcess, nullptr, 0,
        loadLibraryAddr, remoteMemory,
        0, nullptr
    );
    
    if (!hThread) {
        std::wcerr << L"Failed to create remote thread: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    // Wait for thread to complete
    WaitForSingleObject(hThread, INFINITE);
    
    // Cleanup
    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    
    std::wcout << L"DLL injected successfully into process " << processId << std::endl;
    return true;
}

bool Injector::EjectDLL(DWORD processId, const std::wstring& dllPath) {
    // Implementation for ejecting DLL (finding module and calling FreeLibrary)
    // Not implemented in this basic version
    return false;
}

bool Injector::IsProcess64Bit(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess) {
        return false;
    }
    
    BOOL isWow64 = FALSE;
    IsWow64Process(hProcess, &isWow64);
    CloseHandle(hProcess);
    
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
    
    std::wstring path(exePath);
    size_t lastSlash = path.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        path = path.substr(0, lastSlash + 1);
    }
    
    return path + dllName;
}
