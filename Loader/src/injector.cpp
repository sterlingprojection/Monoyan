#include "injector.h"
#include <iostream>
#include <fstream>

Injector::Injector(DWORD process_id) {
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (hProcess) {
        memory = new Memory(hProcess);
        Initialize();
    }
}

Injector::~Injector() {
    if (memory) delete memory;
    if (hProcess) CloseHandle(hProcess);
}

bool Injector::Initialize() {
    if (!MonoUtils::GetMonoModule(hProcess, mono_module)) return false;
    if (!MonoUtils::GetExports(hProcess, mono_module, exports)) return false;
    return true;
}

bool Injector::Inject(const std::string& assembly_path, const std::string& name_space, const std::string& class_name, const std::string& method_name) {
    std::ifstream file(assembly_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(size);
    if (!file.read((char*)buffer.data(), size)) return false;

    uintptr_t root_domain = CallRemote(exports.mono_get_root_domain, {});
    CallRemote(exports.mono_thread_attach, { root_domain });

    uintptr_t status_ptr = memory->Allocate(4);
    uintptr_t raw_data_ptr = memory->AllocateAndWrite(buffer);
    uintptr_t image = CallRemote(exports.mono_image_open_from_data, { raw_data_ptr, (uintptr_t)size, 1, status_ptr });

    uintptr_t assembly = CallRemote(exports.mono_assembly_load_from_full, { image, memory->AllocateAndWrite(""), status_ptr, 0 });

    uintptr_t image_ptr = CallRemote(exports.mono_assembly_get_image, { assembly });
    uintptr_t class_ptr = CallRemote(exports.mono_class_from_name, { image_ptr, memory->AllocateAndWrite(name_space), memory->AllocateAndWrite(class_name) });
    uintptr_t method_ptr = CallRemote(exports.mono_class_get_method_from_name, { class_ptr, memory->AllocateAndWrite(method_name), 0 });

    CallRemote(exports.mono_runtime_invoke, { method_ptr, 0, 0, 0 });

    return true;
}

bool Injector::InjectFromMemory(const std::vector<uint8_t>& buffer, const std::string& name_space, const std::string& class_name, const std::string& method_name) {
    if (buffer.empty()) return false;

    uintptr_t root_domain = CallRemote(exports.mono_get_root_domain, {});
    CallRemote(exports.mono_thread_attach, { root_domain });

    uintptr_t status_ptr = memory->Allocate(4);
    uintptr_t raw_data_ptr = memory->AllocateAndWrite(buffer);
    uintptr_t image = CallRemote(exports.mono_image_open_from_data, { raw_data_ptr, (uintptr_t)buffer.size(), 1, status_ptr });

    uintptr_t assembly = CallRemote(exports.mono_assembly_load_from_full, { image, memory->AllocateAndWrite(""), status_ptr, 0 });

    uintptr_t image_ptr = CallRemote(exports.mono_assembly_get_image, { assembly });
    uintptr_t class_ptr = CallRemote(exports.mono_class_from_name, { image_ptr, memory->AllocateAndWrite(name_space), memory->AllocateAndWrite(class_name) });
    uintptr_t method_ptr = CallRemote(exports.mono_class_get_method_from_name, { class_ptr, memory->AllocateAndWrite(method_name), 0 });

    CallRemote(exports.mono_runtime_invoke, { method_ptr, 0, 0, 0 });

    return true;
}

uintptr_t Injector::CallRemote(uintptr_t address, const std::vector<uintptr_t>& args) {
    return MonoUtils::Execute(hProcess, address, args);
}

bool Injector::InjectFromRemoteMemory(uintptr_t remote_address, size_t size, const std::string& name_space, const std::string& class_name, const std::string& method_name) {
    if (!remote_address || size == 0) return false;

    uintptr_t root_domain = CallRemote(exports.mono_get_root_domain, {});
    CallRemote(exports.mono_thread_attach, { root_domain });

    uintptr_t status_ptr = memory->Allocate(4);
    uintptr_t image = CallRemote(exports.mono_image_open_from_data, { remote_address, (uintptr_t)size, 1, status_ptr });

    uintptr_t assembly = CallRemote(exports.mono_assembly_load_from_full, { image, memory->AllocateAndWrite(""), status_ptr, 0 });

    uintptr_t image_ptr = CallRemote(exports.mono_assembly_get_image, { assembly });
    uintptr_t class_ptr = CallRemote(exports.mono_class_from_name, { image_ptr, memory->AllocateAndWrite(name_space), memory->AllocateAndWrite(class_name) });
    uintptr_t method_ptr = CallRemote(exports.mono_class_get_method_from_name, { class_ptr, memory->AllocateAndWrite(method_name), 0 });

    CallRemote(exports.mono_runtime_invoke, { method_ptr, 0, 0, 0 });

    return true;
}
