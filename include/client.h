#ifndef PERPLEXITY_CPP_CLIENT_H
#define PERPLEXITY_CPP_CLIENT_H
#pragma once

#include <future>
#include <functional>
#include <sstream>
#include <nlohmann/json.hpp>
#include "config.h"
#include "models.h"
#include "HttpClient.h"
#include "RateLimiter.h"
#include "exceptions.h"

namespace perplexity {

using json = nlohmann::json;
using StreamCallback = std::function<void(const StreamChunk&)>;

/**
* @brief The main client of the Perplexity API
*
* Patterns:
* - RAII for resource management
* - Strategy pattern for various types of requests
* - Template Method for processing responses with retry logic
*/
class Client {
private:
    Config config_;
    RateLimiter rate_limiter_;
    CurlGlobalInit curl_init_;

    /**
     * @brief HTTP response processing and error conversion
     */
    void handle_http_response(long status_code, const std::string& response_body) {
        if (status_code >= 200 && status_code < 300) {
            return;
        }

        std::string error_message = "HTTP " + std::to_string(status_code);
        try {
            auto j = json::parse(response_body);
            if (j.contains("error")) {
                if (j["error"].is_string()) {
                    error_message = j["error"].get<std::string>();
                } else if (j["error"].is_object() && j["error"].contains("message")) {
                    error_message = j["error"]["message"].get<std::string>();
                }
            }
        } catch (...) {
            if (!response_body.empty() && response_body.size() < 200) {
                error_message = response_body;
            }
        }

        switch (status_code) {
            case 400:
                throw ValidationError(error_message);
            case 401:
            case 403:
                throw AuthenticationError(error_message);
            case 429: {
                std::optional<int> retry_after;
                try {
                    auto j = json::parse(response_body);
                    if (j.contains("retry_after")) {
                        retry_after = j["retry_after"].get<int>();
                    }
                } catch (...) {}
                throw RateLimitError(error_message, retry_after);
            }
            case 500:
            case 502:
            case 503:
            case 504:
                throw ServerError(error_message, status_code);
            default:
                throw NetworkError(error_message, status_code);
        }
    }

    /**
     * @brief Template Method for executing a query with retry logic
     */
    std::string execute_request_with_retry(
        const std::function<std::string(HttpClient&)>& request_func
    ) {
        int attempt = 0;
        std::exception_ptr last_exception;

        while (attempt <= config_.get_max_retries()) {
            try {
                rate_limiter_.wait_if_needed();

                HttpClient http_client(config_);

                std::string response = request_func(http_client);
                long status_code = http_client.get_response_code();

                handle_http_response(status_code, response);

                return response;

            } catch (const RateLimitError&) {
                throw;
            } catch (const AuthenticationError&) {
                throw;
            } catch (const ValidationError&) {
                throw;
            } catch (const ServerError& e) {
                last_exception = std::current_exception();

                if (attempt < config_.get_max_retries()) {
                    // Exponential backoff
                    auto wait_ms = std::chrono::milliseconds(100 * (1 << attempt));
                    std::this_thread::sleep_for(wait_ms);
                }
            } catch (const NetworkError&) {
                last_exception = std::current_exception();

                if (attempt < config_.get_max_retries()) {
                    auto wait_ms = std::chrono::milliseconds(100 * (1 << attempt));
                    std::this_thread::sleep_for(wait_ms);
                }
            }

            attempt++;
        }

        if (last_exception) {
            std::rethrow_exception(last_exception);
        }

        throw NetworkError("Request failed after " + std::to_string(config_.get_max_retries()) + " retries");
    }

    /**
     * @brief Preparing headers for the request
     */
    void prepare_headers(HttpClient& http_client) const {
        http_client.add_header("Content-Type: application/json");
        http_client.add_header("Authorization: Bearer " + config_.get_api_key());
        http_client.add_header("Accept: application/json");
    }

public:
    /**
     * @brief Constructor with configuration
     */
    explicit Client(Config config)
        : config_(std::move(config))
        , rate_limiter_(
            config_.get_max_requests_per_minute(),
            config_.is_rate_limiting_enabled()
          ) {
        config_.validate();
    }

    /**
     * @brief Constructor with API key (uses default configuration)
     */
    explicit Client(const std::string& api_key)
        : Client(Config(api_key)) {}

    /**
     * @brief Constructor of environment variables
     */
    static Client from_environment() {
        return Client(Config::from_environment());
    }

    /**
     * @brief Synchronous request to the Chat Completions API
     */
    ChatResponse chat(const ChatRequest& request) {
        auto response_json = execute_request_with_retry([&](HttpClient& http_client) {
            prepare_headers(http_client);

            std::string url = config_.get_base_url() + "/chat/completions";
            std::string body = request.to_json().dump();

            return http_client.post(url, body);
        });

        try {
            auto j = json::parse(response_json);
            return ChatResponse::from_json(j);
        } catch (const json::exception& e) {
            throw JsonParseError("Failed to parse response: " + std::string(e.what()));
        }
    }

    /**
     * @brief Asynchronous request to the Chat Completions API
     *
     * Returns std::future for non-blocking execution
     */
    std::future<ChatResponse> chat_async(const ChatRequest& request) {
        return std::async(std::launch::async, [this, request]() {
            return this->chat(request);
        });
    }

    /**
     * @brief Streaming request to Chat Completions API
     *
     * A callback is called for each data chunk
     */
    void chat_stream(ChatRequest request, StreamCallback callback) {
        request.stream(true);

        rate_limiter_.wait_if_needed();

        HttpClient http_client(config_);
        prepare_headers(http_client);

        std::string url = config_.get_base_url() + "/chat/completions";
        std::string body = request.to_json().dump();

        // Streaming requires more complex processing
        // In the simplified version, we use a regular POST and parse SSE
        std::string response = http_client.post(url, body);

        // Parsing Server-Sent Events
        std::istringstream stream(response);
        std::string line;
        std::string data_buffer;

        while (std::getline(stream, line)) {
            if (line.empty() && !data_buffer.empty()) {

                if (data_buffer.substr(0, 6) == "data: ") {
                    std::string json_str = data_buffer.substr(6);

                    if (json_str == "[DONE]") {
                        break;
                    }

                    try {
                        auto j = json::parse(json_str);
                        StreamChunk chunk = StreamChunk::from_json(j);
                        callback(chunk);
                    } catch (const json::exception& e) {
                        throw JsonParseError("Failed to parse stream chunk: " + std::string(e.what()));
                    }
                }
                data_buffer.clear();
            } else {
                data_buffer += line + "\n";
            }
        }
    }

    /**
     * @brief Getting the client configuration
     */
    const Config& get_config() const {
        return config_;
    }

    /**
     * @brief Getting a rate limiter (for management and monitoring)
     */
    RateLimiter& get_rate_limiter() {
        return rate_limiter_;
    }

    const RateLimiter& get_rate_limiter() const {
        return rate_limiter_;
    }
};

} // namespace perplexity
#endif //PERPLEXITY_CPP_CLIENT_H