#pragma once

#include <vector>

struct SidePot {
    int              amount;
    std::vector<int> eligible_seats;
};

class Pot {
public:
    void contribute(int seat, int amount);
    void resolve();
    void reset();
    int  total() const;

    const std::vector<SidePot>& side_pots() const { return _side_pots; }

private:
    struct Contribution { int seat; int amount; };
    std::vector<Contribution> _contributions;
    std::vector<SidePot>      _side_pots;
};
