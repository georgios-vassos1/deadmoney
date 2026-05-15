#include "BettingRound.hpp"

#include <gtest/gtest.h>

TEST(BettingRoundTest, CheckNotValidWhenBetOutstanding) {
  // Seat 0 posted BB of 20; seat 1 posted SB of 10.
  // Seat 1 still owes 10 — check is not valid.
  BettingRound round(2, 20);
  round.set_active(0, 980, 20);
  round.set_active(1, 990, 10);

  EXPECT_FALSE(round.is_action_valid(1, {Action::Check, 0}));
  EXPECT_TRUE (round.is_action_valid(1, {Action::Call,  0}));
}

TEST(BettingRoundTest, CallMatchesCurrentBetAndDeductsFromStack) {
  // BB=20 posted by seat 0; SB=10 posted by seat 1.
  // Seat 1 calls: pays 10 more to match the BB.
  BettingRound round(2, 20);
  round.set_active(0, 980, 20);
  round.set_active(1, 990, 10);

  int stack = 990, bet = 10;
  round.apply(1, {Action::Call, 0}, stack, bet);

  EXPECT_EQ(stack, 980);
  EXPECT_EQ(bet,    20);
}

TEST(BettingRoundTest, CheckClosesActionWhenNoBetOutstanding) {
  BettingRound round(2, 10);
  round.set_active(0, 1000, 0);
  round.set_active(1, 1000, 0);

  EXPECT_TRUE(round.is_action_valid(0, {Action::Check, 0}));

  int stack = 1000, bet = 0;
  round.apply(0, {Action::Check, 0}, stack, bet);
  round.apply(1, {Action::Check, 0}, stack, bet);

  EXPECT_TRUE(round.is_over());
}

TEST(BettingRoundTest, FoldRemovesPlayerFromRound) {
  BettingRound round(2, 10);
  round.set_active(0, 1000, 0);
  round.set_active(1, 1000, 0);

  int stack = 1000, bet = 0;
  round.apply(0, {Action::Fold, 0}, stack, bet);

  // Seat 1 still needs to act, but seat 0 is gone.
  EXPECT_FALSE(round.is_over());

  round.apply(1, {Action::Check, 0}, stack, bet);
  EXPECT_TRUE(round.is_over());
}

TEST(BettingRoundTest, RaiseUpdatesCurrentBetAndReopensAction) {
  // Heads-up: BB=20 (seat 0), SB=10 (seat 1).
  // Seat 1 raises to 60; seat 0 must now act again.
  BettingRound round(2, 20);
  round.set_active(0, 980, 20);
  round.set_active(1, 990, 10);

  int stack = 990, bet = 10;
  round.apply(1, {Action::Raise, 60}, stack, bet);

  EXPECT_EQ(round.current_bet(),  60);
  EXPECT_EQ(round.min_raise_to(), 100); // raise_size = 60-20 = 40; min = 60+40 = 100
  EXPECT_FALSE(round.is_over()); // seat 0 must respond
}

TEST(BettingRoundTest, AllInForLessDoesNotReopenAction) {
  // Seat 0 raised to 100; seat 1 has only 30 left — all-in for less does NOT reopen action.
  BettingRound round(2, 20);
  round.set_active(0, 900, 100);
  round.set_active(1, 30, 0);

  // Seat 0 already acted (raised); mark it as done.
  int s0 = 900, b0 = 100;
  round.apply(0, {Action::Check, 0}, s0, b0); // artificially close seat 0's action

  int stack = 30, bet = 0;
  round.apply(1, {Action::AllIn, 0}, stack, bet);

  // All-in of 30 < min_raise_to (200); action must NOT reopen for seat 0.
  EXPECT_TRUE(round.is_over());
  EXPECT_EQ(stack, 0);
  EXPECT_EQ(bet,   30);
}

TEST(BettingRoundTest, AllInForFullRaiseReopensAction) {
  // Seat 0 raised to 40 (BB=20, raise_size=20, min_raise_to=60); seat 1 goes all-in for 200 — full raise.
  BettingRound round(2, 20);
  round.set_active(0, 960, 40);
  round.set_active(1, 200, 0);

  int s0 = 960, b0 = 40;
  round.apply(0, {Action::Check, 0}, s0, b0); // close seat 0's action first

  int stack = 200, bet = 0;
  round.apply(1, {Action::AllIn, 0}, stack, bet);

  // 200 >= min_raise_to (60) => full raise => seat 0 must act again.
  EXPECT_FALSE(round.is_over());
  EXPECT_EQ(stack, 0);
  EXPECT_EQ(bet,   200);
}
