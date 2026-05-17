#include "HoldemServer.hpp"

#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    const int port = (argc > 1) ? std::atoi(argv[1]) : 8080;

    HoldemServer server;
    server.start(port);

    std::cout << "holdem_server listening on port " << server.port() << "\n";
    std::cout << "POST /game/new   { num_seats, stack, human_seats, max_hands }\n";
    std::cout << "GET  /game/{id}/state\n";
    std::cout << "POST /game/{id}/action   { seat, action, amount }\n";

    // Block until Ctrl-C.
    while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
}
