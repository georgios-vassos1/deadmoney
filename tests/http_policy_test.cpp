#include "HttpPolicy.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <thread>

// Minimal table fixture reused across tests
static Table make_table() {
    Table t(2, 5, 10);
    t.seat_player(0, "A", 1000);
    t.seat_player(1, "B", 1000);
    return t;
}

TEST(HttpPolicyTest, ActBlocksUntilPushAction) {
    auto table = make_table();
    HttpPolicy policy;

    auto fut = std::async(std::launch::async, [&]() {
        return policy.act(0, table);
    });

    ASSERT_EQ(fut.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout);

    policy.push_action({Action::Call, 0});

    ASSERT_EQ(fut.wait_for(std::chrono::milliseconds(200)), std::future_status::ready);
    const auto result = fut.get();
    EXPECT_EQ(result.action, Action::Call);
    EXPECT_EQ(result.amount, 0);
}

TEST(HttpPolicyTest, ActCapturesTableStateSnapshot) {
    Table table(2, 5, 10);
    table.seat_player(0, "A", 400);
    table.seat_player(1, "B", 800);

    HttpPolicy policy;

    auto fut = std::async(std::launch::async, [&]() {
        return policy.act(0, table);
    });

    // Wait until act() has had a chance to capture the state
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    const auto* snapshot = policy.last_table_state();
    ASSERT_NE(snapshot, nullptr);
    EXPECT_EQ(snapshot->num_seats(), 2);
    EXPECT_EQ(snapshot->player(0).stack, 400);
    EXPECT_EQ(snapshot->player(1).stack, 800);

    policy.push_action({Action::Fold, 0});
    fut.get();
}

TEST(HttpPolicyTest, ActRearmsAfterEachPush) {
    const auto table = make_table();
    HttpPolicy policy;

    for (int street = 0; street < 4; ++street) {
        const BetAction expected{Action::Raise, 10 * (street + 1)};

        auto fut = std::async(std::launch::async, [&]() {
            return policy.act(0, table);
        });

        ASSERT_EQ(fut.wait_for(std::chrono::milliseconds(50)), std::future_status::timeout)
            << "should block on street " << street;

        policy.push_action(expected);

        ASSERT_EQ(fut.wait_for(std::chrono::milliseconds(200)), std::future_status::ready)
            << "should unblock on street " << street;
        const auto result = fut.get();
        EXPECT_EQ(result.action, expected.action) << "street " << street;
        EXPECT_EQ(result.amount, expected.amount) << "street " << street;
    }
}
