#include "Session.hpp"
#include <algorithm>
#include <numeric>

Session::Session(Table table, std::vector<std::shared_ptr<PlayerPolicy>> policies)
    : _table(std::move(table))
    , _policies(std::move(policies))
    , _dealer_idx(0)
    , _hands_played(0)
{
    _active_seats.resize(_table.num_seats());
    std::iota(_active_seats.begin(), _active_seats.end(), 0);
}

int Session::dealer_seat() const {
    return _active_seats[_dealer_idx];
}

void Session::run_street(int first_to_act) {
    const int n = _table.num_seats();
    while (!_table.street_over()) {
        for (int offset = 0; offset < n; ++offset) {
            if (_table.street_over()) break;
            const int seat = (first_to_act + offset) % n;
            // is_action_valid(seat, Fold) is true iff the seat is active and needs to act.
            if (_table.current_round().is_action_valid(seat, {Action::Fold, 0}))
                _table.apply(seat, _policies[seat]->act(seat, _table));
        }
    }
}

void Session::run_one_hand() {
    const int dealer       = dealer_seat();
    const int after_dealer = (dealer + 1) % _table.num_seats();

    _table.start_hand(dealer);
    run_street(after_dealer);

    _table.deal_community(3);
    _table.new_street(after_dealer);
    run_street(after_dealer);

    _table.deal_community(1);
    _table.new_street(after_dealer);
    run_street(after_dealer);

    _table.deal_community(1);
    _table.new_street(after_dealer);
    run_street(after_dealer);

    _table.award_pot();
    remove_busted_seats();
    advance_dealer();
    ++_hands_played;
}

void Session::run(int max_hands) {
    for (int i = 0; i < max_hands; ++i)
        run_one_hand();
}

void Session::advance_dealer() {
    if (_active_seats.empty()) return;
    _dealer_idx = (_dealer_idx + 1) % static_cast<int>(_active_seats.size());
}

void Session::remove_busted_seats() {
    const int current_dealer = dealer_seat();
    std::vector<int> surviving;
    for (const int seat : _active_seats)
        if (_table.player(seat).stack > 0)
            surviving.push_back(seat);

    _active_seats = surviving;
    if (_active_seats.empty()) { _dealer_idx = 0; return; }

    // Keep _dealer_idx pointing to the current dealer or the next surviving seat.
    _dealer_idx = static_cast<int>(_active_seats.size()) - 1;
    for (int i = 0; i < static_cast<int>(_active_seats.size()); ++i) {
        if (_active_seats[i] >= current_dealer) { _dealer_idx = i; break; }
    }
}
