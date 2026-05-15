#include "Table.hpp"
#include "HandEvaluator.hpp"
#include <algorithm>

Table::Table(int num_seats, int small_blind, int big_blind)
    : _num_seats(num_seats)
    , _small_blind(small_blind)
    , _big_blind(big_blind)
    , _dealer(0)
    , _players(num_seats)
{}

void Table::seat_player(int seat, std::string name, int stack) {
    _players[seat].name  = std::move(name);
    _players[seat].stack = stack;
}

void Table::start_hand(int dealer_button, Deck preset_deck) {
    _deck = std::move(preset_deck);
    start_hand_impl(dealer_button);
}

void Table::start_hand(int dealer_button) {
    _deck.reset();
    _deck.shuffle();
    start_hand_impl(dealer_button);
}

void Table::start_hand_impl(int dealer_button) {
    _dealer = dealer_button;
    _community.clear();
    _pot.reset();

    for (auto& p : _players)
        p.status = PlayerStatus::Active;

    // Deal 2 hole cards to each player.
    for (auto& p : _players) {
        p.hole_cards[0] = _deck.deal();
        p.hole_cards[1] = _deck.deal();
        p.street_bet    = 0;
    }

    // Post blinds: SB = (dealer+1)%N, BB = (dealer+2)%N.
    const int sb = (_dealer + 1) % _num_seats;
    const int bb = (_dealer + 2) % _num_seats;

    const int sb_paid = _players[sb].post(_small_blind);
    _pot.contribute(sb, sb_paid);

    const int bb_paid = _players[bb].post(_big_blind);
    _pot.contribute(bb, bb_paid);

    // Set up preflop betting round.
    _round.emplace(_num_seats, _big_blind);
    for (int i = 0; i < _num_seats; ++i)
        _round->set_active(i, _players[i].stack, _players[i].street_bet);
}

void Table::deal_community(int n) {
    for (int i = 0; i < n; ++i)
        _community.push_back(_deck.deal());
}

void Table::apply(int seat, BetAction action) {
    const int prev_bet = _players[seat].street_bet;
    _round->apply(seat, action, _players[seat].stack, _players[seat].street_bet);
    const int delta = _players[seat].street_bet - prev_bet;
    if (delta > 0) _pot.contribute(seat, delta);
    if (action.action == Action::Fold) _players[seat].fold();
}

void Table::new_street(int first_to_act) {
    for (auto& p : _players) p.reset_street();
    _round.emplace(_num_seats, _big_blind);
    for (int i = 0; i < _num_seats; ++i)
        if (!_players[i].is_folded())
            _round->set_active(i, _players[i].stack, 0);
    (void)first_to_act; // seat order enforced by caller calling apply in sequence
}

bool Table::street_over() const {
    return _round.has_value() && _round->is_over();
}

std::vector<int> Table::award_pot() {

    _pot.resolve();

    std::vector<int> all_winners;

    for (const auto& sp : _pot.side_pots()) {
        // Find eligible seats that are not folded.
        std::vector<int> contenders;
        for (int s : sp.eligible_seats)
            if (!_players[s].is_folded()) contenders.push_back(s);

        if (contenders.empty()) continue;

        if (contenders.size() == 1) {
            _players[contenders[0]].stack += sp.amount;
            if (std::find(all_winners.begin(), all_winners.end(), contenders[0]) == all_winners.end())
                all_winners.push_back(contenders[0]);
            continue;
        }

        // Evaluate each contender's best 7-card hand.
        auto hand_of = [&](int seat) {
            std::vector<Card> cards(_community.begin(), _community.end());
            cards.push_back(_players[seat].hole_cards[0]);
            cards.push_back(_players[seat].hole_cards[1]);
            return HandEvaluator::evaluate(cards);
        };

        HandRank best_rank = hand_of(contenders[0]);
        for (int i = 1; i < static_cast<int>(contenders.size()); ++i) {
            const HandRank rank = hand_of(contenders[i]);
            if (rank > best_rank) best_rank = rank;
        }

        // Collect all seats that share the best rank (tie split).
        std::vector<int> winners;
        for (const int s : contenders)
            if (hand_of(s) == best_rank) winners.push_back(s);

        const int share     = sp.amount / static_cast<int>(winners.size());
        const int remainder = sp.amount % static_cast<int>(winners.size());

        for (const int s : winners) {
            _players[s].stack += share;
            if (std::find(all_winners.begin(), all_winners.end(), s) == all_winners.end())
                all_winners.push_back(s);
        }

        // Remainder chip goes to the first seat left of dealer among the winners.
        if (remainder > 0) {
            for (int offset = 1; offset <= _num_seats; ++offset) {
                const int s = (_dealer + offset) % _num_seats;
                if (std::find(winners.begin(), winners.end(), s) != winners.end()) {
                    _players[s].stack += remainder;
                    break;
                }
            }
        }
    }

    return all_winners;
}
