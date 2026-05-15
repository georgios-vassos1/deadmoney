#pragma once

#include "Table.hpp"

#include <ostream>

class BoardPrinter {
public:
    // Prints the current table state to out.
    // hole cards are shown for human_seat; all other seats show "?? ??".
    static void print(std::ostream& out, const Table& table, int human_seat);
};
