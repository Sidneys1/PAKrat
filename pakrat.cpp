#include <filesystem>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <cerrno>
#include <vector>
#include <tuple>

struct prelude {
    uint32_t count;
    struct entry {
        char name[64];
        uint32_t offset;
    } entries[];
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " FILE\n";
        return 1;
    }
    std::cout << ("PAKrat 0.1\n");


    std::ifstream file(argv[1], std::fstream::in | std::fstream::binary);
    if (!file) {
        std::cerr << "Error opening '" << argv[1] << "': " << std::strerror(errno) << "\n";
        return 1;
    }

    file.seekg(0, std::ifstream::end);
    size_t file_size = file.tellg();
    std::cout << "File '" << argv[1] << "' size: " << file_size << " Bytes\n";
    file.seekg(0, std::ifstream::beg);

    // Let's start by getting the number of entries, so we know how large a buffer to allocate
    char* buffer = (char*)malloc(sizeof(uint32_t));
    file.read((char*)buffer, sizeof(uint32_t));
    uint32_t count = *(uint32_t*)buffer;
    std::cout << "File contains " << *(uint32_t*)buffer << " entries:\n";

    // Reallocate to the appropriate size.
    buffer = (char*)realloc((void*)buffer, sizeof(prelude) + (sizeof(prelude::entry) * count));
    file.read(buffer + 4, sizeof(prelude::entry) * count);

    // Interpret by casting to a prelude, gather, then print all the files and their offsets.
    prelude* header = (prelude*)buffer;
    std::vector<std::tuple<char*, uint32_t, uint32_t>> entries;
    for (auto i = 1; i < header->count; ++i) {
        auto& entry = header->entries[i];
        auto &prev = header->entries[i - 1];
        entries.push_back(std::make_tuple(prev.name, prev.offset, entry.offset - prev.offset));
    }
    auto& last = header->entries[header->count - 1];
    entries.push_back(std::make_tuple(last.name, last.offset, file_size - last.offset));

    for (auto& entry : entries) {
        std::ios old_state(nullptr);
        old_state.copyfmt(std::cout);
        std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << std::get<1>(entry) 
                  << " " << std::get<0>(entry) << " ";
        std::cout.copyfmt(old_state);
        std::cout << std::get<2>(entry) << " Bytes\n";
    }

    // Extract files
    for (auto& entry : entries) {
        char* path_str = std::get<0>(entry);
        uint32_t offset = std::get<1>(entry);
        uint32_t length = std::get<2>(entry);

        // Replace Windows path separators
        std::replace(path_str, path_str + strlen(path_str), '\\', '/');

        std::filesystem::path path(path_str);
        auto filename = path.filename();
        auto parent = path.parent_path();

        // Create parent folder(s) (if needed)
        if (!parent.empty() && !std::filesystem::exists(parent)) {
            std::cout << "Creating directory " << std::quoted(parent.c_str()) << '\n';
            std::filesystem::create_directories(parent);
        }

        std::cout << "Creating file " << std::quoted(path_str) << '\n';
        std::ofstream out_file(path, std::fstream::out | std::fstream::binary);
        if (!out_file) {
            std::cerr << "Error creating file: " << std::strerror(errno) << "\n";
            continue;
        }

        // Seek to the correct location and copy the file in 1KiB chunks
        file.seekg(offset, std::ifstream::beg);
        uint32_t to_read = length;
        do {
            char buffer[1024];
            auto chunk = std::min((size_t)to_read, sizeof(buffer));
            to_read -= chunk;
            file.read(buffer, chunk);
            out_file.write(buffer, chunk);
        } while (to_read > 0);
    }
}
