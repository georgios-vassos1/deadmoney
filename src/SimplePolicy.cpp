#include "SimplePolicy.hpp"
#include "Table.hpp"

BetAction SimplePolicy::act(int seat, const Table& table) {
    if (table.current_round().is_action_valid(seat, {Action::Check, 0}))
        return {Action::Check, 0};
    return {Action::Call, 0};
}
