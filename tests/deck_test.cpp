#include "Deck.hpp"

#include <gtest/gtest.h>
#include <set>
#include <stdexcept>

TEST(DeckTest, FreshDeckHas52Cards) {
  const Deck deck;
  EXPECT_EQ(deck.size(), 52);
}

TEST(DeckTest, DealReducesSizeByOne) {
  Deck deck;
  const auto first = deck.deal();
  EXPECT_EQ(deck.size(), 51);
  (void)first;
}

TEST(DeckTest, DealFromEmptyDeckThrows) {
  Deck deck;
  while (!deck.empty()) deck.deal();
  EXPECT_THROW(deck.deal(), std::runtime_error);
}

// Encodes a Card as a unique integer so it can be stored in a std::set.
static int card_key(const Card& c) {
  return static_cast<int>(c.suit) * 13 + (static_cast<int>(c.rank) - 2);
}

TEST(DeckTest, ResetRestoresDepletedDeckTo52Cards) {
  Deck deck;
  while (!deck.empty()) deck.deal();
  ASSERT_EQ(deck.size(), 0);
  deck.reset();
  EXPECT_EQ(deck.size(), 52);
}

TEST(DeckTest, ShufflePreservesAll52UniqueCards) {
  Deck deck;
  deck.shuffle();

  ASSERT_EQ(deck.size(), 52);

  std::set<int> seen;
  while (!deck.empty())
    seen.insert(card_key(deck.deal()));

  EXPECT_EQ(static_cast<int>(seen.size()), 52);
}
