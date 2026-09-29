#include "gui/text_layout.hpp"

#include <tesla.hpp>

namespace text_layout {

    namespace {

        int measure(const std::string& text, float fontSize) {
            auto [width, height] = tsl::gfx::Renderer::measureString(text.c_str(), false, fontSize);
            return static_cast<int>(width);
        }

        void wrapParagraph(const std::string& paragraph, float fontSize, int maxWidth, std::vector<std::string>& out) {
            std::string line;
            size_t pos = 0;

            while (pos <= paragraph.size()) {
                size_t space = paragraph.find(' ', pos);
                std::string word = paragraph.substr(pos, space == std::string::npos ? std::string::npos : space - pos);
                pos = space == std::string::npos ? paragraph.size() + 1 : space + 1;

                std::string candidate = line.empty() ? word : line + " " + word;
                if (line.empty() || measure(candidate, fontSize) <= maxWidth) {
                    line = std::move(candidate);
                } else {
                    out.push_back(std::move(line));
                    line = word;
                }
            }

            out.push_back(std::move(line));
        }

    }

    std::vector<std::string> wrap(const std::string& text, float fontSize, int maxWidth) {
        std::vector<std::string> lines;
        size_t start = 0;
        while (true) {
            size_t newline = text.find('\n', start);
            wrapParagraph(text.substr(start, newline == std::string::npos ? std::string::npos : newline - start),
                          fontSize, maxWidth, lines);
            if (newline == std::string::npos)
                break;
            start = newline + 1;
        }
        return lines;
    }

}
