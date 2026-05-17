#pragma once

struct GameDefaults {
    static constexpr int num_seats   = 2;
    static constexpr int stack       = 1000;
    static constexpr int small_blind = 5;
    static constexpr int big_blind   = 10;
    static constexpr int max_hands   = 100;
    static constexpr int server_port = 8080;
};
