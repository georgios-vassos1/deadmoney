#include "HttpPolicy.hpp"

HttpPolicy::HttpPolicy() : _future(_promise.get_future()) {}

BetAction HttpPolicy::act(int /*seat*/, const Table& table) {
    _last_table = table;
    BetAction action = _future.get();
    _promise = std::promise<BetAction>{};
    _future  = _promise.get_future();
    return action;
}

void HttpPolicy::push_action(BetAction action) {
    _promise.set_value(action);
}

const Table* HttpPolicy::last_table_state() const {
    return _last_table ? &*_last_table : nullptr;
}
