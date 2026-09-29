#include "core/translate_client.hpp"

#include "config.hpp"
#include "core/http.hpp"

#include <jansson.h>

namespace translate_client {

    namespace {

        /// jansson tự tạo seed bằng cách open("/dev/urandom") -> crash trên Switch.
        /// Cấp seed trước (khác 0) để jansson bỏ qua bước đó.
        void ensureJsonSeed() {
            static bool seeded = false;
            if (seeded)
                return;

            // jansson chỉ dùng 32 bit thấp và coi 0 là "chưa có seed"
            u32 seed = 0;
            randomGet(&seed, sizeof(seed));
            json_object_seed(seed | 1);
            seeded = true;
        }

        int intField(json_t* object, const char* key) {
            json_t* value = json_object_get(object, key);
            return json_is_integer(value) ? static_cast<int>(json_integer_value(value)) : 0;
        }

        std::string stringField(json_t* object, const char* key) {
            const char* value = json_string_value(json_object_get(object, key));
            return value != nullptr ? value : "";
        }

        /// Lấy trường "error" trong JSON lỗi của server, nếu có
        std::string serverError(const std::string& body) {
            json_t* root = json_loadb(body.data(), body.size(), 0, nullptr);
            std::string message = root != nullptr ? stringField(root, "error") : "";
            json_decref(root);
            return message;
        }

    }

    std::string translate(const settings::Settings& settings, const u8* jpeg, size_t size, Translation& out) {
        ensureJsonSeed();

        http::Response response = http::post(
            settings.url,
            {
                "Content-Type: image/jpeg",
                "x-app-token: " + settings.token,
                "x-target-lang: " + settings.lang,
            },
            jpeg, size, config::TranslateTimeoutSec);

        if (!response.error.empty())
            return "Mạng: " + response.error;

        if (response.status != 200) {
            std::string message = serverError(response.body);
            return "Server " + std::to_string(response.status) + (message.empty() ? "" : ": " + message);
        }

        json_error_t jsonError;
        json_t* root = json_loadb(response.body.data(), response.body.size(), 0, &jsonError);
        if (root == nullptr)
            return std::string("JSON lỗi: ") + jsonError.text;

        json_t* lines = json_object_get(root, "lines");
        if (!json_is_array(lines)) {
            json_decref(root);
            return "JSON thiếu lines";
        }

        Translation result;
        result.provider = stringField(root, "provider");

        size_t index;
        json_t* item;
        json_array_foreach(lines, index, item) {
            TextLine line;
            line.x = intField(item, "x");
            line.y = intField(item, "y");
            line.w = intField(item, "w");
            line.h = intField(item, "h");
            line.source = stringField(item, "source");
            line.translation = stringField(item, "translation");
            if (!line.translation.empty())
                result.lines.push_back(std::move(line));
        }

        json_decref(root);
        out = std::move(result);
        return "";
    }

}
