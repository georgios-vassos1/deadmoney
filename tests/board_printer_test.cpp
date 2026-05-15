#include "BoardPrinter.hpp"
#include "Table.hpp"

#include <gtest/gtest.h>
#include <sstream>

// Preset deck for 2 players — same layout as table_test.cpp.
// deal() pops from back; vector order: [River..., P0c1 at back].
static Deck make_known_deck() {
    return Deck(std::vector<Card>{
        {Rank::Ten,   Suit::Spades},
        {Rank::Jack,  Suit::Spades},
        {Rank::Queen, Suit::Spades},
        {Rank::King,  Suit::Spades},
        {Rank::Ace,   Suit::Spades},
        {Rank::Five,  Suit::Hearts},
        {Rank::Four,  Suit::Hearts},
        {Rank::Three, Suit::Hearts},
        {Rank::Two,   Suit::Hearts},
    });
}

static Table started_table() {
    Table t(2, 5, 10);
    t.seat_player(0, "Alice", 1000);
    t.seat_player(1, "Bob",   1000);
    t.start_hand(0, make_known_deck());
    return t;
}

TEST(BoardPrinterTest, PrintsHoleCardsForHumanSeat) {
  const auto table = started_table();
  std::ostringstream out;
  BoardPrinter::print(out, table, /*human_seat=*/0);
  const auto s = out.str();
  // Seat 0 gets 2h and 3h (per preset deck).
  EXPECT_NE(s.find("2h"), std::string::npos);
  EXPECT_NE(s.find("3h"), std::string::npos);
}

TEST(BoardPrinterTest, PrintsHiddenCardsForAISeat) {
  const auto table = started_table();
  std::ostringstream out;
  BoardPrinter::print(out, table, /*human_seat=*/0);
  const auto s = out.str();
  EXPECT_NE(s.find("??"), std::string::npos);
}

TEST(BoardPrinterTest, PrintsCommunityCardsAfterFlop) {
  auto table = started_table();
  table.apply(1, {Action::Call,  0});
  table.apply(0, {Action::Check, 0});
  table.new_street(0);
  table.deal_community(3); // As Ks Qs

  std::ostringstream out;
  BoardPrinter::print(out, table, /*human_seat=*/0);
  const auto s = out.str();
  EXPECT_NE(s.find("As"), std::string::npos);
  EXPECT_NE(s.find("Ks"), std::string::npos);
  EXPECT_NE(s.find("Qs"), std::string::npos);
}

TEST(BoardPrinterTest, PrintsPotTotal) {
  auto table = started_table();
  table.apply(1, {Action::Call, 0});

  std::ostringstream out;
  BoardPrinter::print(out, table, /*human_seat=*/0);
  EXPECT_NE(out.str().find("20"), std::string::npos); // pot = 10+10
}

TEST(BoardPrinterTest, PrintsPlayerStacks) {
  const auto table = started_table();
  std::ostringstream out;
  BoardPrinter::print(out, table, /*human_seat=*/0);
  const auto s = out.str();
  EXPECT_NE(s.find("Alice"), std::string::npos);
  EXPECT_NE(s.find("Bob"),   std::string::npos);
}
