/**
* @file async_requests.cpp
* @brief An example of asynchronous requests to the Perplexity API
*
* Demonstrates:
* - Asynchronous requests with std::future
* - Parallel execution of multiple queries
* - Processing of results
*/

#include <perplexity.h>
#include <iostream>
#include <vector>
#include <chrono>

int main() {
    try {
        auto client = perplexity::Client::from_environment();

        std::cout << "Asynchronous requests to Perplexity API\n";
        std::cout << std::string(80, '=') << "\n\n";

        std::vector<std::string> questions = {
            "What is quantum computing?",
            "Explain machine learning in simple terms",
            "What are the latest developments in AI?"
        };

        std::cout << "Sending " << questions.size() << " requests in parallel...\n\n";

        auto start_time = std::chrono::steady_clock::now();

        std::vector<std::future<perplexity::ChatResponse>> futures;
        for (const auto& question : questions) {
            auto request = perplexity::ChatRequest("sonar-pro")
                .add_message(perplexity::Message::user(question))
                .max_tokens(500);

            futures.push_back(client.chat_async(request));
            std::cout << "The request has been sent: " << question << '\n';
        }

        std::cout << "\nWaiting for answers...\n\n";

        // Получение результатов
        for (size_t i = 0; i < futures.size(); ++i) {
            std::cout << "Question " << (i + 1) << ": " << questions[i] << '\n';

            auto response = futures[i].get();

            std::cout << "Answer: " << response.content.substr(0, 200);
            if (response.content.length() > 200) {
                std::cout << "...";
            }
            std::cout << "\n\n";
        }

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time
        );

        std::cout << "All requests were completed in " << duration.count() << " ms\n";

        return 0;

    } catch (const perplexity::PerplexityException& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}