#pragma once

#include "PlayerPolicy.hpp"

class SimplePolicy : public PlayerPolicy {
public:
    BetAction act(int seat, const Table& table) override;
};
