#pragma once

#include "GameServer.hpp"
#include "httplib.h"
#include "json.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

class HoldemServer {
public:
    HoldemServer();
    ~HoldemServer();

    void start(int port);
    void stop();
    int  port() const;

private:
    void register_routes();

    static std::string         new_game_id();
    static nlohmann::json      state_json(const Table& table);
    static nlohmann::json      state_json_or_done(const GameServer& gs);

    httplib::Server                                        _svr;
    std::thread                                            _thread;
    std::mutex                                             _mu;
    std::unordered_map<std::string, std::unique_ptr<GameServer>> _games;
    int                                                    _port{0};
};
