#pragma once

#include <stdexcept>
#include <string>
#include <vector>

enum class Action { Fold, Check, Call, Raise, AllIn };

const char*  to_string(Action a);
Action       action_from_string(const std::string& s);

struct BetAction {
    Action action;
    int    amount; // raise-to total; only meaningful for Raise
};

class BettingRound {
public:
    BettingRound(int num_seats, int big_blind_size);

    void set_active(int seat, int stack, int street_bet = 0);

    bool is_action_valid(int seat, BetAction action) const;
    void apply(int seat, BetAction action, int& stack, int& street_bet);

    bool is_over()      const;
    int  current_bet()  const { return _current_bet; }
    int  min_raise_to() const { return _min_raise_to; }

private:
    int               _num_seats;
    int               _current_bet;
    int               _min_raise_to;
    std::vector<bool> _active;
    std::vector<bool> _needs_to_act;
    std::vector<int>  _stacks;
    std::vector<int>  _bets;
};
