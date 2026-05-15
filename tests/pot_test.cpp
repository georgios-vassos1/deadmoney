#include "Pot.hpp"

#include <gtest/gtest.h>

TEST(PotTest, TotalReflectsAllContributions) {
  Pot pot;
  pot.contribute(0, 100);
  pot.contribute(1, 100);
  pot.contribute(2, 100);
  EXPECT_EQ(pot.total(), 300);
}

TEST(PotTest, OnePlayerAllInForLessCreatesTwoSidePots) {
  // Seat 0 all-in for 50; seats 1 and 2 call 100.
  // Main pot:  50 * 3 = 150, all three eligible.
  // Side pot:  50 * 2 = 100, seats 1 and 2 only.
  Pot pot;
  pot.contribute(0, 50);
  pot.contribute(1, 100);
  pot.contribute(2, 100);
  pot.resolve();

  ASSERT_EQ(pot.side_pots().size(), 2u);

  const auto& main = pot.side_pots()[0];
  EXPECT_EQ(main.amount, 150);
  EXPECT_EQ(main.eligible_seats.size(), 3u);

  const auto& side = pot.side_pots()[1];
  EXPECT_EQ(side.amount, 100);
  ASSERT_EQ(side.eligible_seats.size(), 2u);
  // Seat 0 must NOT appear in the side pot.
  for (int s : side.eligible_seats)
    EXPECT_NE(s, 0) << "seat 0 should not be eligible for the side pot";
}

TEST(PotTest, TwoAllInsAtDifferentLevelsCreateThreeSidePots) {
  // Seat 0 all-in for 30; seat 1 all-in for 60; seat 2 calls 100.
  // Pot 1: 30 * 3 = 90,  seats 0,1,2 eligible.
  // Pot 2: 30 * 2 = 60,  seats 1,2 eligible.
  // Pot 3: 40 * 1 = 40,  seat 2 only.
  Pot pot;
  pot.contribute(0, 30);
  pot.contribute(1, 60);
  pot.contribute(2, 100);
  pot.resolve();

  ASSERT_EQ(pot.side_pots().size(), 3u);
  EXPECT_EQ(pot.total(), 190);

  EXPECT_EQ(pot.side_pots()[0].amount, 90);
  EXPECT_EQ(pot.side_pots()[0].eligible_seats.size(), 3u);

  EXPECT_EQ(pot.side_pots()[1].amount, 60);
  EXPECT_EQ(pot.side_pots()[1].eligible_seats.size(), 2u);

  EXPECT_EQ(pot.side_pots()[2].amount, 40);
  ASSERT_EQ(pot.side_pots()[2].eligible_seats.size(), 1u);
  EXPECT_EQ(pot.side_pots()[2].eligible_seats[0], 2);
}

TEST(PotTest, EqualContributionsProduceOneSidePotWithAllSeatsEligible) {
  Pot pot;
  pot.contribute(0, 100);
  pot.contribute(1, 100);
  pot.contribute(2, 100);
  pot.resolve();

  ASSERT_EQ(pot.side_pots().size(), 1u);
  EXPECT_EQ(pot.side_pots()[0].amount, 300);
  EXPECT_EQ(pot.side_pots()[0].eligible_seats.size(), 3u);
}

TEST(PotTest, ResetClearsContributionsAndSidePots) {
  Pot pot;
  pot.contribute(0, 200);
  pot.contribute(1, 200);
  pot.resolve();
  ASSERT_GT(pot.total(), 0);

  pot.reset();

  EXPECT_EQ(pot.total(), 0);
  EXPECT_TRUE(pot.side_pots().empty());
}
