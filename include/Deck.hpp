#pragma once

#include "Card.hpp"
#include <stdexcept>
#include <vector>

class Deck {
public:
    Deck();

    Card deal();
    void shuffle();
    void reset();

    int  size()  const { return static_cast<int>(_cards.size()); }
    bool empty() const { return _cards.empty(); }

private:
    void populate();
    std::vector<Card> _cards;
};
