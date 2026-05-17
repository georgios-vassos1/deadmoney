#include "HttpPolicy.hpp"
#include <stdexcept>

HttpPolicy::HttpPolicy() : _future(_promise.get_future()) {}

BetAction HttpPolicy::act(int /*seat*/, const Table& table) {
    if (_shutdown) return {Action::Fold, 0};
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

void HttpPolicy::shutdown() {
    _shutdown   = true;
    _is_waiting = false;
    try { _promise.set_value({Action::Fold, 0}); }
    catch (const std::future_error&) {
        // Promise already satisfied — session is mid-rearm.
        // _shutdown flag ensures act() returns fold on next call.
    }
}

const Table* HttpPolicy::last_table_state() const {
    return _is_waiting ? &*_last_table : nullptr;
}
