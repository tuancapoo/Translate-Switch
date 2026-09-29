#include "core/settings.hpp"

#include <tesla.hpp>
#include <cstdio>

namespace settings {

    namespace {

        std::string trim(const std::string& s) {
            const char* spaces = " \t\r\n";
            size_t start = s.find_first_not_of(spaces);
            if (start == std::string::npos)
                return "";
            return s.substr(start, s.find_last_not_of(spaces) - start + 1);
        }

    }

    std::string load(const char* path, Settings& out) {
        bool found = false;
        Settings result;

        tsl::hlp::doWithSDCardHandle([&] {
            FILE* file = fopen(path, "r");
            if (file == nullptr)
                return;
            found = true;

            char buffer[512];
            while (fgets(buffer, sizeof(buffer), file) != nullptr) {
                std::string line = trim(buffer);
                if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[')
                    continue;

                size_t eq = line.find('=');
                if (eq == std::string::npos)
                    continue;

                std::string key = trim(line.substr(0, eq));
                std::string value = trim(line.substr(eq + 1));
                if (key == "url")
                    result.url = value;
                else if (key == "token")
                    result.token = value;
                else if (key == "lang" && !value.empty())
                    result.lang = value;
            }
            fclose(file);
        });

        if (!found)
            return "Thiếu file config.ini";
        if (result.url.empty())
            return "config.ini thiếu url";
        if (result.token.empty())
            return "config.ini thiếu token";

        out = result;
        return "";
    }

}
