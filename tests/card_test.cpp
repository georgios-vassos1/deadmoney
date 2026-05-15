#include "Card.hpp"

#include <gtest/gtest.h>
#include <string>

TEST(CardTest, EqualCardsCompareEqual) {
  const Card a{Rank::Ace, Suit::Spades};
  const Card b{Rank::Ace, Suit::Spades};
  EXPECT_EQ(a, b);
}

TEST(CardTest, DifferentRankComparesUnequal) {
  const Card a{Rank::Ace,  Suit::Spades};
  const Card b{Rank::King, Suit::Spades};
  EXPECT_NE(a, b);
}

TEST(CardTest, DifferentSuitComparesUnequal) {
  const Card a{Rank::Ace, Suit::Spades};
  const Card b{Rank::Ace, Suit::Hearts};
  EXPECT_NE(a, b);
}

TEST(CardTest, ToStringProducesRankSuitNotation) {
  EXPECT_EQ(to_string(Card{Rank::Ace,   Suit::Spades}),   std::string("As"));
  EXPECT_EQ(to_string(Card{Rank::King,  Suit::Hearts}),   std::string("Kh"));
  EXPECT_EQ(to_string(Card{Rank::Ten,   Suit::Diamonds}), std::string("Td"));
  EXPECT_EQ(to_string(Card{Rank::Two,   Suit::Clubs}),    std::string("2c"));
}
