#include "memory.h"
#include <stdexcept>

Memory::Memory(HANDLE process_handle) : hProcess(process_handle) {}

Memory::~Memory() {
    for (auto addr : allocations) {
        VirtualFreeEx(hProcess, (LPVOID)addr, 0, MEM_RELEASE);
    }
}

uintptr_t Memory::Allocate(size_t size, DWORD protection) {
    LPVOID addr = VirtualAllocEx(hProcess, nullptr, size, MEM_COMMIT | MEM_RESERVE, protection);
    if (!addr) return 0;
    allocations.push_back((uintptr_t)addr);
    return (uintptr_t)addr;
}

bool Memory::Free(uintptr_t address) {
    return VirtualFreeEx(hProcess, (LPVOID)address, 0, MEM_RELEASE);
}

bool Memory::Write(uintptr_t address, const void* buffer, size_t size) {
    SIZE_T written;
    return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &written) && written == size;
}

bool Memory::Read(uintptr_t address, void* buffer, size_t size) {
    SIZE_T read;
    return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &read) && read == size;
}

uintptr_t Memory::AllocateAndWrite(const void* buffer, size_t size) {
    uintptr_t addr = Allocate(size);
    if (addr && Write(addr, buffer, size)) {
        return addr;
    }
    return 0;
}

uintptr_t Memory::AllocateAndWrite(const std::string& str) {
    return AllocateAndWrite(str.c_str(), str.length() + 1);
}

uintptr_t Memory::AllocateAndWrite(const std::vector<uint8_t>& data) {
    return AllocateAndWrite(data.data(), data.size());
}
