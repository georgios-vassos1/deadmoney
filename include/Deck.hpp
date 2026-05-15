#pragma once

#include "Card.hpp"
#include <stdexcept>
#include <vector>

class Deck {
public:
    Deck();
    explicit Deck(std::vector<Card> preset); // preset ordering, for replay/testing

    Card deal();
    void shuffle();
    void reset();

    int  size()  const { return static_cast<int>(_cards.size()); }
    bool empty() const { return _cards.empty(); }

private:
    void populate();
    std::vector<Card> _cards;
};
