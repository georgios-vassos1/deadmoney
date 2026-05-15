#include "Player.hpp"
#include <algorithm>

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
