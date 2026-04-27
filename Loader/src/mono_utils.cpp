#include "mono_utils.h"
#include <psapi.h>
#include <iostream>

bool MonoUtils::GetMonoModule(HANDLE hProcess, uintptr_t& out_module) {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            TCHAR szModName[MAX_PATH];
            if (GetModuleBaseName(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                std::wstring name(szModName);
                if (name.find(L"mono") != std::wstring::npos && name.find(L".dll") != std::wstring::npos) {
                    out_module = (uintptr_t)hMods[i];
                    return true;
                }
            }
        }
    }
    return false;
}

bool MonoUtils::GetExports(HANDLE hProcess, uintptr_t mono_module, MonoExports& out_exports) {
    auto get_proc = [&](const char* name) -> uintptr_t {
        HMODULE local_mono = LoadLibraryExA("mono.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
        if (!local_mono) {
            local_mono = LoadLibraryA("mono.dll");
        }
        if (!local_mono) return 0;
        uintptr_t local_func = (uintptr_t)GetProcAddress(local_mono, name);
        uintptr_t offset = local_func - (uintptr_t)local_mono;
        FreeLibrary(local_mono);
        return mono_module + offset;
    };

    out_exports.mono_get_root_domain = get_proc("mono_get_root_domain");
    out_exports.mono_thread_attach = get_proc("mono_thread_attach");
    out_exports.mono_image_open_from_data = get_proc("mono_image_open_from_data");
    out_exports.mono_assembly_load_from_full = get_proc("mono_assembly_load_from_full");
    out_exports.mono_assembly_get_image = get_proc("mono_assembly_get_image");
    out_exports.mono_class_from_name = get_proc("mono_class_from_name");
    out_exports.mono_class_get_method_from_name = get_proc("mono_class_get_method_from_name");
    out_exports.mono_runtime_invoke = get_proc("mono_runtime_invoke");
    out_exports.mono_assembly_close = get_proc("mono_assembly_close");
    out_exports.mono_image_strerror = get_proc("mono_image_strerror");

    return out_exports.mono_get_root_domain != 0;
}

uintptr_t MonoUtils::Execute(HANDLE hProcess, uintptr_t address, const std::vector<uintptr_t>& args) {
    if (!address) return 0;

    void* remote_mem = VirtualAllocEx(hProcess, nullptr, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remote_mem) return 0;

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)address, (LPVOID)(args.empty() ? 0 : args[0]), 0, nullptr);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        DWORD exit_code;
        GetExitCodeThread(hThread, &exit_code);
        CloseHandle(hThread);
        VirtualFreeEx(hProcess, remote_mem, 0, MEM_RELEASE);
        return (uintptr_t)exit_code;
    }

    VirtualFreeEx(hProcess, remote_mem, 0, MEM_RELEASE);
    return 0;
}
