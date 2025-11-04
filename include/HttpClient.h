#ifndef PERPLEXITY_CPP_HTTPCLIENT_H
#define PERPLEXITY_CPP_HTTPCLIENT_H
#pragma once

#include <curl/curl.h>
#include <string>
#include <memory>
#include <stdexcept>
#include "exceptions.h"
#include "config.h"

namespace perplexity {

/**
 * @brief RAII wrapper for libcurl
 *
 * Applies the RAII pattern for automatic CURL resource management.
 */
class HttpClient {
private:
    CURL* curl_ = nullptr;
    curl_slist* headers_ = nullptr;
    std::string response_buffer_;
    long response_code_ = 0;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t total_size = size * nmemb;
        std::string* buffer = static_cast<std::string*>(userp);
        buffer->append(static_cast<char*>(contents), total_size);
        return total_size;
    }

    void initialize(const Config& config) {
        curl_ = curl_easy_init();
        if (!curl_) {
            throw NetworkError("Failed to initialize CURL");
        }

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_buffer_);
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT, config.get_timeout().count());

        // SSL verification
        if (!config.should_verify_ssl()) {
            curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        // Proxy
        if (config.get_proxy()) {
            curl_easy_setopt(curl_, CURLOPT_PROXY, config.get_proxy()->c_str());
        }

        // User-Agent
        std::string user_agent = config.get_user_agent().value_or("perplexity-cpp/1.0");
        curl_easy_setopt(curl_, CURLOPT_USERAGENT, user_agent.c_str());

        // Follow redirects
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 5L);
    }

    void cleanup() {
        if (headers_) {
            curl_slist_free_all(headers_);
            headers_ = nullptr;
        }
        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
    }

public:

    explicit HttpClient(const Config& config) {
        initialize(config);
    }

    ~HttpClient() {
        cleanup();
    }

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpClient(HttpClient&& other) noexcept
        : curl_(other.curl_)
        , headers_(other.headers_)
        , response_buffer_(std::move(other.response_buffer_))
        , response_code_(other.response_code_) {
        other.curl_ = nullptr;
        other.headers_ = nullptr;
    }

    HttpClient& operator=(HttpClient&& other) noexcept {
        if (this != &other) {
            cleanup();
            curl_ = other.curl_;
            headers_ = other.headers_;
            response_buffer_ = std::move(other.response_buffer_);
            response_code_ = other.response_code_;
            other.curl_ = nullptr;
            other.headers_ = nullptr;
        }
        return *this;
    }

    void add_header(const std::string& header) {
        headers_ = curl_slist_append(headers_, header.c_str());
    }

    // POST request
    std::string post(const std::string& url, const std::string& data) {
        if (!curl_) {
            throw NetworkError("CURL not initialized");
        }

        response_buffer_.clear();

        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);

        CURLcode res = curl_easy_perform(curl_);

        if (res != CURLE_OK) {
            std::string error_msg = curl_easy_strerror(res);

            if (res == CURLE_OPERATION_TIMEDOUT) {
                throw TimeoutError("Request timed out: " + error_msg);
            }
            throw NetworkError("CURL request failed: " + error_msg);
        }

        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code_);

        return response_buffer_;
    }

    // GET request
    std::string get(const std::string& url) {
        if (!curl_) {
            throw NetworkError("CURL not initialized");
        }

        response_buffer_.clear();

        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);

        CURLcode res = curl_easy_perform(curl_);

        if (res != CURLE_OK) {
            std::string error_msg = curl_easy_strerror(res);

            if (res == CURLE_OPERATION_TIMEDOUT) {
                throw TimeoutError("Request timed out: " + error_msg);
            }
            throw NetworkError("CURL request failed: " + error_msg);
        }

        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code_);

        return response_buffer_;
    }

    long get_response_code() const {
        return response_code_;
    }

    void reset() {
        response_buffer_.clear();
        response_code_ = 0;

        if (headers_) {
            curl_slist_free_all(headers_);
            headers_ = nullptr;
        }
    }
};

/**
 * @brief RAII initializer for libcurl (global)
 *
 * Must be created once at the start of the program.
 */
class CurlGlobalInit {
public:
    CurlGlobalInit() {
        CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
        if (res != CURLE_OK) {
            throw NetworkError("Failed to initialize CURL globally");
        }
    }

    ~CurlGlobalInit() {
        curl_global_cleanup();
    }

    CurlGlobalInit(const CurlGlobalInit&) = delete;
    CurlGlobalInit& operator=(const CurlGlobalInit&) = delete;
    CurlGlobalInit(CurlGlobalInit&&) = delete;
    CurlGlobalInit& operator=(CurlGlobalInit&&) = delete;
};

} // namespace perplexity

#endif //PERPLEXITY_CPP_HTTPCLIENT_H