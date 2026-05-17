#include "HttpPolicy.hpp"

HttpPolicy::HttpPolicy() : _future(_promise.get_future()) {}

BetAction HttpPolicy::act(int /*seat*/, const Table& table) {
    _last_table = table;
    _is_waiting = true;
    BetAction action = _future.get();
    _promise = std::promise<BetAction>{};
    _future  = _promise.get_future();
    return action;
}

void HttpPolicy::push_action(BetAction action) {
    _is_waiting = false;
    _promise.set_value(action);
}

const Table* HttpPolicy::last_table_state() const {
    return _is_waiting ? &*_last_table : nullptr;
}
