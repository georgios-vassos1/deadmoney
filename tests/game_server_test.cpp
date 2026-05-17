#include "GameServer.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <stdexcept>
#include <thread>

// Poll helper: spin until predicate is true or timeout elapses.
static bool wait_for(std::function<bool()> pred, int timeout_ms = 1000) {
    for (int i = 0; i < timeout_ms / 10; ++i) {
        if (pred()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
}

TEST(GameServerTest, BotOnlyGameCompletesOnBackgroundThread) {
    GameServer server(2, 1000, 5, 10, {}, 3);

    for (int i = 0; i < 100 && !server.is_done(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(server.is_done());
    EXPECT_EQ(server.hands_played(), 3);
}

TEST(GameServerTest, SessionBlocksAtHumanSeatUntilPushAction) {
    // Seat 0 is human; seat 1 is bot. Human always folds → one action per hand.
    GameServer server(2, 1000, 5, 10, {0}, 1);

    ASSERT_TRUE(wait_for([&]{ return server.current_state() != nullptr; }))
        << "session should have reached human turn";

    server.push_action(0, {Action::Fold, 0});

    ASSERT_TRUE(wait_for([&]{ return server.is_done(); }))
        << "hand should complete after fold";
    EXPECT_EQ(server.hands_played(), 1);
}

TEST(GameServerTest, MultiHandGameWithHumanSeatCompletesAllHands) {
    const int num_hands = 4;
    GameServer server(2, 1000, 5, 10, {0}, num_hands);

    for (int hand = 0; hand < num_hands; ++hand) {
        ASSERT_TRUE(wait_for([&]{ return server.current_state() != nullptr; }))
            << "should reach human turn on hand " << hand;
        server.push_action(0, {Action::Fold, 0});
    }

    ASSERT_TRUE(wait_for([&]{ return server.is_done(); }));
    EXPECT_EQ(server.hands_played(), num_hands);
}

TEST(GameServerTest, PushActionOnBotSeatThrows) {
    GameServer server(2, 1000, 5, 10, {0}, 1); // seat 1 is a bot
    EXPECT_THROW(server.push_action(1, {Action::Call, 0}), std::invalid_argument);
    // clean up: push a valid action so the destructor doesn't hang
    ASSERT_TRUE(wait_for([&]{ return server.current_state() != nullptr; }));
    server.push_action(0, {Action::Fold, 0});
}
