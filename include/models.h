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

/**
 * @brief The message in the chat
 */
    struct Message {
    MessageRole role;
    std::string content;

    Message() = default;
    Message(MessageRole r, std::string c)
        : role(r), content(std::move(c)) {}

    static Message system(std::string content) {
        return Message(MessageRole::System, std::move(content));
    }

    static Message user(std::string content) {
        return Message(MessageRole::User, std::move(content));
    }

    static Message assistant(std::string content) {
        return Message(MessageRole::Assistant, std::move(content));
    }

    json to_json() const {
        return {
                {"role", to_string(role)},
                {"content", content}
        };
    }

    static Message from_json(const json& j) {
        return Message(
            role_from_string(j.at("role").get<std::string>()),
            j.at("content").get<std::string>()
        );
    }
};

/**
 * @brief Quote from the source
 */
    struct Citation {
        std::string url;
        std::optional<std::string> title;
        std::optional<std::string> snippet;
        std::optional<std::string> date;

        static Citation from_json(const json& j) {
            Citation citation;
            citation.url = j.at("url").get<std::string>();

            if (j.contains("title") && !j["title"].is_null()) {
                citation.title = j["title"].get<std::string>();
            }
            if (j.contains("snippet") && !j["snippet"].is_null()) {
                citation.snippet = j["snippet"].get<std::string>();
            }
            if (j.contains("date") && !j["date"].is_null()) {
                citation.date = j["date"].get<std::string>();
            }

            return citation;
        }
    };

/**
 * @brief Search result
 */
    struct SearchResult {
        std::string title;
        std::string url;
        std::optional<std::string> snippet;
        std::optional<std::string> date;
        std::optional<std::string> last_updated;

        static SearchResult from_json(const json& j) {
            SearchResult result;
            result.title = j.at("title").get<std::string>();
            result.url = j.at("url").get<std::string>();

            if (j.contains("snippet") && !j["snippet"].is_null()) {
                result.snippet = j["snippet"].get<std::string>();
            }
            if (j.contains("date") && !j["date"].is_null()) {
                result.date = j["date"].get<std::string>();
            }
            if (j.contains("last_updated") && !j["last_updated"].is_null()) {
                result.last_updated = j["last_updated"].get<std::string>();
            }

            return result;
        }
    };

/**
* @brief Information about the use of tokens
*/
    struct Usage {
        int prompt_tokens = 0;
        int completion_tokens = 0;
        int total_tokens = 0;
        std::optional<std::string> search_context_size;

        struct Cost {
            double input_tokens_cost = 0.0;
            double output_tokens_cost = 0.0;
            double request_cost = 0.0;
            double total_cost = 0.0;

            static Cost from_json(const json& j) {
                Cost cost;
                if (j.contains("input_tokens_cost")) {
                    cost.input_tokens_cost = j["input_tokens_cost"].get<double>();
                }
                if (j.contains("output_tokens_cost")) {
                    cost.output_tokens_cost = j["output_tokens_cost"].get<double>();
                }
                if (j.contains("request_cost")) {
                    cost.request_cost = j["request_cost"].get<double>();
                }
                if (j.contains("total_cost")) {
                    cost.total_cost = j["total_cost"].get<double>();
                }
                return cost;
            }
        };

        std::optional<Cost> cost;

        static Usage from_json(const json& j) {
            Usage usage;
            usage.prompt_tokens = j.at("prompt_tokens").get<int>();
            usage.completion_tokens = j.at("completion_tokens").get<int>();
            usage.total_tokens = j.at("total_tokens").get<int>();

            if (j.contains("search_context_size") && !j["search_context_size"].is_null()) {
                usage.search_context_size = j["search_context_size"].get<std::string>();
            }
            if (j.contains("cost") && !j["cost"].is_null()) {
                usage.cost = Cost::from_json(j["cost"]);
            }

            return usage;
        }
    };

} // namespace perplexity

#endif //PERPLEXITY_AI_CPP_CLIENT_MODELS_H