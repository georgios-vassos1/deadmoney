#include "HandEvaluator.hpp"

#include <gtest/gtest.h>

#include <array>
#include <tuple>
#include <vector>

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::array<Card, 5> H(Card a, Card b, Card c, Card d, Card e) {
  return {a, b, c, d, e};
}
static Card C(Rank r, Suit s) { return {r, s}; }

// Short suit aliases for readability in hand literals.
static constexpr Suit Cl = Suit::Clubs;
static constexpr Suit Di = Suit::Diamonds;
static constexpr Suit He = Suit::Hearts;
static constexpr Suit Sp = Suit::Spades;

// ── TEST_P: hand category classification ─────────────────────────────────────

using CategoryParam = std::tuple<std::array<Card, 5>, HandCategory>;

class HandCategoryTest : public ::testing::TestWithParam<CategoryParam> {};

TEST_P(HandCategoryTest, ClassifiesCorrectly) {
  const auto& [hand, expected] = GetParam();
  EXPECT_EQ(HandEvaluator::evaluate(hand).category, expected);
}

INSTANTIATE_TEST_SUITE_P(
    AllCategories,
    HandCategoryTest,
    ::testing::Values(
        // HighCard: A K Q J 9, mixed suits, no straight
        CategoryParam{H(C(Rank::Ace,   Sp), C(Rank::King,  He),
                        C(Rank::Queen, Di), C(Rank::Jack,  Cl),
                        C(Rank::Nine,  Sp)),
                      HandCategory::HighCard},

        // OnePair: AA KQJ
        CategoryParam{H(C(Rank::Ace,   Sp), C(Rank::Ace,   He),
                        C(Rank::King,  Di), C(Rank::Queen, Cl),
                        C(Rank::Jack,  Sp)),
                      HandCategory::OnePair},

        // TwoPair: AAKKQ
        CategoryParam{H(C(Rank::Ace,   Sp), C(Rank::Ace,   He),
                        C(Rank::King,  Di), C(Rank::King,  Cl),
                        C(Rank::Queen, Sp)),
                      HandCategory::TwoPair},

        // ThreeOfAKind: AAAKQ
        CategoryParam{H(C(Rank::Ace,   Sp), C(Rank::Ace,   He),
                        C(Rank::Ace,   Di), C(Rank::King,  Cl),
                        C(Rank::Queen, Sp)),
                      HandCategory::ThreeOfAKind},

        // Straight: 5-6-7-8-9 mixed suits
        CategoryParam{H(C(Rank::Five,  Sp), C(Rank::Six,   He),
                        C(Rank::Seven, Di), C(Rank::Eight, Cl),
                        C(Rank::Nine,  Sp)),
                      HandCategory::Straight},

        // Flush: 2-5-7-J-A all spades, not a straight
        CategoryParam{H(C(Rank::Two,   Sp), C(Rank::Five,  Sp),
                        C(Rank::Seven, Sp), C(Rank::Jack,  Sp),
                        C(Rank::Ace,   Sp)),
                      HandCategory::Flush},

        // FullHouse: AAAKK
        CategoryParam{H(C(Rank::Ace,   Sp), C(Rank::Ace,   He),
                        C(Rank::Ace,   Di), C(Rank::King,  Cl),
                        C(Rank::King,  Sp)),
                      HandCategory::FullHouse},

        // FourOfAKind: AAAAK
        CategoryParam{H(C(Rank::Ace,   Sp), C(Rank::Ace,   He),
                        C(Rank::Ace,   Di), C(Rank::Ace,   Cl),
                        C(Rank::King,  Sp)),
                      HandCategory::FourOfAKind},

        // StraightFlush: 5-6-7-8-9 all spades
        CategoryParam{H(C(Rank::Five,  Sp), C(Rank::Six,   Sp),
                        C(Rank::Seven, Sp), C(Rank::Eight, Sp),
                        C(Rank::Nine,  Sp)),
                      HandCategory::StraightFlush},

        // RoyalFlush: T-J-Q-K-A all spades
        CategoryParam{H(C(Rank::Ten,   Sp), C(Rank::Jack,  Sp),
                        C(Rank::Queen, Sp), C(Rank::King,  Sp),
                        C(Rank::Ace,   Sp)),
                      HandCategory::RoyalFlush}));

// ── Category ordering ─────────────────────────────────────────────────────────

TEST(HandEvaluatorTest, CategoryOrderingIsCorrect) {
  // Verify the full ranking chain: HC < OP < TP < TOK < ST < FL < FH < FOK < SF < RF
  const auto hc  = HandEvaluator::evaluate(H(C(Rank::Ace,  Sp),C(Rank::King, He),
                                              C(Rank::Queen,Di),C(Rank::Jack, Cl),C(Rank::Nine,Sp)));
  const auto op  = HandEvaluator::evaluate(H(C(Rank::Ace,  Sp),C(Rank::Ace,  He),
                                              C(Rank::King, Di),C(Rank::Queen,Cl),C(Rank::Jack,Sp)));
  const auto tp  = HandEvaluator::evaluate(H(C(Rank::Ace,  Sp),C(Rank::Ace,  He),
                                              C(Rank::King, Di),C(Rank::King, Cl),C(Rank::Queen,Sp)));
  const auto tok = HandEvaluator::evaluate(H(C(Rank::Ace,  Sp),C(Rank::Ace,  He),
                                              C(Rank::Ace,  Di),C(Rank::King, Cl),C(Rank::Queen,Sp)));
  const auto st  = HandEvaluator::evaluate(H(C(Rank::Five, Sp),C(Rank::Six,  He),
                                              C(Rank::Seven,Di),C(Rank::Eight,Cl),C(Rank::Nine,Sp)));
  const auto fl  = HandEvaluator::evaluate(H(C(Rank::Two,  Sp),C(Rank::Five, Sp),
                                              C(Rank::Seven,Sp),C(Rank::Jack, Sp),C(Rank::Ace, Sp)));
  const auto fh  = HandEvaluator::evaluate(H(C(Rank::Ace,  Sp),C(Rank::Ace,  He),
                                              C(Rank::Ace,  Di),C(Rank::King, Cl),C(Rank::King,Sp)));
  const auto fok = HandEvaluator::evaluate(H(C(Rank::Ace,  Sp),C(Rank::Ace,  He),
                                              C(Rank::Ace,  Di),C(Rank::Ace,  Cl),C(Rank::King,Sp)));
  const auto sf  = HandEvaluator::evaluate(H(C(Rank::Five, Sp),C(Rank::Six,  Sp),
                                              C(Rank::Seven,Sp),C(Rank::Eight,Sp),C(Rank::Nine,Sp)));
  const auto rf  = HandEvaluator::evaluate(H(C(Rank::Ten,  Sp),C(Rank::Jack, Sp),
                                              C(Rank::Queen,Sp),C(Rank::King, Sp),C(Rank::Ace, Sp)));

  EXPECT_LT(hc,  op);
  EXPECT_LT(op,  tp);
  EXPECT_LT(tp,  tok);
  EXPECT_LT(tok, st);
  EXPECT_LT(st,  fl);
  EXPECT_LT(fl,  fh);
  EXPECT_LT(fh,  fok);
  EXPECT_LT(fok, sf);
  EXPECT_LT(sf,  rf);
}

// ── Kicker tiebreak ───────────────────────────────────────────────────────────

TEST(HandEvaluatorTest, KickerBreaksTieWithinSameCategory) {
  // AA K beats AA Q — same pair, king kicker wins
  const auto high_kicker = HandEvaluator::evaluate(H(
      C(Rank::Ace,  Sp), C(Rank::Ace,  He),
      C(Rank::King, Di), C(Rank::Three,Cl), C(Rank::Two,  Sp)));
  const auto low_kicker  = HandEvaluator::evaluate(H(
      C(Rank::Ace,  Cl), C(Rank::Ace,  Di),
      C(Rank::Queen,Sp), C(Rank::Three,He), C(Rank::Two,  Di)));

  EXPECT_GT(high_kicker, low_kicker);
}

// ── Wheel straight (A-2-3-4-5) ───────────────────────────────────────────────

TEST(HandEvaluatorTest, WheelIsAStraightAndLosesToSixHighStraight) {
  const auto wheel = HandEvaluator::evaluate(H(
      C(Rank::Ace,  Sp), C(Rank::Two,  He),
      C(Rank::Three,Di), C(Rank::Four, Cl), C(Rank::Five, Sp)));
  const auto six_high = HandEvaluator::evaluate(H(
      C(Rank::Two,  Sp), C(Rank::Three,He),
      C(Rank::Four, Di), C(Rank::Five, Cl), C(Rank::Six,  Sp)));

  EXPECT_EQ(wheel.category,    HandCategory::Straight);
  EXPECT_GT(six_high, wheel);
}

// ── 7-card best-hand selection ────────────────────────────────────────────────

TEST(HandEvaluatorTest, SevenCardEvaluatorFindsRoyalFlushInHoleAndBoard) {
  // Hold: Ac Kc  Board: Qc Jc Tc 2h 3d → best 5 is royal flush
  const std::vector<Card> cards = {
      C(Rank::Ace,   Cl), C(Rank::King,  Cl),
      C(Rank::Queen, Cl), C(Rank::Jack,  Cl), C(Rank::Ten,   Cl),
      C(Rank::Two,   He), C(Rank::Three, Di)
  };
  EXPECT_EQ(HandEvaluator::evaluate(cards).category, HandCategory::RoyalFlush);
}

TEST(HandEvaluatorTest, SevenCardEvaluatorPrefersBestHandOverWeaker) {
  // Hold: As Ah  Board: Ks Kh Kd 2c 3d → best is full house KKK-AA, not just trips
  const std::vector<Card> cards = {
      C(Rank::Ace,  Sp), C(Rank::Ace,  He),
      C(Rank::King, Sp), C(Rank::King, He), C(Rank::King, Di),
      C(Rank::Two,  Cl), C(Rank::Three,Di)
  };
  EXPECT_EQ(HandEvaluator::evaluate(cards).category, HandCategory::FullHouse);
}
