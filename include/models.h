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

/**
 * @brief Request to the Chat Completions API
 *
 * Uses the Builder pattern to create queries conveniently
 */
class ChatRequest {
private:
    std::string model_;
    std::vector<Message> messages_;
    std::optional<double> temperature_;
    std::optional<int> max_tokens_;
    std::optional<double> top_p_;
    std::optional<int> top_k_;
    std::optional<double> presence_penalty_;
    std::optional<double> frequency_penalty_;
    bool stream_ = false;
    bool return_citations_ = true;
    bool return_images_ = false;
    std::optional<std::vector<std::string>> search_domain_filter_;
    std::optional<std::string> search_recency_filter_;

public:
    ChatRequest() = default;
    explicit ChatRequest(std::string model) : model_(std::move(model)) {}

    // Builder methods (fluent interface pattern)
    ChatRequest& model(std::string m) {
        model_ = std::move(m);
        return *this;
    }

    ChatRequest& add_message(Message msg) {
        messages_.push_back(std::move(msg));
        return *this;
    }

    ChatRequest& add_message(MessageRole role, std::string content) {
        messages_.emplace_back(role, std::move(content));
        return *this;
    }

    ChatRequest& messages(std::vector<Message> msgs) {
        messages_ = std::move(msgs);
        return *this;
    }

    ChatRequest& temperature(double t) {
        if (t < 0.0 || t > 2.0) {
            throw ValidationError("Temperature must be between 0.0 and 2.0");
        }
        temperature_ = t;
        return *this;
    }

    ChatRequest& max_tokens(int tokens) {
        if (tokens < 1) {
            throw ValidationError("max_tokens must be positive");
        }
        max_tokens_ = tokens;
        return *this;
    }

    ChatRequest& top_p(double p) {
        if (p < 0.0 || p > 1.0) {
            throw ValidationError("top_p must be between 0.0 and 1.0");
        }
        top_p_ = p;
        return *this;
    }

    ChatRequest& top_k(int k) {
        if (k < 0) {
            throw ValidationError("top_k must be non-negative");
        }
        top_k_ = k;
        return *this;
    }

    ChatRequest& presence_penalty(double p) {
        if (p < -2.0 || p > 2.0) {
            throw ValidationError("presence_penalty must be between -2.0 and 2.0");
        }
        presence_penalty_ = p;
        return *this;
    }

    ChatRequest& frequency_penalty(double p) {
        if (p < -2.0 || p > 2.0) {
            throw ValidationError("frequency_penalty must be between -2.0 and 2.0");
        }
        frequency_penalty_ = p;
        return *this;
    }

    ChatRequest& stream(bool s) {
        stream_ = s;
        return *this;
    }

    ChatRequest& return_citations(bool c) {
        return_citations_ = c;
        return *this;
    }

    ChatRequest& return_images(bool i) {
        return_images_ = i;
        return *this;
    }

    ChatRequest& search_domain_filter(std::vector<std::string> domains) {
        search_domain_filter_ = std::move(domains);
        return *this;
    }

    ChatRequest& search_recency_filter(std::string filter) {
        search_recency_filter_ = std::move(filter);
        return *this;
    }

    void validate() const {
        if (model_.empty()) {
            throw ValidationError("Model must be specified");
        }
        if (messages_.empty()) {
            throw ValidationError("At least one message is required");
        }
    }

    // Serialization in JSON
    json to_json() const {
        validate();

        json j = {
            {"model", model_},
            {"messages", json::array()}
        };

        for (const auto& msg : messages_) {
            j["messages"].push_back(msg.to_json());
        }

        if (temperature_) j["temperature"] = *temperature_;
        if (max_tokens_) j["max_tokens"] = *max_tokens_;
        if (top_p_) j["top_p"] = *top_p_;
        if (top_k_) j["top_k"] = *top_k_;
        if (presence_penalty_) j["presence_penalty"] = *presence_penalty_;
        if (frequency_penalty_) j["frequency_penalty"] = *frequency_penalty_;

        j["stream"] = stream_;
        j["return_citations"] = return_citations_;
        j["return_images"] = return_images_;

        if (search_domain_filter_) {
            j["search_domain_filter"] = *search_domain_filter_;
        }
        if (search_recency_filter_) {
            j["search_recency_filter"] = *search_recency_filter_;
        }

        return j;
    }
};

/**
* @brief Response from the Chat Completions API
*/
    struct ChatResponse {
        std::string id;
        std::string model;
        int64_t created;
        std::string content;
        std::string finish_reason;
        std::vector<std::string> citations;
        std::vector<SearchResult> search_results;
        Usage usage;

        static ChatResponse from_json(const json& j) {
            ChatResponse response;

            response.id = j.at("id").get<std::string>();
            response.model = j.at("model").get<std::string>();
            response.created = j.at("created").get<int64_t>();

            // Extracting content from choices
            if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
                const auto& choice = j["choices"][0];
                if (choice.contains("message")) {
                    response.content = choice["message"]["content"].get<std::string>();
                }
                if (choice.contains("finish_reason")) {
                    response.finish_reason = choice["finish_reason"].get<std::string>();
                }
            }

            // Quotes
            if (j.contains("citations") && j["citations"].is_array()) {
                for (const auto& url : j["citations"]) {
                    response.citations.push_back(url.get<std::string>());
                }
            }

            // Search results
            if (j.contains("search_results") && j["search_results"].is_array()) {
                for (const auto& result : j["search_results"]) {
                    response.search_results.push_back(SearchResult::from_json(result));
                }
            }

            // Usage info
            if (j.contains("usage")) {
                response.usage = Usage::from_json(j["usage"]);
            }

            return response;
        }
    };

} // namespace perplexity

#endif //PERPLEXITY_AI_CPP_CLIENT_MODELS_H