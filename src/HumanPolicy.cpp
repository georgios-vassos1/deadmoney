#include "HumanPolicy.hpp"
#include <algorithm>
#include <sstream>
#include <string>

HumanPolicy::HumanPolicy(std::istream& input) : _input(input) {}

BetAction HumanPolicy::act(int /*seat*/, const Table& /*table*/) {
    std::string line;
    while (std::getline(_input, line)) {
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
        if (cmd == "fold")  return {Action::Fold,  0};
        if (cmd == "check") return {Action::Check, 0};
        if (cmd == "call")  return {Action::Call,  0};
        if (cmd == "allin") return {Action::AllIn, 0};
        if (cmd == "raise") {
            int amount = 0;
            ss >> amount;
            return {Action::Raise, amount};
        }
    }
    return {Action::Fold, 0}; // EOF or unrecognised — fold is the safe default
}
