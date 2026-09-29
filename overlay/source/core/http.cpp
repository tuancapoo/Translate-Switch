#include "core/http.hpp"

#include <curl/curl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

namespace http {

    namespace {

        // Buffer nhỏ hơn mặc định để tiết kiệm RAM của overlay
        constexpr SocketInitConfig SocketConfig = {
            .tcp_tx_buf_size     = 0x8000,
            .tcp_rx_buf_size     = 0x10000,
            .tcp_tx_buf_max_size = 0x40000,
            .tcp_rx_buf_max_size = 0x40000,
            .udp_tx_buf_size     = 0x2400,
            .udp_rx_buf_size     = 0xA500,
            .sb_efficiency       = 2,
            .num_bsd_sessions    = 3,
            .bsd_service_type    = BsdServiceType_User,
        };

        // Chặn phản hồi quá lớn (JSON bản dịch thường chỉ vài KB)
        constexpr size_t MaxBodySize = 256 * 1024;

        size_t onWrite(char* ptr, size_t size, size_t count, void* userdata) {
            auto* body = static_cast<std::string*>(userdata);
            size_t bytes = size * count;
            if (body->size() + bytes > MaxBodySize)
                return 0; // curl sẽ báo lỗi CURLE_WRITE_ERROR
            body->append(ptr, bytes);
            return bytes;
        }

        bool s_socketReady = false;
        bool s_servicesReady = false;
        bool s_curlReady = false;

    }

    Result init() {
        Result rc = socketInitialize(&SocketConfig);
        if (R_FAILED(rc))
            return rc;
        s_socketReady = true;

        // curl (TLS backend libnx) tự mở các service này, có thể từ thread nền khi không còn sm session.
        // Mở trước ở đây (đang có sm session) để các lần mở sau của curl chỉ tăng refcount.
        if (R_FAILED(rc = sslInitialize(3)) || R_FAILED(rc = nifmInitialize(NifmServiceType_User)) || R_FAILED(rc = csrngInitialize()))
            return rc;
        s_servicesReady = true;

        s_curlReady = curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
        return 0;
    }

    void exit() {
        if (s_curlReady)
            curl_global_cleanup();
        if (s_servicesReady) {
            csrngExit();
            nifmExit();
            sslExit();
        }
        if (s_socketReady)
            socketExit();
        s_curlReady = s_servicesReady = s_socketReady = false;
    }

    std::string diagnose(const std::string& url) {
        std::string report;

        NifmInternetConnectionType type{};
        NifmInternetConnectionStatus status{};
        u32 strength = 0;
        Result rc = nifmGetInternetConnectionStatus(&type, &strength, &status);
        if (R_FAILED(rc)) {
            report = "Mạng: chưa kết nối";
        } else {
            report = type == NifmInternetConnectionType_Ethernet ? "LAN" : "Wi-Fi " + std::to_string(strength) + "/3";
            report += status == NifmInternetConnectionStatus_Connected ? " · Internet OK" : " · chưa có Internet";
        }

        u32 ip = 0;
        if (R_SUCCEEDED(nifmGetCurrentIpAddress(&ip)) && ip != 0) {
            in_addr addr{ .s_addr = ip };
            report += std::string(" · IP ") + inet_ntoa(addr);
        }

        // Lấy host từ url: https://host/path
        size_t start = url.find("://");
        start = start == std::string::npos ? 0 : start + 3;
        std::string host = url.substr(start, url.find_first_of(":/", start) - start);

        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo* result = nullptr;
        int err = getaddrinfo(host.c_str(), "443", &hints, &result);
        if (err == 0 && result != nullptr) {
            auto* resolved = reinterpret_cast<sockaddr_in*>(result->ai_addr);
            report += std::string(" · DNS OK ") + inet_ntoa(resolved->sin_addr);
            freeaddrinfo(result);
        } else {
            report += " · DNS lỗi " + std::to_string(err) + " (" + gai_strerror(err) + ")";
        }

        return report;
    }

    Response post(const std::string& url, const std::vector<std::string>& headers,
                  const void* data, size_t size, long timeoutSec) {
        Response response;
        if (!s_curlReady) {
            response.error = "network not initialized";
            return response;
        }

        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            response.error = "curl_easy_init failed";
            return response;
        }

        curl_slist* headerList = nullptr;
        for (const auto& header : headers)
            headerList = curl_slist_append(headerList, header.c_str());

        char errorBuffer[CURL_ERROR_SIZE] = {};

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(size));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSec);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "translate-switch-overlay");

        CURLcode code = curl_easy_perform(curl);
        if (code == CURLE_OK)
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);
        else
            response.error = errorBuffer[0] != '\0' ? errorBuffer : curl_easy_strerror(code);

        curl_slist_free_all(headerList);
        curl_easy_cleanup(curl);
        return response;
    }

}
