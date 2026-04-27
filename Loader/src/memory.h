#pragma once
#include <windows.h>
#include <string>
#include <vector>

class Memory {
public:
    Memory(HANDLE process_handle);
    ~Memory();

    uintptr_t Allocate(size_t size, DWORD protection = PAGE_EXECUTE_READWRITE);
    bool Free(uintptr_t address);
    bool Write(uintptr_t address, const void* buffer, size_t size);
    bool Read(uintptr_t address, void* buffer, size_t size);

    uintptr_t AllocateAndWrite(const void* buffer, size_t size);
    uintptr_t AllocateAndWrite(const std::string& str);
    uintptr_t AllocateAndWrite(const std::vector<uint8_t>& data);

    template<typename T>
    T Read(uintptr_t address) {
        T value;
        Read(address, &value, sizeof(T));
        return value;
    }

private:
    HANDLE hProcess;
    std::vector<uintptr_t> allocations;
};
