#include "Player.hpp"

#include <gtest/gtest.h>

#include <string>

TEST(PlayerStatusStringTest, ToStringReturnsCanonicalNames) {
    EXPECT_EQ(to_string(PlayerStatus::Active), "active");
    EXPECT_EQ(to_string(PlayerStatus::Folded), "folded");
    EXPECT_EQ(to_string(PlayerStatus::AllIn),  "allin");
}

TEST(PlayerTest, PostDeductsFromStackAndSetsStreetBet) {
  Player p{"Alice", 1000};
  p.post(100);
  EXPECT_EQ(p.stack,      900);
  EXPECT_EQ(p.street_bet, 100);
  EXPECT_EQ(p.status,     PlayerStatus::Active);
}

TEST(PlayerTest, PostMoreThanStackCapsAtStackAndGoesAllIn) {
  Player p{"Bob", 50};
  const int actual = p.post(200);
  EXPECT_EQ(actual,       50);
  EXPECT_EQ(p.stack,      0);
  EXPECT_EQ(p.street_bet, 50);
  EXPECT_EQ(p.status,     PlayerStatus::AllIn);
}

TEST(PlayerTest, PostExactStackGoesAllIn) {
  Player p{"Carol", 100};
  p.post(100);
  EXPECT_EQ(p.stack,  0);
  EXPECT_EQ(p.status, PlayerStatus::AllIn);
}

TEST(PlayerTest, FoldSetsFoldedStatus) {
  Player p{"Dave", 1000};
  p.fold();
  EXPECT_TRUE(p.is_folded());
  EXPECT_FALSE(p.is_active());
  EXPECT_FALSE(p.is_all_in());
}

TEST(PlayerTest, ResetStreetClearsStreetBet) {
  Player p{"Eve", 1000};
  p.post(200);
  ASSERT_EQ(p.street_bet, 200);
  p.reset_street();
  EXPECT_EQ(p.street_bet, 0);
  EXPECT_EQ(p.stack, 800); // stack unchanged by reset_street
}
