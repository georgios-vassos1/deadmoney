#include "BettingRound.hpp"
#include <algorithm>

BettingRound::BettingRound(int num_seats, int big_blind_size)
    : _num_seats(num_seats)
    , _current_bet(0)
    , _min_raise_to(big_blind_size * 2)
    , _active(num_seats, false)
    , _needs_to_act(num_seats, false)
    , _stacks(num_seats, 0)
    , _bets(num_seats, 0)
{}

void BettingRound::set_active(int seat, int stack, int street_bet) {
    _active[seat]       = true;
    _needs_to_act[seat] = true;
    _stacks[seat]       = stack;
    _bets[seat]         = street_bet;
    if (street_bet > _current_bet)
        _current_bet = street_bet;
}

bool BettingRound::is_over() const {
    for (int i = 0; i < _num_seats; ++i)
        if (_needs_to_act[i]) return false;
    return true;
}

bool BettingRound::is_action_valid(int seat, BetAction action) const {
    if (!_active[seat] || !_needs_to_act[seat]) return false;
    switch (action.action) {
    case Action::Fold:  return true;
    case Action::Check: return _bets[seat] == _current_bet;
    case Action::Call:  return _bets[seat] < _current_bet && _stacks[seat] > 0;
    case Action::Raise:
        return action.amount >= _min_raise_to
            && _stacks[seat] > (_current_bet - _bets[seat]);
    case Action::AllIn: return _stacks[seat] > 0;
    }
    return false;
}

void BettingRound::apply(int seat, BetAction action, int& stack, int& street_bet) {
    switch (action.action) {

    case Action::Check:
        _needs_to_act[seat] = false;
        break;

    case Action::Fold:
        _active[seat]       = false;
        _needs_to_act[seat] = false;
        break;

    case Action::Call: {
        const int to_call  = std::min(_current_bet - _bets[seat], _stacks[seat]);
        _stacks[seat]     -= to_call;
        _bets[seat]       += to_call;
        stack             -= to_call;
        street_bet        += to_call;
        _needs_to_act[seat] = false;
        break;
    }

    case Action::Raise: {
        const int to_put_in  = action.amount - _bets[seat];
        const int raise_size = action.amount - _current_bet;
        _stacks[seat]       -= to_put_in;
        _bets[seat]          = action.amount;
        stack               -= to_put_in;
        street_bet           = action.amount;
        _current_bet         = action.amount;
        _min_raise_to        = action.amount + raise_size;
        _needs_to_act[seat]  = false;
        // Reopen action for every other active player.
        for (int i = 0; i < _num_seats; ++i)
            if (_active[i] && i != seat) _needs_to_act[i] = true;
        break;
    }

    case Action::AllIn: {
        const int to_put_in = _stacks[seat];
        const int new_total = _bets[seat] + to_put_in;
        _stacks[seat]       = 0;
        _bets[seat]         = new_total;
        stack              -= to_put_in;
        street_bet          = new_total;
        if (new_total > _current_bet) {
            // Only reopen action if this constitutes a full raise.
            if (new_total >= _min_raise_to) {
                const int raise_size = new_total - _current_bet;
                _min_raise_to        = new_total + raise_size;
                for (int i = 0; i < _num_seats; ++i)
                    if (_active[i] && i != seat) _needs_to_act[i] = true;
            }
            _current_bet = new_total;
        }
        _active[seat]       = false; // all-in players cannot act further
        _needs_to_act[seat] = false;
        break;
    }
    }
}
