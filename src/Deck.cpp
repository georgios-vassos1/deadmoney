#include "Deck.hpp"
#include <algorithm>
#include <random>
#include <stdexcept>

static constexpr Rank kRanks[] = {
    Rank::Two,   Rank::Three, Rank::Four, Rank::Five, Rank::Six,
    Rank::Seven, Rank::Eight, Rank::Nine, Rank::Ten,
    Rank::Jack,  Rank::Queen, Rank::King, Rank::Ace
};
static constexpr Suit kSuits[] = {
    Suit::Clubs, Suit::Diamonds, Suit::Hearts, Suit::Spades
};

void Deck::populate() {
    _cards.clear();
    _cards.reserve(52);
    for (const Suit s : kSuits)
        for (const Rank r : kRanks)
            _cards.push_back({r, s});
}

Deck::Deck()                       { populate(); }
Deck::Deck(std::vector<Card> preset) : _cards(std::move(preset)) {}
void Deck::reset() { populate(); }

void Deck::shuffle() {
    static std::mt19937 rng{std::random_device{}()};
    std::shuffle(_cards.begin(), _cards.end(), rng);
}

Card Deck::deal() {
    if (_cards.empty())
        throw std::runtime_error("Deck::deal — deck is empty");
    const Card c = _cards.back();
    _cards.pop_back();
    return c;
}
