#include "Player.hpp"

#include <algorithm>

const char* to_string(PlayerStatus s) {
    switch (s) {
    case PlayerStatus::Active: return "active";
    case PlayerStatus::Folded: return "folded";
    case PlayerStatus::AllIn:  return "allin";
    }
    return "unknown";
}

int Player::post(int amount) {
    const int actual = std::min(amount, stack);
    stack      -= actual;
    street_bet += actual;
    if (stack == 0)
        status = PlayerStatus::AllIn;
    return actual;
}

void Player::fold() {
    status = PlayerStatus::Folded;
}

void Player::reset_street() {
    street_bet = 0;
}
