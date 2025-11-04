#ifndef PERPLEXITY_CPP_CONFIG_H
#define PERPLEXITY_CPP_CONFIG_H
#pragma once

#include <string>
#include <optional>
#include <chrono>
#include "exceptions.h"

namespace perplexity {

/**
 * @brief Configuration of the Perplexity API client
 *
 * Uses the Builder pattern for convenient configuration.
 * Applies RAII for secure resource management.
 */
class Config {
private:
    std::string api_key_;
    std::string base_url_ = "https://api.perplexity.ai";
    std::chrono::seconds timeout_{30};
    int max_retries_ = 3;
    bool verify_ssl_ = true;
    std::optional<std::string> proxy_;
    std::optional<std::string> user_agent_;

    // Rate limiting configuration
    bool enable_rate_limiting_ = true;
    int max_requests_per_minute_ = 60;

public:
    Config() = default;
    explicit Config(std::string api_key) : api_key_(std::move(api_key)) {
        if (api_key_.empty()) {
            throw ConfigurationError("API key cannot be empty");
        }
    }

    // Builder methods (Fluent Interface)
    Config& api_key(std::string key) {
        if (key.empty()) {
            throw ConfigurationError("API key cannot be empty");
        }
        api_key_ = std::move(key);
        return *this;
    }

    Config& base_url(std::string url) {
        if (url.empty()) {
            throw ConfigurationError("Base URL cannot be empty");
        }
        base_url_ = std::move(url);
        return *this;
    }

    Config& timeout(std::chrono::seconds t) {
        if (t.count() <= 0) {
            throw ConfigurationError("Timeout must be positive");
        }
        timeout_ = t;
        return *this;
    }

    Config& max_retries(int retries) {
        if (retries < 0) {
            throw ConfigurationError("Max retries cannot be negative");
        }
        max_retries_ = retries;
        return *this;
    }

    Config& verify_ssl(bool verify) {
        verify_ssl_ = verify;
        return *this;
    }

    Config& proxy(std::string proxy_url) {
        proxy_ = std::move(proxy_url);
        return *this;
    }

    Config& user_agent(std::string agent) {
        user_agent_ = std::move(agent);
        return *this;
    }

    Config& enable_rate_limiting(bool enable) {
        enable_rate_limiting_ = enable;
        return *this;
    }

    Config& max_requests_per_minute(int max_requests) {
        if (max_requests <= 0) {
            throw ConfigurationError("Max requests per minute must be positive");
        }
        max_requests_per_minute_ = max_requests;
        return *this;
    }

    // Getters
    const std::string& get_api_key() const { return api_key_; }
    const std::string& get_base_url() const { return base_url_; }
    std::chrono::seconds get_timeout() const { return timeout_; }
    int get_max_retries() const { return max_retries_; }
    bool should_verify_ssl() const { return verify_ssl_; }
    const std::optional<std::string>& get_proxy() const { return proxy_; }
    const std::optional<std::string>& get_user_agent() const { return user_agent_; }
    bool is_rate_limiting_enabled() const { return enable_rate_limiting_; }
    int get_max_requests_per_minute() const { return max_requests_per_minute_; }

    // Configuration validation
    void validate() const {
        if (api_key_.empty()) {
            throw ConfigurationError("API key must be set");
        }
        if (base_url_.empty()) {
            throw ConfigurationError("Base URL must be set");
        }
        if (timeout_.count() <= 0) {
            throw ConfigurationError("Timeout must be positive");
        }
    }

    // Creating a configuration from environment variables
    static Config from_environment() {
        const char* api_key_env = std::getenv("PERPLEXITY_API_KEY");
        if (!api_key_env) {
            throw ConfigurationError(
                "PERPLEXITY_API_KEY environment variable not set"
            );
        }

        Config config(api_key_env);

        if (const char* base_url_env = std::getenv("PERPLEXITY_BASE_URL")) {
            config.base_url(base_url_env);
        }

        if (const char* timeout_env = std::getenv("PERPLEXITY_TIMEOUT")) {
            try {
                int timeout_seconds = std::stoi(timeout_env);
                config.timeout(std::chrono::seconds(timeout_seconds));
            } catch (const std::exception&) {
                throw ConfigurationError("Invalid PERPLEXITY_TIMEOUT value");
            }
        }

        if (const char* proxy_env = std::getenv("PERPLEXITY_PROXY")) {
            config.proxy(proxy_env);
        }

        return config;
    }
};

} // namespace perplexity
#endif //PERPLEXITY_CPP_CONFIG_H