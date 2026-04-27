#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "memory.h"
#include "mono_utils.h"

class Injector {
public:
    Injector(DWORD process_id);
    ~Injector();

    bool Inject(const std::string& assembly_path, const std::string& name_space, const std::string& class_name, const std::string& method_name);
    bool InjectFromMemory(const std::vector<uint8_t>& buffer, const std::string& name_space, const std::string& class_name, const std::string& method_name);
    bool InjectFromRemoteMemory(uintptr_t remote_address, size_t size, const std::string& name_space, const std::string& class_name, const std::string& method_name);
    bool Eject(uintptr_t assembly_ptr, const std::string& name_space, const std::string& class_name, const std::string& method_name);

    Memory* GetMemory() { return memory; }

private:
    HANDLE hProcess;
    Memory* memory;
    uintptr_t mono_module;
    MonoExports exports;

    bool Initialize();
    uintptr_t CallRemote(uintptr_t address, const std::vector<uintptr_t>& args);
};
