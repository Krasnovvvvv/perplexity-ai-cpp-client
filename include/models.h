#ifndef PERPLEXITY_AI_CPP_CLIENT_MODELS_H
#define PERPLEXITY_AI_CPP_CLIENT_MODELS_H
#pragma once

#include <vector>
#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "exceptions.h"

namespace perplexity {

using json = nlohmann::json;

/**
 * @brief The role of the message in the chat
 */
enum class MessageRole {
    System,
    User,
    Assistant
};

/**
 * @brief Converting a role to a string
 */
inline std::string to_string(MessageRole role) {
    switch (role) {
        case MessageRole::System: return "system";
        case MessageRole::User: return "user";
        case MessageRole::Assistant: return "assistant";
        default: throw ValidationError("Invalid message role");
    }
}

/**
 * @brief Converting a string to a role
 */
inline MessageRole role_from_string(const std::string& str) {
    if (str == "system") return MessageRole::System;
    if (str == "user") return MessageRole::User;
    if (str == "assistant") return MessageRole::Assistant;
    throw ValidationError("Unknown message role: " + str);
}

} // namespace perplexity

#endif //PERPLEXITY_AI_CPP_CLIENT_MODELS_H