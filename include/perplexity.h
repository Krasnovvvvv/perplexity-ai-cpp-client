#ifndef PERPLEXITY_CPP_PERPLEXITY_H
#define PERPLEXITY_CPP_PERPLEXITY_H
#pragma once

/**
 * @file perplexity.h
 * @brief The main header file of the Perplexity C++ library
 *
 * Includes all the necessary components to work with the Perplexity API.
 * This is the only file that needs to be attached to use the library.
 *
 * @example Easy to use:
 * @code
 * #include <perplexity/perplexity.hpp>
 *
 * int main() {
 *     perplexity::Client client("your-api-key");
 *
 *     auto request = perplexity::ChatRequest("sonar-pro")
 *         .add_message(perplexity::Message::user("Hello, world!"));
 *
 *     auto response = client.chat(request);
 *     std::cout << response.content << std::endl;
 *
 *     return 0;
 * }
 * @endcode
 *
 * @version 1.0.0
 * @author Perplexity C++ Library
 * @license MIT
 */

// Library version
#define PERPLEXITY_CPP_VERSION_MAJOR 1
#define PERPLEXITY_CPP_VERSION_MINOR 0
#define PERPLEXITY_CPP_VERSION_PATCH 0

#define PERPLEXITY_CPP_VERSION \
(PERPLEXITY_CPP_VERSION_MAJOR * 10000 + \
PERPLEXITY_CPP_VERSION_MINOR * 100 + \
PERPLEXITY_CPP_VERSION_PATCH)

// Main Library Components
#include "exceptions.h"
#include "config.h"
#include "models.h"
#include "HttpClient.h"
#include "RateLimiter.h"
#include "client.h"

/**
 * @namespace perplexity
 * @brief The main namespace of the Perplexity C++ library
 *
 * It contains all classes and functions for working with the Perplexity API.
 */
namespace perplexity {

    /**
     * @brief Returns the library version as a string
     */
    inline std::string get_version() {
        return std::to_string(PERPLEXITY_CPP_VERSION_MAJOR) + "." +
               std::to_string(PERPLEXITY_CPP_VERSION_MINOR) + "." +
               std::to_string(PERPLEXITY_CPP_VERSION_PATCH);
    }

    using ChatClient = Client;

} // namespace perplexity
#endif //PERPLEXITY_CPP_PERPLEXITY_H