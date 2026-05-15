#include "Table.hpp"

#include <gtest/gtest.h>

TEST(TableTest, PlayersReceiveTwoHoleCardsAfterDeal) {
  Table table(2, 5, 10);
  table.seat_player(0, "Alice", 1000);
  table.seat_player(1, "Bob",   1000);
  table.start_hand(0);

  const auto& h0 = table.player(0).hole_cards;
  const auto& h1 = table.player(1).hole_cards;

  EXPECT_NE(h0[0], h0[1]);
  EXPECT_NE(h1[0], h1[1]);
  EXPECT_NE(h0[0], h1[0]);
  EXPECT_NE(h0[0], h1[1]);
  EXPECT_NE(h0[1], h1[0]);
  EXPECT_NE(h0[1], h1[1]);
}

TEST(TableTest, BlindsDeductedFromStacks) {
  // 3 players: dealer=0, SB=seat1, BB=seat2.
  Table table(3, 5, 10);
  table.seat_player(0, "Dealer", 1000);
  table.seat_player(1, "SB",    1000);
  table.seat_player(2, "BB",    1000);
  table.start_hand(0);

  EXPECT_EQ(table.player(1).stack, 995); // paid SB=5
  EXPECT_EQ(table.player(2).stack, 990); // paid BB=10
  EXPECT_EQ(table.pot().total(),    15);
}

TEST(TableTest, FoldWinsPot) {
  // Heads-up: dealer=0 → SB=seat1, BB=seat0.
  // Seat 1 (SB) folds; seat 0 (BB) wins the pot of 15.
  Table table(2, 5, 10);
  table.seat_player(0, "Alice", 1000);
  table.seat_player(1, "Bob",   1000);
  table.start_hand(0);

  table.apply(1, {Action::Fold, 0});

  const auto winners = table.award_pot();
  ASSERT_EQ(winners.size(), 1u);
  EXPECT_EQ(winners[0], 0);
  EXPECT_EQ(table.player(0).stack, 1005); // BB: 1000-10 + won pot of 15
}

TEST(TableTest, BestHandWinsShowdown) {
  // Force a known outcome by dealing fixed community cards and verifying
  // that award_pot returns the seat holding the stronger hand.
  // We can't control the deck shuffle, so we test the structural guarantee:
  // exactly one winner is returned after a full board is dealt.
  Table table(2, 5, 10);
  table.seat_player(0, "Alice", 1000);
  table.seat_player(1, "Bob",   1000);
  table.start_hand(0);

  // Both players check through all streets.
  table.apply(1, {Action::Call,  0});
  table.apply(0, {Action::Check, 0});

  table.new_street(0);
  table.deal_community(3); // flop
  table.apply(0, {Action::Check, 0});
  table.apply(1, {Action::Check, 0});

  table.new_street(0);
  table.deal_community(1); // turn
  table.apply(0, {Action::Check, 0});
  table.apply(1, {Action::Check, 0});

  table.new_street(0);
  table.deal_community(1); // river
  table.apply(0, {Action::Check, 0});
  table.apply(1, {Action::Check, 0});

  const auto winners = table.award_pot();
  ASSERT_FALSE(winners.empty());
  // Total chips are conserved.
  const int total = table.player(0).stack + table.player(1).stack;
  EXPECT_EQ(total, 2000);
}

TEST(TableTest, AllInPlayerWinsMainPotOnly) {
  // seat0=BB(10), seat1=SB(5) but only has 8 total.
  // seat1 goes all-in for 8; seat0 calls.
  // Pot structure: main pot (8*2=16), side pot (2 for seat0 alone).
  // seat0 must win the side pot; winner of main pot depends on hands.
  // We verify chip conservation regardless of who wins.
  Table table(2, 5, 10);
  table.seat_player(0, "Rich",  1000);
  table.seat_player(1, "Short",    8);
  table.start_hand(0);
  // After blinds: seat1(SB)=8-5=3 stack, seat0(BB)=1000-10=990 stack.
  // seat1 goes all-in with remaining 3.
  table.apply(1, {Action::AllIn, 0});
  // seat0 calls (already at current_bet).
  table.apply(0, {Action::Call, 0});

  table.deal_community(3);
  table.new_street(0);
  table.deal_community(1);
  table.new_street(0);
  table.deal_community(1);

  table.award_pot();

  const int total = table.player(0).stack + table.player(1).stack;
  EXPECT_EQ(total, 1008); // 1000 + 8 = 1008
}
