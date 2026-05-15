#pragma once

#include "Card.hpp"
#include <array>
#include <vector>

enum class HandCategory : uint8_t {
    HighCard = 1,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
    RoyalFlush
};

struct HandRank {
    HandCategory           category;
    std::array<uint8_t, 5> tiebreakers; // kickers, highest first

    bool operator==(const HandRank& o) const;
    bool operator< (const HandRank& o) const;
    bool operator> (const HandRank& o) const { return o < *this; }
};

class HandEvaluator {
public:
    static HandRank evaluate(const std::array<Card, 5>& hand);
    static HandRank evaluate(const std::vector<Card>& cards); // best 5 from N >= 5
};
