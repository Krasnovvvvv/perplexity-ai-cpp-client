#ifndef PERPLEXITY_CPP_RATELIMITER_H
#define PERPLEXITY_CPP_RATELIMITER_H
#pragma once

#include <chrono>
#include <mutex>
#include <deque>
#include <thread>
#include "exceptions.h"

namespace perplexity {

/**
 * @brief Rate limiter for controlling the frequency of API requests
 *
 * Uses the Token Bucket algorithm to limit the rate.
 * Thread-safe implementation using mutex.
 */
class RateLimiter {
private:
    mutable std::mutex mutex_;
    std::deque<std::chrono::steady_clock::time_point> request_times_;
    int max_requests_per_minute_;
    bool enabled_;

    void cleanup_old_requests() {
        auto now = std::chrono::steady_clock::now();
        auto cutoff = now - std::chrono::minutes(1);

        while (!request_times_.empty() && request_times_.front() < cutoff) {
            request_times_.pop_front();
        }
    }

public:
    RateLimiter(int max_requests_per_minute = 60, bool enabled = true)
        : max_requests_per_minute_(max_requests_per_minute)
        , enabled_(enabled) {
        if (max_requests_per_minute <= 0) {
            throw ConfigurationError("Max requests per minute must be positive");
        }
    }

    /**
     * @brief Waiting before the next request (if necessary)
     *
     * Blocks execution if the limit is reached.
     */
    void wait_if_needed() {
        if (!enabled_) {
            return;
        }

        std::unique_lock<std::mutex> lock(mutex_);

        cleanup_old_requests();

        while (request_times_.size() >= static_cast<size_t>(max_requests_per_minute_)) {
            auto oldest = request_times_.front();
            auto wait_until = oldest + std::chrono::minutes(1);
            auto now = std::chrono::steady_clock::now();

            if (wait_until > now) {
                auto wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    wait_until - now
                );

                lock.unlock();
                std::this_thread::sleep_for(wait_duration);
                lock.lock();

                cleanup_old_requests();
            } else {
                break;
            }
        }

        // Registering a new request
        request_times_.push_back(std::chrono::steady_clock::now());
    }

    /**
     * @brief Checking if it is possible to make a request now
     */
    bool can_make_request() const {
        if (!enabled_) {
            return true;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        const_cast<RateLimiter*>(this)->cleanup_old_requests();

        return request_times_.size() <static_cast<size_t>(max_requests_per_minute_);
    }

    /**
     * @brief Getting the current number of requests in the last minute
     */
    size_t get_current_request_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        const_cast<RateLimiter*>(this)->cleanup_old_requests();
        return request_times_.size();
    }

    /**
     * @brief Enabling/disabling rate limiting
     */
    void set_enabled(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_ = enabled;
    }

    /**
     * @brief Setting a new limit
     */
    void set_limit(int max_requests_per_minute) {
        if (max_requests_per_minute <= 0) {
            throw ConfigurationError("Max requests per minute must be positive");
        }
        std::lock_guard<std::mutex> lock(mutex_);
        max_requests_per_minute_ = max_requests_per_minute;
    }

    /**
     * @brief Resetting the request counter
     */
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        request_times_.clear();
    }
};

} // namespace perplexity
#endif //PERPLEXITY_CPP_RATELIMITER_H