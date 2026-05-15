#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

enum class Rank : uint8_t {
    Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
    Jack, Queen, King, Ace
};

enum class Suit : uint8_t { Clubs, Diamonds, Hearts, Spades };

struct Card {
    Rank rank;
    Suit suit;

    bool operator==(const Card& o) const { return rank == o.rank && suit == o.suit; }
    bool operator!=(const Card& o) const { return !(*this == o); }
};

inline std::string to_string(Rank r) {
    switch (r) {
    case Rank::Two:   return "2"; case Rank::Three: return "3";
    case Rank::Four:  return "4"; case Rank::Five:  return "5";
    case Rank::Six:   return "6"; case Rank::Seven: return "7";
    case Rank::Eight: return "8"; case Rank::Nine:  return "9";
    case Rank::Ten:   return "T"; case Rank::Jack:  return "J";
    case Rank::Queen: return "Q"; case Rank::King:  return "K";
    case Rank::Ace:   return "A";
    }
    throw std::invalid_argument("unknown Rank");
}

inline std::string to_string(Suit s) {
    switch (s) {
    case Suit::Clubs:    return "c"; case Suit::Diamonds: return "d";
    case Suit::Hearts:   return "h"; case Suit::Spades:   return "s";
    }
    throw std::invalid_argument("unknown Suit");
}

inline std::string to_string(const Card& c) { return to_string(c.rank) + to_string(c.suit); }
