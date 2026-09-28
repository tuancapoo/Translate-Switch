#include "core/storage.hpp"

#include <tesla.hpp>
#include <cstdio>
#include <string>
#include <sys/stat.h>

namespace storage {

    namespace {

        // Giả định SD đã được mount
        void makeDirs(const std::string& path) {
            for (size_t pos = path.find('/', path.find(":/") + 2); pos != std::string::npos; pos = path.find('/', pos + 1))
                mkdir(path.substr(0, pos).c_str(), 0777);
            mkdir(path.c_str(), 0777);
        }

        std::string parentDir(const char* path) {
            std::string p = path;
            return p.substr(0, p.find_last_of('/'));
        }

    }

    void ensureDir(const char* path) {
        tsl::hlp::doWithSDCardHandle([&] { makeDirs(path); });
    }

    bool writeFile(const char* path, const void* data, size_t size) {
        bool ok = false;
        tsl::hlp::doWithSDCardHandle([&] {
            makeDirs(parentDir(path));

            FILE* file = fopen(path, "wb");
            if (file == nullptr)
                return;
            ok = fwrite(data, 1, size, file) == size;
            fclose(file);
        });
        return ok;
    }

}
