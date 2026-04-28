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
    
    std::ofstream out("payload.h");
    out << "#pragma once\n#include <cstdint>\n\n";
    out << "namespace yan_data {\n";
    out << "    inline uint8_t ghost_key[16] = { ";
    for (int i = 0; i < 16; ++i) out << (int)key[i] << (i == 15 ? "" : ", ");
    out << " };\n";
    out << "    inline uint8_t blob[] = { ";
    for (size_t i = 0; i < buf.size(); ++i) {
        out << (int)buf[i] << (i == buf.size() - 1 ? "" : ", ");
        if (i % 20 == 19) out << "\n        ";
    }
    out << " };\n";
    out << "    inline size_t blob_size = sizeof(blob);\n";
    out << "}\n";
    out.close();

    set_color(10);
    std::cout << "\n[+] payload.h created. Move this to Loader/src/ and rebuild." << std::endl;
    set_color(7);
    system("pause > nul");
    return 0;
}
