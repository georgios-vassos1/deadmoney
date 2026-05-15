#pragma once

#include "BettingRound.hpp"
#include "Card.hpp"
#include "Deck.hpp"
#include "Player.hpp"
#include "Pot.hpp"

#include <optional>
#include <string>
#include <vector>

class Table {
public:
    Table(int num_seats, int small_blind, int big_blind);

    void seat_player(int seat, std::string name, int stack);
    void start_hand(int dealer_button);
    void deal_community(int n);
    void apply(int seat, BetAction action);
    void new_street(int first_to_act);
    std::vector<int> award_pot();

    const Player&            player(int seat) const { return _players[seat]; }
    const Pot&               pot()            const { return _pot; }
    const std::vector<Card>& community()      const { return _community; }
    bool                     street_over()    const;

private:
    int                         _num_seats;
    int                         _small_blind;
    int                         _big_blind;
    int                         _dealer;
    std::vector<Player>         _players;
    Deck                        _deck;
    Pot                         _pot;
    std::vector<Card>           _community;
    std::optional<BettingRound> _round;
};
