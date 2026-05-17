#pragma once

#include "PlayerPolicy.hpp"
#include "Table.hpp"

#include <atomic>
#include <future>
#include <optional>

class HttpPolicy : public PlayerPolicy {
public:
    HttpPolicy();

    BetAction     act(int seat, const Table& table) override;
    void          push_action(BetAction action);
    const Table*  last_table_state() const;

private:
    std::promise<BetAction> _promise;
    std::future<BetAction>  _future;
    std::optional<Table>    _last_table;
    std::atomic<bool>       _is_waiting{false};
};
