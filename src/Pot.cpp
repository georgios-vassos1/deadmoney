#include "Pot.hpp"
#include <algorithm>

void Pot::contribute(int seat, int amount) {
    _contributions.push_back({seat, amount});
}

int Pot::total() const {
    int sum = 0;
    for (const auto& c : _contributions) sum += c.amount;
    return sum;
}

void Pot::reset() {
    _contributions.clear();
    _side_pots.clear();
}

void Pot::resolve() {
    // Aggregate contributions per seat.
    std::vector<std::pair<int,int>> per_seat; // {seat, total_contributed}
    for (const auto& c : _contributions) {
        bool found = false;
        for (auto& ps : per_seat) {
            if (ps.first == c.seat) { ps.second += c.amount; found = true; break; }
        }
        if (!found) per_seat.push_back({c.seat, c.amount});
    }

    // Collect unique contribution levels, sorted ascending.
    std::vector<int> levels;
    for (const auto& ps : per_seat) levels.push_back(ps.second);
    std::sort(levels.begin(), levels.end());
    levels.erase(std::unique(levels.begin(), levels.end()), levels.end());

    // For each level, build a side pot from the marginal chips at that level.
    _side_pots.clear();
    int prev = 0;
    for (const int level : levels) {
        SidePot sp;
        sp.amount = 0;
        for (const auto& ps : per_seat) {
            // Each seat contributes min(their_total, level) - prev to this pot.
            const int contributed = std::min(ps.second, level) - prev;
            if (contributed > 0) sp.amount += contributed;
            // Eligible if they put in the full level.
            if (ps.second >= level) sp.eligible_seats.push_back(ps.first);
        }
        _side_pots.push_back(sp);
        prev = level;
    }
}
