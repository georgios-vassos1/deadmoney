#include "BoardPrinter.hpp"
#include "Card.hpp"

void BoardPrinter::print(std::ostream& out, const Table& table, int human_seat) {
    // Community cards.
    out << "Board: [ ";
    for (const auto& c : table.community())
        out << to_string(c) << " ";
    out << "]\n";

    // Pot.
    out << "Pot:   " << table.pot().total() << "\n\n";

    // Players.
    for (int i = 0; i < table.num_seats(); ++i) {
        const auto& p = table.player(i);
        out << p.name << " ($" << p.stack << ")  ";
        if (i == human_seat) {
            out << to_string(p.hole_cards[0]) << " " << to_string(p.hole_cards[1]);
        } else {
            out << "?? ??";
        }
        out << "\n";
    }
    out << "\n";
}
