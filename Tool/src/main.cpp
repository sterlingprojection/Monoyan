#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <filesystem>
#include <random>
#include <intrin.h>

namespace fs = std::filesystem;

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void process_data(std::vector<uint8_t>& data, const uint8_t* key) {
    uint8_t r = key[0];
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= key[i % 16];
        data[i] = (data[i] + r) % 256;
        data[i] = (data[i] << 4) | (data[i] >> 4);
        r = data[i];
    }
}

std::string to_script(const std::vector<uint8_t>& data) {
    std::string out;
    for (uint8_t b : data) {
        uint16_t c = 0x4E00 + b; 
        out += (char)(0xE0 | (c >> 12));
        out += (char)(0x80 | ((c >> 6) & 0x3F));
        out += (char)(0x80 | (c & 0x3F));
    }
    return out;
}

int main(int argc, char* argv[]) {
    SetConsoleTitleA("Tool");
    set_color(13);
    std::cout << "=============================================" << std::endl;
    std::cout << "                MONOYAN TOOL                 " << std::endl;
    std::cout << "=============================================" << std::endl;
    set_color(7);
    if (argc < 2) {
        set_color(12);
        std::cout << "\n[!] Drop DLL onto EXE." << std::endl;
        system("pause");
        return 1;
    }
    std::string path = argv[1];
    if (!fs::exists(path)) return 1;
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    std::streamsize sz = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> buf(sz);
    f.read((char*)buf.data(), sz);
    f.close();
    uint8_t key[16];
    std::mt19937 rng((unsigned int)__rdtsc());
    for (int i = 0; i < 16; ++i) key[i] = (uint8_t)(rng() % 256);
    process_data(buf, key);
    std::vector<uint8_t> blob;
    blob.insert(blob.end(), key, key + 16);
    uint32_t total = (uint32_t)sz;
    uint8_t* p_sz = (uint8_t*)&total;
    blob.insert(blob.end(), p_sz, p_sz + 4);
    blob.insert(blob.end(), buf.begin(), buf.end());
    std::string script = to_script(blob);
    std::ofstream out("payload.yan", std::ios::binary);
    out.write(script.c_str(), script.size());
    out.close();
    set_color(10);
    std::cout << "\n[+] payload.yan created." << std::endl;
    set_color(7);
    system("pause > nul");
    return 0;
}
