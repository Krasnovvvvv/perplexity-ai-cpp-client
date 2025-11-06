#include <gtest/gtest.h>
#include <perplexity.h>

using namespace perplexity;

// Test: Basic Configuration error
TEST(PerplexityExceptions, ConfigurationError) {
    EXPECT_THROW(Config(""), ConfigurationError);
    Config cfg("api-key");
    EXPECT_NO_THROW(cfg.timeout(std::chrono::seconds(10)));
}

// Test: Checking the Message builder
TEST(PerplexityModels, MessageBuilder) {
    Message msg = Message::user("Testuser");
    ASSERT_EQ(msg.role, MessageRole::User);
    ASSERT_EQ(msg.content, "Testuser");
    // Checking serialization
    auto j = msg.to_json();
    EXPECT_EQ(j["role"], "user");
    EXPECT_EQ(j["content"], "Testuser");
}

// Test: Request validation
TEST(PerplexityModels, ChatRequestValidation) {
    ChatRequest req;
    EXPECT_THROW(req.validate(), ValidationError);
    req.model("sonar-pro").add_message(Message::user("Hi!"));
    EXPECT_NO_THROW(req.validate());
    auto j = req.to_json();
    EXPECT_EQ(j["model"], "sonar-pro");
}

// Test: RateLimiter
TEST(PerplexityUtils, RateLimiterBasic) {
    RateLimiter limiter(5, true);
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(limiter.can_make_request());
        limiter.wait_if_needed();
    }
    EXPECT_FALSE(limiter.can_make_request());
    limiter.reset();
    EXPECT_TRUE(limiter.can_make_request());
}

// Test: Client Exclusions
TEST(PerplexityExceptions, AuthAndNetwork) {
    EXPECT_THROW(AuthenticationError("fail"), AuthenticationError);
    EXPECT_THROW(NetworkError("fail", 400), NetworkError);
    EXPECT_THROW(RateLimitError("fail", 5), RateLimitError);
}

TEST(PerplexityIntegration, RealRequest) {
    const char* key = std::getenv("PERPLEXITY_API_KEY");
    if (!key) GTEST_SKIP() << "No API key in env";
    Client client(key);
    auto req = ChatRequest("sonar-pro")
        .add_message(Message::user("Ping!"))
        .max_tokens(10);
    ChatResponse resp = client.chat(req);
    EXPECT_TRUE(!resp.content.empty());
}

