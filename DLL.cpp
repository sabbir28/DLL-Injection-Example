#include <windows.h>
#include <iostream>
#include <string>
#include <cstdio>      // For printf and freopen
#include <cstdint>     // For uintptr_t

#include "text.h"    // Contains all memory addresses (e.g., PLAYING_TIME)
#include "model/health.h"

// Function prototypes
int ReadMemory(uintptr_t address);
void WriteMemory(uintptr_t address, int newValue);
void CommandHandler(const std::string& command);
void CreateConsole();
DWORD WINAPI ConsoleThread(LPVOID lpParam);

// Function to handle console commands
void CommandHandler(const std::string& command) {
    if (command == "help") {
        std::cout << "Available commands:\n";
        std::cout << "  help   - Show this help message\n";
        std::cout << "  time   - Read playing time\n";
        std::cout << "  health - Read current health value\n";
        std::cout << "  exit   - Close the console\n";
    } else if (command == "time") {
        std::cout << "[INFO] Counting Time...\n";
        int timeValue = ReadMemory(PLAYING_TIME);
        std::cout << "[INFO] Playing Time: " << timeValue << " seconds\n";
    } else if (command == "health") {
        std::cout << "[INFO] Reading Health...\n";
        int healthValue = ReadHealthFromExternalProcess();
        std::cout << "[INFO] Health: " << healthValue << "\n";
    } else if (command == "exit") {
        std::cout << "Closing console...\n";
        FreeConsole();
        ExitProcess(0);
    } else {
        std::cout << "Unknown command: " << command << "\n";
    }
}

// Function to create and manage console input
void CreateConsole() {
    if (AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONIN$", "r", stdin);

        std::cout << "Console successfully initialized!\n";
        std::cout << "Type 'help' for a list of commands.\n\n";

        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);
            CommandHandler(input);
        }
    } else {
        std::cout << "Failed to allocate console.\n";
    }
}

// Function to safely read memory from an address
int ReadMemory(uintptr_t address) {
    int value = 0;

    if (IsBadReadPtr(reinterpret_cast<void*>(address), sizeof(int))) {  // Check if memory is accessible
        printf("[ReadMemory] Failed to read memory at 0x%p (Access Violation?)\n", reinterpret_cast<void*>(address));
        return 0;
    }

    value = *reinterpret_cast<int*>(address);
    printf("[ReadMemory] Address: 0x%p, Value: %d\n", reinterpret_cast<void*>(address), value);
    
    return value;
}

// Function to safely write a value to an address
void WriteMemory(uintptr_t address, int newValue) {
    DWORD oldProtect;

    if (IsBadWritePtr(reinterpret_cast<void*>(address), sizeof(int))) {  // Check if memory is writable
        printf("[WriteMemory] Failed to write memory at 0x%p (Access Violation?)\n", reinterpret_cast<void*>(address));
        return;
    }

    // Change memory protection to allow writing
    if (VirtualProtect(reinterpret_cast<LPVOID>(address), sizeof(int), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *reinterpret_cast<int*>(address) = newValue;
        printf("[WriteMemory] Address: 0x%p, New Value: %d\n", reinterpret_cast<void*>(address), newValue);
        VirtualProtect(reinterpret_cast<LPVOID>(address), sizeof(int), oldProtect, &oldProtect);  // Restore protection
    } else {
        printf("[WriteMemory] Failed to change memory protection at 0x%p\n", reinterpret_cast<void*>(address));
    }
}

// Thread function for console management
DWORD WINAPI ConsoleThread(LPVOID lpParam) {
    CreateConsole();
    return 0;
}

// DLL entry point
BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        // Prevent DLL_THREAD_ATTACH and DLL_THREAD_DETACH notifications
        DisableThreadLibraryCalls(hinstDll);

        // Start console thread
        HANDLE hConsoleThread = CreateThread(NULL, 0, ConsoleThread, NULL, 0, NULL);
        if (hConsoleThread) {
            CloseHandle(hConsoleThread);
        }
    }
    return TRUE;
}
