#ifndef INJECTOR_H
#define INJECTOR_H

#include <Windows.h>
#include <string>
#include <vector>

struct ProcessInfo {
    DWORD processId;
    std::wstring processName;
    std::wstring windowTitle;
};

class Injector {
public:
    // Enumerate all running processes
    static std::vector<ProcessInfo> EnumerateProcesses();
    
    // Inject DLL into target process
    static bool InjectDLL(DWORD processId, const std::wstring& dllPath);
    
    // Eject DLL from target process
    static bool EjectDLL(DWORD processId, const std::wstring& dllPath);
    
    // Check if process is 64-bit
    static bool IsProcess64Bit(DWORD processId);
    
private:
    // Helper to get full path to DLL
    static std::wstring GetDLLPath(const std::wstring& dllName);
};

#endif // INJECTOR_H
