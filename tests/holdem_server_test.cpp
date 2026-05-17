#include "HoldemServer.hpp"

#include "httplib.h"
#include "json.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

using json = nlohmann::json;

// Shared fixture: one server for all tests in this file.
class HoldemServerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        s_server = std::make_unique<HoldemServer>();
        s_server->start(0); // port 0 → OS picks a free port
        s_port = s_server->port();
    }
    static void TearDownTestSuite() {
        s_server->stop();
        s_server.reset();
    }

    httplib::Client client() const {
        return httplib::Client("127.0.0.1", s_port);
    }

    static std::unique_ptr<HoldemServer> s_server;
    static int                           s_port;
};

std::unique_ptr<HoldemServer> HoldemServerTest::s_server;
int                           HoldemServerTest::s_port = 0;

static std::string create_game(httplib::Client& c, int human_seat = 0, int max_hands = 10) {
    const nlohmann::json body = {
        {"num_seats", 2}, {"stack", 1000},
        {"human_seats", nlohmann::json::array({human_seat})},
        {"max_hands", max_hands}
    };
    const auto res = c.Post("/game/new", body.dump(), "application/json");
    return nlohmann::json::parse(res->body)["game_id"].get<std::string>();
}

TEST_F(HoldemServerTest, NewGameReturnsGameId) {
    const json body = {
        {"num_seats",   2},
        {"stack",    1000},
        {"human_seats", json::array({0})},
        {"max_hands",  10}
    };

    auto c = client();
    const auto res = c.Post("/game/new", body.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->status, 200);

    const json resp = json::parse(res->body);
    EXPECT_TRUE(resp.contains("game_id"));
    EXPECT_FALSE(resp["game_id"].get<std::string>().empty());
}

TEST_F(HoldemServerTest, StateReturnsNumSeatsAndStacks) {
    auto c = client();
    const std::string id = create_game(c);

    // Wait for the session to reach the first human decision point.
    json state;
    for (int i = 0; i < 100; ++i) {
        const auto res = c.Get("/game/" + id + "/state");
        ASSERT_NE(res, nullptr);
        ASSERT_EQ(res->status, 200);
        state = json::parse(res->body);
        if (state.value("done", false) == false && state.contains("players"))
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(state["num_seats"].get<int>(), 2);
    ASSERT_TRUE(state.contains("players"));
    ASSERT_EQ(state["players"].size(), 2u);
    for (const auto& p : state["players"])
        EXPECT_GT(p["stack"].get<int>(), 0);
}

TEST_F(HoldemServerTest, PostActionAdvancesGame) {
    auto c = client();
    const std::string id = create_game(c, 0, 1); // 1 hand

    // Wait for human decision point.
    for (int i = 0; i < 100; ++i) {
        const auto s = json::parse(c.Get("/game/" + id + "/state")->body);
        if (s.contains("players")) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    const json action = {{"seat", 0}, {"action", "fold"}, {"amount", 0}};
    const auto res = c.Post("/game/" + id + "/action", action.dump(), "application/json");

    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->status, 200);
    const json resp = json::parse(res->body);
    EXPECT_TRUE(resp.contains("hands_played"));
}

TEST_F(HoldemServerTest, UnknownGameIdReturns404) {
    auto c = client();
    const auto res = c.Get("/game/doesnotexist/state");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(HoldemServerTest, InvalidActionStringReturns400) {
    auto c = client();
    const std::string id = create_game(c);
    const json action = {{"seat", 0}, {"action", "yolo"}, {"amount", 0}};
    const auto res = c.Post("/game/" + id + "/action", action.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(HoldemServerTest, DoneStateIncludesNumSeats) {
    auto c = client();
    const std::string id = create_game(c, 0, 1);

    for (int i = 0; i < 100; ++i) {
        const auto s = json::parse(c.Get("/game/" + id + "/state")->body);
        if (s.contains("players")) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    const json act = {{"seat", 0}, {"action", "fold"}, {"amount", 0}};
    c.Post("/game/" + id + "/action", act.dump(), "application/json");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const auto res = c.Get("/game/" + id + "/state");
    ASSERT_NE(res, nullptr);
    const auto done_state = json::parse(res->body);
    EXPECT_TRUE(done_state.value("done", false));
    EXPECT_EQ(done_state.value("num_seats", 0), 2);
}

TEST_F(HoldemServerTest, InvalidRaiseAmountReturns400) {
    auto c = client();
    const std::string id = create_game(c, 0, 1);

    for (int i = 0; i < 100; ++i) {
        const auto s = json::parse(c.Get("/game/" + id + "/state")->body);
        if (s.contains("players")) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    const json action = {{"seat", 0}, {"action", "raise"}, {"amount", 0}};
    const auto res = c.Post("/game/" + id + "/action", action.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}
