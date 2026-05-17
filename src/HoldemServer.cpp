#include "HoldemServer.hpp"
#include "GameDefaults.hpp"

#include <sstream>
#include <iomanip>
#include <random>

using json = nlohmann::json;

HoldemServer::HoldemServer() {
    register_routes();
}

HoldemServer::~HoldemServer() {
    stop();
}

void HoldemServer::start(int port) {
    if (port == 0)
        _port = _svr.bind_to_any_port("127.0.0.1");
    else {
        _svr.bind_to_port("127.0.0.1", port);
        _port = port;
    }
    _thread = std::thread([this]() { _svr.listen_after_bind(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void HoldemServer::stop() {
    _svr.stop();
    if (_thread.joinable()) _thread.join();
}

int HoldemServer::port() const {
    return _port;
}

std::string HoldemServer::new_game_id() {
    static std::mt19937_64 rng{std::random_device{}()};
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << rng();
    return oss.str();
}

json HoldemServer::state_json(const Table& table) {
    json players = json::array();
    for (int i = 0; i < table.num_seats(); ++i) {
        const auto& p = table.player(i);
        players.push_back({
            {"seat",       i},
            {"stack",      p.stack},
            {"street_bet", p.street_bet},
            {"status",     to_string(p.status)}
        });
    }
    return {
        {"num_seats", table.num_seats()},
        {"pot",       table.pot().total()},
        {"dealer",    table.dealer()},
        {"players",   players}
    };
}

json HoldemServer::state_json_or_done(const GameServer& gs) {
    const Table* t = gs.current_state();
    json j = t ? state_json(*t) : json::object();
    j["hands_played"] = gs.hands_played();
    j["done"]         = gs.is_done();
    return j;
}

void HoldemServer::register_routes() {
    _svr.Post("/game/new", [this](const httplib::Request& req, httplib::Response& res) {
        json body;
        try { body = json::parse(req.body); }
        catch (...) { res.status = 400; return; }

        const int num_seats   = body.value("num_seats",    GameDefaults::num_seats);
        const int stack       = body.value("stack",       GameDefaults::stack);
        const int small_blind = body.value("small_blind", GameDefaults::small_blind);
        const int big_blind   = body.value("big_blind",   GameDefaults::big_blind);
        const int max_hands   = body.value("max_hands",   GameDefaults::max_hands);
        std::vector<int> human_seats;
        if (body.contains("human_seats"))
            for (auto& s : body["human_seats"]) human_seats.push_back(s.get<int>());

        const std::string id = new_game_id();
        {
            std::lock_guard<std::mutex> lock(_mu);
            _games[id] = std::make_unique<GameServer>(num_seats, stack, small_blind, big_blind, human_seats, max_hands);
        }
        res.set_content(json{{"game_id", id}}.dump(), "application/json");
    });

    _svr.Get(R"(/game/([^/]+)/state)", [this](const httplib::Request& req, httplib::Response& res) {
        const std::string id = req.matches[1];
        std::lock_guard<std::mutex> lock(_mu);
        auto it = _games.find(id);
        if (it == _games.end()) { res.status = 404; return; }
        res.set_content(state_json_or_done(*it->second).dump(), "application/json");
    });

    _svr.Post(R"(/game/([^/]+)/action)", [this](const httplib::Request& req, httplib::Response& res) {
        const std::string id = req.matches[1];
        json body;
        try { body = json::parse(req.body); }
        catch (...) { res.status = 400; return; }

        GameServer* gs = nullptr;
        {
            std::lock_guard<std::mutex> lock(_mu);
            auto it = _games.find(id);
            if (it == _games.end()) { res.status = 404; return; }
            gs = it->second.get();
        }

        const int seat   = body.value("seat", -1);
        const std::string act_str = body.value("action", "");
        const int amount = body.value("amount", 0);

        Action action;
        try {
            action = action_from_string(act_str);
        } catch (const std::invalid_argument&) {
            res.status = 400;
            res.set_content(R"({"error":"unknown action"})", "application/json");
            return;
        }

        try {
            gs->push_action(seat, {action, amount});
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", e.what()}}.dump(), "application/json");
            return;
        }

        res.set_content(state_json_or_done(*gs).dump(), "application/json");
    });
}
