#include "GameServer.hpp"
#include "SimplePolicy.hpp"

#include <algorithm>
#include <stdexcept>

GameServer::GameServer(int num_seats, int stack, int small_blind, int big_blind,
                       std::vector<int> human_seats, int max_hands)
{
    Table table(num_seats, small_blind, big_blind);
    for (int i = 0; i < num_seats; ++i)
        table.seat_player(i, "P" + std::to_string(i), stack);

    _http_policies.resize(num_seats);

    std::vector<std::shared_ptr<PlayerPolicy>> policies(num_seats);
    for (int i = 0; i < num_seats; ++i) {
        const bool is_human = std::find(human_seats.begin(), human_seats.end(), i) != human_seats.end();
        if (is_human) {
            auto hp = std::make_shared<HttpPolicy>();
            _http_policies[i] = hp;
            policies[i] = hp;
        } else {
            policies[i] = std::make_shared<SimplePolicy>();
        }
    }

    _session = std::make_unique<Session>(std::move(table), std::move(policies));

    _thread = std::thread([this, max_hands]() {
        _session->run(max_hands);
        _done = true;
    });
}

GameServer::~GameServer() {
    for (auto& hp : _http_policies)
        if (hp) hp->shutdown();
    if (_thread.joinable())
        _thread.join();
}

void GameServer::push_action(int seat, BetAction action) {
    if (!_http_policies[seat])
        throw std::invalid_argument("seat is not a human seat");
    _http_policies[seat]->push_action(action);
}

const Table* GameServer::current_state() const {
    for (const auto& hp : _http_policies)
        if (hp && hp->last_table_state())
            return hp->last_table_state();
    return nullptr;
}

int GameServer::hands_played() const {
    return _session->hands_played();
}

bool GameServer::is_done() const {
    return _done;
}
