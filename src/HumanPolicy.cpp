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

        Action action;
        try {
            action = action_from_string(cmd);
        } catch (const std::invalid_argument&) {
            continue;
        }

        int amount = 0;
        if (action == Action::Raise)
            ss >> amount;

        BetAction bet{action, amount};
        if (!table.has_active_round() || table.current_round().is_action_valid(seat, bet))
            return bet;

        if (&_input == &std::cin)
            std::cout << "Invalid action — try: fold / check / call / raise N / allin\n";
    }
    return {Action::Fold, 0};
}
