#pragma once

#include "BettingRound.hpp"

class Table;

class PlayerPolicy {
public:
    virtual BetAction act(int seat, const Table& table) = 0;
    virtual ~PlayerPolicy() = default;
};
