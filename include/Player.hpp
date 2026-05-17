#pragma once

#include "Card.hpp"
#include <array>
#include <string>

enum class PlayerStatus { Active, Folded, AllIn };

const char* to_string(PlayerStatus s);

struct Player {
    std::string         name;
    int                 stack      = 0;
    int                 street_bet = 0;
    PlayerStatus        status     = PlayerStatus::Active;
    std::array<Card, 2> hole_cards = {Card{Rank::Two, Suit::Clubs},
                                      Card{Rank::Two, Suit::Clubs}};

    // Deducts up to amount from stack; goes AllIn if stack is exhausted.
    // Returns the actual amount deducted.
    int  post(int amount);

    void fold();
    void reset_street();

    bool is_active() const { return status == PlayerStatus::Active; }
    bool is_folded() const { return status == PlayerStatus::Folded; }
    bool is_all_in() const { return status == PlayerStatus::AllIn;  }
};
