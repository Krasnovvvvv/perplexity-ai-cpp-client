 /**
 * @file basic_chat.cpp
 * @brief A basic example of using the Perplexity C++ API
 *
 * Demonstrates the main features of the library:
 * - Initializing the client
 * - Creating a request using the Builder pattern
 * - Response and quotation processing
 * - Error handling
 */

#include <perplexity.h>
#include <iostream>
#include <iomanip>

void print_separator() {
    std::cout << std::string(80, '=') << '\n';
}

int main() {
    try {
        std::cout << "Initialization of the client's Perplexity API...\n";
        auto client = perplexity::Client::from_environment();

        std::cout << "The client is initialized (version: "
                  << perplexity::get_version() << ")\n\n";

        print_separator();

        // Creating a simple query using the Builder pattern
        std::cout << "Creating a request...\n";

        auto request = perplexity::ChatRequest("sonar-pro")
            .add_message(perplexity::Message::user(
                "What are the latest developments in quantum computing?"
            ))
            .temperature(0.7)
            .max_tokens(1000)
            .return_citations(true);

        std::cout << "   Request created\n";
        std::cout << "   Мodel: sonar-pro\n";
        std::cout << "   Temperature: 0.7\n";
        std::cout << "   Max tokens: 1000\n\n";

        print_separator();

        // Request Execution
        std::cout << "Sending an API request...\n";
        auto response = client.chat(request);
        std::cout << "Response received\n\n";

        print_separator();

        // Output of the result
        std::cout << "ANSWER:\n\n";
        std::cout << response.content << "\n\n";

        print_separator();

        // Output of quotes
        if (!response.citations.empty()) {
            std::cout << "SOURCES (" << response.citations.size() << "):\n\n";
            for (size_t i = 0; i < response.citations.size(); ++i) {
                std::cout << "[" << (i + 1) << "] " << response.citations[i] << '\n';
            }
            std::cout << '\n';
            print_separator();
        }

        // Output of usage statistics
        std::cout << "STATISTICS:\n";
        std::cout << "   Request ID : " << response.id << '\n';
        std::cout << "   Model: " << response.model << '\n';
        std::cout << "   Tokens (prompt): " << response.usage.prompt_tokens << '\n';
        std::cout << "   Tokens (completion): " << response.usage.completion_tokens << '\n';
        std::cout << "   Total tokens: " << response.usage.total_tokens << '\n';

        if (response.usage.cost) {
            std::cout << "   Cost: $" << std::fixed << std::setprecision(6)
                      << response.usage.cost->total_cost << '\n';
        }

        std::cout << '\n';
        print_separator();

        // Example with multiple messages (dialog)
        std::cout << "\nDIALOG EXAMPLE:\n\n";

        auto dialog_request = perplexity::ChatRequest("sonar-pro")
            .add_message(perplexity::Message::system(
                "You are a helpful AI assistant."
            ))
            .add_message(perplexity::Message::user(
                "What is the capital of France?"
            ))
            .add_message(perplexity::Message::assistant(
                "The capital of France is Paris."
            ))
            .add_message(perplexity::Message::user(
                "What is its population?"
            ))
            .temperature(0.5);

        auto dialog_response = client.chat(dialog_request);
        std::cout << "User: What is its population?\n";
        std::cout << "Assistant: " << dialog_response.content << "\n\n";

        print_separator();

        // Checking the rate limiter
        auto& rate_limiter = client.get_rate_limiter();
        std::cout << "\nRATE LIMITER:\n";
        std::cout << "   Current number of requests: "
                  << rate_limiter.get_current_request_count() << '\n';
        std::cout << "   You can make a request: "
                  << (rate_limiter.can_make_request() ? "Yes" : "No") << '\n';

        std::cout << "\nAll operations have been completed successfully!\n";

        return 0;

    } catch (const perplexity::AuthenticationError& e) {
        std::cerr << "Authentication error: " << e.what() << '\n';
        std::cerr << "   Check your API key\n";
        return 1;

    } catch (const perplexity::RateLimitError& e) {
        std::cerr << "Request limit exceeded: " << e.what() << '\n';
        if (e.getRetryAfter()) {
            std::cerr << "   Try again after "
                      << *e.getRetryAfter() << " seconds\n";
        }
        return 1;

    } catch (const perplexity::ValidationError& e) {
        std::cerr << "Validation error: " << e.what() << '\n';
        return 1;

    } catch (const perplexity::NetworkError& e) {
        std::cerr << "Network error: " << e.what() << '\n';
        if (e.getHttpStatusCode()) {
            std::cerr << "   HTTP status: " << *e.getHttpStatusCode() << '\n';
        }
        return 1;

    } catch (const perplexity::PerplexityException& e) {
        std::cerr << "Perplexity API error: " << e.what() << '\n';
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << '\n';
        return 1;
    }
}