#ifndef PERPLEXITY_AI_CPP_CLIENT_EXCEPTIONS_H
#define PERPLEXITY_AI_CPP_CLIENT_EXCEPTIONS_H
#pragma once

#include <stdexcept>
#include <string>
#include <optional>

namespace perplexity {

/**
 * @brief The base class for all library exceptions Perplexity
 *
 * Follows the exception hierarchy pattern for type-safe error handling.
 * All exceptions are inherited from std::runtime_error for compatibility.
 */
class PerplexityException : public std::runtime_error {
public:
    explicit PerplexityException(const std::string& message)
        : std::runtime_error(message) {}

    virtual ~PerplexityException() = default;
};

/**
 * @brief Exception for client API configuration errors
 */
class ConfigurationError : public PerplexityException {
public:
    explicit ConfigurationError(const std::string& message)
        : PerplexityException("Configuration error: " + message) {}
};

/**
 * @brief An exception for HTTP/network errors
 */
class NetworkError : public PerplexityException {
private:
    std::optional<int> http_status_code_;

public:
    explicit NetworkError(const std::string& message,
                         std::optional<int> status_code = std::nullopt)
        : PerplexityException("Network error: " + message)
        , http_status_code_(status_code) {}

    std::optional<int> getHttpStatusCode() const noexcept {
        return http_status_code_;
    }
};

/**
 * @brief Exception for authentication errors (401, 403)
 */
class AuthenticationError : public PerplexityException {
public:
    explicit AuthenticationError(const std::string& message)
        : PerplexityException("Authentication error: " + message) {}
};

/**
 * @brief Exception for rate limiting errors (429)
 */
class RateLimitError : public PerplexityException {
private:
    std::optional<int> retry_after_seconds_;

public:
    explicit RateLimitError(const std::string& message,
                           std::optional<int> retry_after = std::nullopt)
        : PerplexityException("Rate limit exceeded: " + message)
        , retry_after_seconds_(retry_after) {}

    std::optional<int> getRetryAfter() const noexcept {
        return retry_after_seconds_;
    }
};

/**
 * @brief Exception for request validation errors (400)
 */
class ValidationError : public PerplexityException {
public:
    explicit ValidationError(const std::string& message)
        : PerplexityException("Validation error: " + message) {}
};

/**
 * @brief Exception for JSON parsing errors
 */
class JsonParseError : public PerplexityException {
public:
    explicit JsonParseError(const std::string& message)
        : PerplexityException("JSON parse error: " + message) {}
};

/**
 * @brief Exception for server errors (5xx)
 */
class ServerError : public PerplexityException {
private:
    int status_code_;

public:
    explicit ServerError(const std::string& message, int status_code = 500)
        : PerplexityException("Server error: " + message)
        , status_code_(status_code) {}

    int getStatusCode() const noexcept {
        return status_code_;
    }
};

/**
 * @brief Exception for timeouts
 */
class TimeoutError : public PerplexityException {
public:
    explicit TimeoutError(const std::string& message)
        : PerplexityException("Timeout error: " + message) {}
};

} // namespace perplexity
#endif //PERPLEXITY_AI_CPP_CLIENT_EXCEPTIONS_H