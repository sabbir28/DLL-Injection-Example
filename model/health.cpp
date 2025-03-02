// MemoryReader.cpp
#include "health.h"
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <cstdint>

// Define TH32CS_SNAPMODULE32 if it's not defined.
#ifndef TH32CS_SNAPMODULE32
#define TH32CS_SNAPMODULE32 0x10
#endif

// Function to get the process ID by process name.
DWORD GetProcessID(const wchar_t* processName) {
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W procEntry;
        procEntry.dwSize = sizeof(procEntry);
        if (Process32FirstW(hSnap, &procEntry)) {
            do {
                if (std::wstring(procEntry.szExeFile) == processName) {
                    processID = procEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(hSnap, &procEntry));
        }
        CloseHandle(hSnap);
    }
    return processID;
}

// Function to get the base address of a module in a remote process.
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32FirstW(hSnap, &modEntry)) {
            do {
                if (std::wstring(modEntry.szModule) == modName) {
                    modBaseAddr = reinterpret_cast<uintptr_t>(modEntry.modBaseAddr);
                    break;
                }
            } while (Module32NextW(hSnap, &modEntry));
        }
        CloseHandle(hSnap);
    }
    return modBaseAddr;
}

float ReadHealthFromExternalProcess() {
    // Step 1: Get the process ID for gta_sa.exe
    DWORD processID = GetProcessID(L"gta_sa.exe");
    if (processID == 0) {
        std::cerr << "[Error] Could not find process gta_sa.exe\n";
        return 0.0f;
    }
    
    // Step 2: Open the process with the necessary permissions.
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "[Error] Failed to open process gta_sa.exe\n";
        return 0.0f;
    }
    
    // Step 3: Get the base address of gta_sa.exe in the target process.
    uintptr_t moduleBase = GetModuleBaseAddress(processID, L"gta_sa.exe");
    if (moduleBase == 0) {
        std::cerr << "[Error] Failed to get module base address for gta_sa.exe\n";
        CloseHandle(hProcess);
        return 0.0f;
    }
    
    // Pointer chain details:
    // Base pointer is at moduleBase + 0x77CD98 and then we add an offset of 0x540 for health.
    uintptr_t pointerAddress = moduleBase + 0x77CD98;
    uintptr_t intermediateAddress = 0;
    SIZE_T bytesRead = 0;
    
    // Step 4: Read the intermediate pointer from the target process.
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(pointerAddress), &intermediateAddress, sizeof(intermediateAddress), &bytesRead) || bytesRead != sizeof(intermediateAddress)) {
        std::cerr << "[Error] Failed to read memory at pointer address: 0x" << std::hex << pointerAddress << "\n";
        CloseHandle(hProcess);
        return 0.0f;
    }
    
    // Step 5: Calculate the final address for the Health value.
    uintptr_t healthAddress = intermediateAddress + 0x540;
    float health = 0.0f;
    
    // Step 6: Read the health value.
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(healthAddress), &health, sizeof(health), &bytesRead) || bytesRead != sizeof(health)) {
        std::cerr << "[Error] Failed to read health at address: 0x" << std::hex << healthAddress << "\n";
        CloseHandle(hProcess);
        return 0.0f;
    }
    
    CloseHandle(hProcess);
    return health;
}
