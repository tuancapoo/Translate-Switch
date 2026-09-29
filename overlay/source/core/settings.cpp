#include "core/settings.hpp"

#include <tesla.hpp>
#include <cstdio>
#include <vector>

namespace settings {

    namespace {

        std::string trim(const std::string& s) {
            const char* spaces = " \t\r\n";
            size_t start = s.find_first_not_of(spaces);
            if (start == std::string::npos)
                return "";
            return s.substr(start, s.find_last_not_of(spaces) - start + 1);
        }

        /// Đọc toàn bộ các dòng của file (false nếu không mở được). Giả định SD đã được mount.
        bool readLines(const char* path, std::vector<std::string>& lines) {
            FILE* file = fopen(path, "r");
            if (file == nullptr)
                return false;

            char buffer[512];
            while (fgets(buffer, sizeof(buffer), file) != nullptr) {
                std::string line = buffer;
                while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                    line.pop_back();
                lines.push_back(std::move(line));
            }
            fclose(file);
            return true;
        }

        /// Tách "key=value", bỏ qua dòng trống / comment / [section]
        bool parseLine(const std::string& raw, std::string& key, std::string& value) {
            std::string line = trim(raw);
            if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[')
                return false;

            size_t eq = line.find('=');
            if (eq == std::string::npos)
                return false;

            key = trim(line.substr(0, eq));
            value = trim(line.substr(eq + 1));
            return true;
        }

    }

    std::string load(const char* path, Settings& out) {
        bool found = false;
        Settings result;

        tsl::hlp::doWithSDCardHandle([&] {
            std::vector<std::string> lines;
            found = readLines(path, lines);

            for (const auto& line : lines) {
                std::string key, value;
                if (!parseLine(line, key, value))
                    continue;
                if (key == "url")
                    result.url = value;
                else if (key == "token")
                    result.token = value;
                else if (key == "lang" && !value.empty())
                    result.lang = value;
            }
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

    std::string readValue(const char* path, const std::string& wanted) {
        std::string result;
        tsl::hlp::doWithSDCardHandle([&] {
            std::vector<std::string> lines;
            readLines(path, lines);
            for (const auto& line : lines) {
                std::string key, value;
                if (parseLine(line, key, value) && key == wanted)
                    result = value;
            }
        });
        return result;
    }

    bool writeValue(const char* path, const std::string& wanted, const std::string& newValue) {
        bool ok = false;
        tsl::hlp::doWithSDCardHandle([&] {
            std::vector<std::string> lines;
            readLines(path, lines);

            bool replaced = false;
            for (auto& line : lines) {
                std::string key, value;
                if (parseLine(line, key, value) && key == wanted) {
                    line = wanted + "=" + newValue;
                    replaced = true;
                }
            }
            if (!replaced)
                lines.push_back(wanted + "=" + newValue);

            FILE* file = fopen(path, "w");
            if (file == nullptr)
                return;
            ok = true;
            for (const auto& line : lines)
                ok &= fprintf(file, "%s\n", line.c_str()) >= 0;
            fclose(file);
        });
        return ok;
    }

}
