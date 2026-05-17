# Dead Money

A Texas Hold'em engine written in C++17, developed strictly by TDD. Every production line is covered by a failing test written before the code existed.

## Features

- Full hand lifecycle: blinds, preflop through river, side pots, showdown, tie splitting
- 7-card hand evaluator with kicker comparison across all hand categories
- Multi-hand session with dealer rotation and bust-out elimination
- Human vs. bot CLI — plug in any `PlayerPolicy` subclass as an AI opponent
- REST API server for building frontends or bots over HTTP
- 87 unit tests, all passing

## Requirements

- CMake ≥ 3.14
- C++17 compiler (GCC 9+ or Clang 10+)
- Internet access on first build (GoogleTest fetched via `FetchContent`)

Third-party headers (bundled in `include/`, no install needed):
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) — header-only HTTP server/client
- [nlohmann/json](https://github.com/nlohmann/json) — header-only JSON

## Build

```bash
git clone <repo-url>
cd myHoldem
cmake -S . -B build
cmake --build build --parallel
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Play (CLI)

```bash
./build/holdem_cli [num_seats] [start_stack] [human_seat] [max_hands]
```

| Argument | Default | Description |
|---|---|---|
| `num_seats` | `2` | Total seats at the table |
| `start_stack` | `1000` | Chips each player starts with |
| `human_seat` | `0` | Which seat you occupy |
| `max_hands` | `100` | Hands before the session ends |

Blinds are fixed at SB=5 / BB=10.

**Examples:**

```bash
./build/holdem_cli                  # heads-up vs 1 bot
./build/holdem_cli 3 500 0          # 3-handed, 500 chips, you at seat 0
./build/holdem_cli 4 2000 1 50      # 4-handed, 2000 chips, you at seat 1, 50 hands
```

**Actions** (type when prompted):

| Input | Meaning |
|---|---|
| `fold` | Fold your hand |
| `check` | Check (only valid when no bet is outstanding) |
| `call` | Call the current bet |
| `raise <amount>` | Raise to `amount` (e.g. `raise 30`) |
| `allin` | Put all remaining chips in |

Invalid actions are re-prompted. The session ends when only one player has chips or `max_hands` is reached.

## REST API Server

```bash
./build/holdem_server [port]    # default: 8080
```

### Endpoints

**POST /game/new** — Create a new game.

```json
{
  "num_seats": 2,
  "stack": 1000,
  "small_blind": 5,
  "big_blind": 10,
  "human_seats": [0],
  "max_hands": 10
}
```

All fields are optional and fall back to `GameDefaults`. Returns `{"game_id": "..."}`.

**GET /game/{id}/state** — Poll current game state.

```json
{
  "num_seats": 2,
  "pot": 15,
  "dealer": 0,
  "players": [
    {"seat": 0, "stack": 990, "street_bet": 10, "status": "active"},
    {"seat": 1, "stack": 995, "street_bet": 5, "status": "active"}
  ],
  "hands_played": 0,
  "done": false
}
```

**POST /game/{id}/action** — Submit an action for a human seat.

```json
{"seat": 0, "action": "raise", "amount": 30}
```

Returns the updated game state. Invalid actions return `400` with `{"error": "..."}`.

### Error codes

| Code | Cause |
|---|---|
| 400 | Malformed JSON, unknown action, invalid raise, bot seat, out-of-range seat |
| 404 | Unknown game ID |

## Project structure

```
include/      Public headers (one per component)
src/          Implementations, CLI entry (main.cpp), server entry (server_main.cpp)
tests/        GTest unit tests (one file per component)
CMakeLists.txt
```

## Architecture

| Component | Role |
|---|---|
| `Card` / `Deck` | 52-card deck with preset constructor for deterministic tests |
| `HandEvaluator` | Best 5-of-7 evaluation; `HandRank` supports `<` / `==` for direct comparison |
| `Player` | Stack, street bet, status (Active / Folded / AllIn); `post()` auto-caps at stack |
| `Pot` | Accumulates contributions; `resolve()` builds side pots by all-in level |
| `BettingRound` | Validates and applies all actions; tracks current bet, min-raise, action-reopen |
| `Table` | Orchestrates a single hand end-to-end |
| `Session` | Multi-hand loop: rotates dealer, runs streets through policies, eliminates busted players |
| `PlayerPolicy` | Abstract interface (`act(seat, table) → BetAction`); implement to add any AI |
| `HumanPolicy` | Reads from any `std::istream`; used with `std::cin` in the CLI and `istringstream` in tests |
| `SimplePolicy` | Stub AI: checks when free, calls otherwise |
| `BoardPrinter` | Prints board state to any `std::ostream`; hides opponent hole cards |
| `HttpPolicy` | `PlayerPolicy` that blocks on a `std::promise`; HTTP handler fulfills it via `push_action()` |
| `GameServer` | Owns a `Session` on a background thread; validates and routes actions to `HttpPolicy` instances |
| `HoldemServer` | HTTP server (cpp-httplib) managing multiple concurrent `GameServer` instances |
| `GameDefaults` | Centralized default values for seats, stack, blinds, max hands, server port |

## Extending with a custom AI

Subclass `PlayerPolicy` and pass it to `Session`:

```cpp
#include "PlayerPolicy.hpp"
#include "Table.hpp"

class MyPolicy : public PlayerPolicy {
public:
    BetAction act(int seat, const Table& table) override {
        // inspect table.current_round(), table.player(seat), table.community(), ...
        return {Action::Call, 0};
    }
};
```

Then wire it in alongside `HumanPolicy` or instead of `SimplePolicy`.

## Known limitations

- Board is printed after each hand, not before each action prompt
- No "your turn" prompt printed before reading stdin
- Burn cards are not dealt (standard rules burn one before flop/turn/river)
- Preflop UTG position is not adjusted for 3+ players (always left of dealer)
- No rebuy or sit-out support
- API does not expose whose turn it is (client must try actions and handle 400)

## License

MIT
