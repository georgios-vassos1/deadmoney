#pragma once

#include "PlayerPolicy.hpp"
#include "Table.hpp"

#include <memory>
#include <vector>

class Session {
public:
    Session(Table table, std::vector<std::shared_ptr<PlayerPolicy>> policies);

    void run_one_hand();
    void run(int max_hands);

    int              dealer_seat()   const;
    int              hands_played()  const { return _hands_played; }
    std::vector<int> active_seats()  const { return _active_seats; }
    const Table&     table()         const { return _table; }

private:
    void run_street(int first_to_act);
    void advance_dealer();
    void remove_busted_seats();

    Table                                       _table;
    std::vector<std::shared_ptr<PlayerPolicy>>  _policies;
    std::vector<int>                            _active_seats;
    int                                         _dealer_idx;
    int                                         _hands_played;
};
