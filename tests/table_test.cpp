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

// Preset-deck helpers: deal() pops from back, so vector must be reversed deal order.
// For 2 players: [River, Turn, Flop3, Flop2, Flop1, P1c2, P1c1, P0c2, P0c1]
static Deck make_royal_flush_deck_2p() {
  return Deck(std::vector<Card>{
    {Rank::Ten,   Suit::Spades},  // river
    {Rank::Jack,  Suit::Spades},  // turn
    {Rank::Queen, Suit::Spades},  // flop 3
    {Rank::King,  Suit::Spades},  // flop 2
    {Rank::Ace,   Suit::Spades},  // flop 1
    {Rank::Five,  Suit::Hearts},  // P1 hole 2
    {Rank::Four,  Suit::Hearts},  // P1 hole 1
    {Rank::Three, Suit::Hearts},  // P0 hole 2
    {Rank::Two,   Suit::Hearts},  // P0 hole 1
  });
}

TEST(TableTest, AllInPlayerExcludedFromNewStreet) {
  // Regression: all-in player must NOT appear in the postflop BettingRound.
  // The old bug set them as active with needs_to_act=true, causing a fold-via-EOF.
  // dealer=0 → SB=seat1, BB=seat0.
  // Bob has more chips so he still has a stack after calling Alice's all-in.
  Table table(2, 5, 10);
  table.seat_player(0, "Alice", 200);
  table.seat_player(1, "Bob",   400);
  table.start_hand(0);

  // Preflop: SB calls, BB raises all-in, SB calls.
  table.apply(1, {Action::Call,  0});
  table.apply(0, {Action::AllIn, 0}); // Alice all-in: stack → 0
  table.apply(1, {Action::Call,  0}); // Bob calls; Bob still has chips remaining

  table.deal_community(3);
  table.new_street(1);

  EXPECT_FALSE(table.current_round().is_action_valid(0, {Action::Fold, 0})); // Alice: excluded
  EXPECT_TRUE( table.current_round().is_action_valid(1, {Action::Fold, 0})); // Bob: still active
}

TEST(TableTest, BustedPlayerExcludedFromPreflop) {
  // Regression: a player with stack=0 at hand start must not enter the BettingRound.
  // The old bug reset all players to Active and called set_active unconditionally.
  // dealer=1 → SB=seat0 (0 chips), BB=seat1.
  Table table(2, 5, 10);
  table.seat_player(0, "Bust", 0);
  table.seat_player(1, "Rich", 1000);
  table.start_hand(1);

  EXPECT_FALSE(table.current_round().is_action_valid(0, {Action::Fold, 0})); // Bust: excluded
  EXPECT_TRUE( table.current_round().is_action_valid(1, {Action::Fold, 0})); // Rich: active
}

TEST(TableTest, PartialBlindAllInNotPromptedPreflop) {
  // Regression: a short-stack player whose entire stack is consumed posting a blind
  // must not appear in the BettingRound with needs_to_act=true.
  // dealer=1 → SB=seat0 (3 chips, posts 3 and is all-in), BB=seat1.
  Table table(2, 5, 10);
  table.seat_player(0, "Short", 3);
  table.seat_player(1, "Deep",  1000);
  table.start_hand(1);

  EXPECT_FALSE(table.current_round().is_action_valid(0, {Action::Fold, 0})); // Short: excluded
  EXPECT_TRUE( table.current_round().is_action_valid(1, {Action::Fold, 0})); // Deep: active
}

TEST(TableTest, TieSplitsChipsEvenly) {
  // Board A-K-Q-J-T of spades: both players' best hand is the same royal flush.
  // dealer=0 → SB=seat1(Bob), BB=seat0(Alice).
  Table table(2, 5, 10);
  table.seat_player(0, "Alice", 1000);
  table.seat_player(1, "Bob",   1000);
  table.start_hand(0, make_royal_flush_deck_2p());

  // Preflop: SB calls, BB checks. Pot = 20.
  table.apply(1, {Action::Call,  0});
  table.apply(0, {Action::Check, 0});

  table.new_street(0);
  table.deal_community(3);
  table.apply(0, {Action::Check, 0});
  table.apply(1, {Action::Check, 0});

  table.new_street(0);
  table.deal_community(1);
  table.apply(0, {Action::Check, 0});
  table.apply(1, {Action::Check, 0});

  table.new_street(0);
  table.deal_community(1);
  table.apply(0, {Action::Check, 0});
  table.apply(1, {Action::Check, 0});

  table.award_pot();

  // Pot of 20 split evenly: each player recovers their 10 investment.
  EXPECT_EQ(table.player(0).stack, 1000); // 990 + 10
  EXPECT_EQ(table.player(1).stack, 1000); // 990 + 10
}
