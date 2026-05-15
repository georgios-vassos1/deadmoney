#include "HandEvaluator.hpp"
#include <algorithm>
#include <stdexcept>

// ── HandRank ──────────────────────────────────────────────────────────────────

bool HandRank::operator==(const HandRank& o) const {
    return category == o.category && tiebreakers == o.tiebreakers;
}

bool HandRank::operator<(const HandRank& o) const {
    if (category != o.category)
        return static_cast<uint8_t>(category) < static_cast<uint8_t>(o.category);
    return tiebreakers < o.tiebreakers;
}

// ── Internal helpers ──────────────────────────────────────────────────────────

namespace {

// Returns rank values sorted descending.
std::array<uint8_t, 5> sorted_desc(const std::array<Card, 5>& hand) {
    std::array<uint8_t, 5> v;
    for (int i = 0; i < 5; ++i)
        v[i] = static_cast<uint8_t>(hand[i].rank);
    std::sort(v.begin(), v.end(), std::greater<uint8_t>());
    return v;
}

bool is_flush(const std::array<Card, 5>& hand) {
    for (int i = 1; i < 5; ++i)
        if (hand[i].suit != hand[0].suit) return false;
    return true;
}

// Returns true if v (sorted desc) forms a straight; adjusts v for the wheel.
bool is_straight(std::array<uint8_t, 5>& v) {
    bool normal = true;
    for (int i = 1; i < 5; ++i)
        if (v[i] != v[i-1] - 1) { normal = false; break; }
    if (normal) return true;
    // Wheel: A-2-3-4-5 → treat Ace as 1
    if (v[0] == 14 && v[1] == 5 && v[2] == 4 && v[3] == 3 && v[4] == 2) {
        v = {5, 4, 3, 2, 1};
        return true;
    }
    return false;
}

} // namespace

// ── evaluate(5 cards) ─────────────────────────────────────────────────────────

HandRank HandEvaluator::evaluate(const std::array<Card, 5>& hand) {
    auto v      = sorted_desc(hand);
    const bool flush  = is_flush(hand);
    const bool str    = is_straight(v);

    if (flush && str)
        return {v[0] == 14 ? HandCategory::RoyalFlush : HandCategory::StraightFlush, v};

    // Count rank frequencies
    uint8_t freq[15] = {};
    for (auto r : v) freq[r]++;

    uint8_t four_rank = 0, three_rank = 0;
    uint8_t pairs[2]  = {};
    int     pair_cnt  = 0;
    for (int r = 14; r >= 2; --r) {
        const auto rv = static_cast<uint8_t>(r);
        if      (freq[r] == 4)              four_rank  = rv;
        else if (freq[r] == 3)              three_rank = rv;
        else if (freq[r] == 2 && pair_cnt < 2) pairs[pair_cnt++] = rv;
    }

    if (four_rank) {
        uint8_t kicker = 0;
        for (auto r : v) if (r != four_rank) { kicker = r; break; }
        return {HandCategory::FourOfAKind,
                {four_rank, four_rank, four_rank, four_rank, kicker}};
    }
    if (three_rank && pair_cnt >= 1)
        return {HandCategory::FullHouse,
                {three_rank, three_rank, three_rank, pairs[0], pairs[0]}};
    if (flush)  return {HandCategory::Flush, v};
    if (str)    return {HandCategory::Straight, v};
    if (three_rank) {
        std::array<uint8_t, 5> tb = {three_rank, three_rank, three_rank, 0, 0};
        int ki = 3;
        for (auto r : v) if (r != three_rank) tb[ki++] = r;
        return {HandCategory::ThreeOfAKind, tb};
    }
    if (pair_cnt == 2) {
        uint8_t kicker = 0;
        for (auto r : v) if (r != pairs[0] && r != pairs[1]) { kicker = r; break; }
        return {HandCategory::TwoPair, {pairs[0], pairs[0], pairs[1], pairs[1], kicker}};
    }
    if (pair_cnt == 1) {
        std::array<uint8_t, 5> tb = {pairs[0], pairs[0], 0, 0, 0};
        int ki = 2;
        for (auto r : v) if (r != pairs[0]) tb[ki++] = r;
        return {HandCategory::OnePair, tb};
    }
    return {HandCategory::HighCard, v};
}

// ── evaluate(N cards) — best 5 from N ────────────────────────────────────────

HandRank HandEvaluator::evaluate(const std::vector<Card>& cards) {
    const int n = static_cast<int>(cards.size());
    if (n < 5) throw std::invalid_argument("HandEvaluator::evaluate — need at least 5 cards");
    HandRank best{HandCategory::HighCard, {0,0,0,0,0}};
    bool first = true;
    for (int a = 0;   a < n-4; ++a)
    for (int b = a+1; b < n-3; ++b)
    for (int c = b+1; c < n-2; ++c)
    for (int d = c+1; d < n-1; ++d)
    for (int e = d+1; e < n;   ++e) {
        const std::array<Card,5> h = {cards[a],cards[b],cards[c],cards[d],cards[e]};
        const HandRank hr = evaluate(h);
        if (first || hr > best) { best = hr; first = false; }
    }
    return best;
}
