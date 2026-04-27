#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <vector>

struct MonoExports {
    uintptr_t mono_get_root_domain;
    uintptr_t mono_thread_attach;
    uintptr_t mono_image_open_from_data;
    uintptr_t mono_assembly_load_from_full;
    uintptr_t mono_assembly_get_image;
    uintptr_t mono_class_from_name;
    uintptr_t mono_class_get_method_from_name;
    uintptr_t mono_runtime_invoke;
    uintptr_t mono_assembly_close;
    uintptr_t mono_image_strerror;
};

class MonoUtils {
public:
    static bool GetMonoModule(HANDLE hProcess, uintptr_t& out_module);
    static bool GetExports(HANDLE hProcess, uintptr_t mono_module, MonoExports& out_exports);
    
    static uintptr_t Execute(HANDLE hProcess, uintptr_t address, const std::vector<uintptr_t>& args);
};
