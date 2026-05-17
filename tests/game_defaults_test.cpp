#include "GameDefaults.hpp"

#include <gtest/gtest.h>

TEST(GameDefaultsTest, DefaultsAreConsistentWithGameRules) {
    EXPECT_EQ(GameDefaults::num_seats, 2);
    EXPECT_EQ(GameDefaults::stack, 1000);
    EXPECT_EQ(GameDefaults::small_blind, 5);
    EXPECT_EQ(GameDefaults::big_blind, 10);
    EXPECT_EQ(GameDefaults::max_hands, 100);
    EXPECT_EQ(GameDefaults::server_port, 8080);
    EXPECT_EQ(GameDefaults::big_blind, GameDefaults::small_blind * 2);
}
