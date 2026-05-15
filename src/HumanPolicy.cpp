#include "HumanPolicy.hpp"
#include "Table.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

HumanPolicy::HumanPolicy(std::istream& input) : _input(input) {}

BetAction HumanPolicy::act(int seat, const Table& table) {
    std::string line;
    while (std::getline(_input, line)) {
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        BetAction action{Action::Fold, 0};
        bool recognised = false;
        if (cmd == "fold")  { action = {Action::Fold,  0}; recognised = true; }
        if (cmd == "check") { action = {Action::Check, 0}; recognised = true; }
        if (cmd == "call")  { action = {Action::Call,  0}; recognised = true; }
        if (cmd == "allin") { action = {Action::AllIn, 0}; recognised = true; }
        if (cmd == "raise") {
            int amount = 0;
            ss >> amount;
            action = {Action::Raise, amount};
            recognised = true;
        }
        if (!recognised) continue;

        if (!table.has_active_round() || table.current_round().is_action_valid(seat, action))
            return action;

        // Print hint only when stdin is a terminal (i.e. interactive session).
        if (&_input == &std::cin)
            std::cout << "Invalid action — try: fold / check / call / raise N / allin\n";
    }
    return {Action::Fold, 0}; // EOF — fold is the safe default
}
