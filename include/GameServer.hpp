#pragma once

#include "HttpPolicy.hpp"
#include "Session.hpp"
#include "Table.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

class GameServer {
public:
    GameServer(int num_seats, int stack, int small_blind, int big_blind,
               std::vector<int> human_seats, int max_hands);
    ~GameServer();

    void          push_action(int seat, BetAction action);
    const Table*  current_state() const;
    int           hands_played() const;
    bool          is_done() const;
    int           num_seats() const { return _num_seats; }

private:
    int                                      _num_seats;
    std::vector<std::shared_ptr<HttpPolicy>> _http_policies; // indexed by seat, null for bots
    std::unique_ptr<Session>                 _session;
    std::thread                              _thread;
    std::atomic<bool>                        _done{false};
};
