#pragma once

#include "PlayerPolicy.hpp"

#include <istream>

class HumanPolicy : public PlayerPolicy {
public:
    explicit HumanPolicy(std::istream& input);
    BetAction act(int seat, const Table& table) override;

private:
    std::istream& _input;
};
