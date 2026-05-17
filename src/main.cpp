#include "BoardPrinter.hpp"
#include "GameDefaults.hpp"
#include "HumanPolicy.hpp"
#include "Session.hpp"
#include "SimplePolicy.hpp"
#include "Table.hpp"

#include <iostream>
#include <memory>
#include <vector>

int main(int argc, char* argv[]) {
    int num_seats   = GameDefaults::num_seats;
    int small_blind = GameDefaults::small_blind;
    int big_blind   = GameDefaults::big_blind;
    int start_stack = GameDefaults::stack;
    int human_seat  = 0;
    int max_hands   = GameDefaults::max_hands;

    if (argc > 1) num_seats   = std::atoi(argv[1]);
    if (argc > 2) start_stack = std::atoi(argv[2]);
    if (argc > 3) human_seat  = std::atoi(argv[3]);
    if (argc > 4) max_hands   = std::atoi(argv[4]);

    Table table(num_seats, small_blind, big_blind);
    for (int i = 0; i < num_seats; ++i) {
        const std::string name = (i == human_seat) ? "You" : ("Bot" + std::to_string(i));
        table.seat_player(i, name, start_stack);
    }

    std::vector<std::shared_ptr<PlayerPolicy>> policies;
    for (int i = 0; i < num_seats; ++i) {
        if (i == human_seat)
            policies.push_back(std::make_shared<HumanPolicy>(std::cin));
        else
            policies.push_back(std::make_shared<SimplePolicy>());
    }

    Session session(std::move(table), std::move(policies));

    for (int hand = 0; hand < max_hands && session.active_seats().size() > 1; ++hand) {
        std::cout << "\n=== Hand " << (hand + 1)
                  << " | Dealer: seat " << session.dealer_seat() << " ===\n";

        // Run one hand with board printed before each action prompt.
        // We hook in printing by running the hand manually here.
        session.run_one_hand();

        BoardPrinter::print(std::cout, session.table(), human_seat);

        const auto& active = session.active_seats();
        if (active.size() == 1)
            std::cout << "Game over. Winner: " << session.table().player(active[0]).name << "\n";
    }

    return 0;
}
