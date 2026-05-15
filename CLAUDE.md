# myHoldem — Development Rules

This project is developed under three skills: **TDD**, **unit-tests**, and **refine**.
The rules below are their combined non-negotiable core. Deviation is not "pragmatic" — it is a defect.

---

## TDD — The Iron Law

```
NO PRODUCTION CODE WITHOUT A FAILING TEST FIRST.
Write code before the test? Delete it. Start over.
```

### The cycle — no shortcuts, no exceptions

1. **RED** — Write one test for one scenario. It must fail to *compile or run* before any implementation exists.
2. **Verify RED** — Run the build. Confirm the failure is for the right reason (missing feature, not a typo). If the test passes immediately, you are testing existing behavior — fix the test.
3. **GREEN** — Write the *minimum* code that makes the test pass. No extra features, no "while I'm here" additions.
4. **Verify GREEN** — Run the build and all tests. New test passes; no regressions.
5. **REFACTOR** — Clean up duplication, names, structure. Tests stay green. No new behavior.
6. **Repeat** — Next scenario, next failing test.

### Build and test commands

```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Red flags — stop and start over if any apply

- Wrote implementation before the test
- Test passed on the first run without any implementation
- Cannot explain why the test failed
- Added "just a bit more" beyond what the failing test required
- "I'll add the test after" — No. Delete the code.

---

## unit-tests — Test Quality Rules

### Framework and file structure

- **GTest only**: `TEST()`, `TEST_F()`, `TEST_P()`, `TYPED_TEST()`
- Test files live in `tests/`, one file per component
- New test executables get their own `add_holdem_test(name)` entry in `CMakeLists.txt`
- Header under test is included **first**, before `<gtest/gtest.h>`

### Naming

- PascalCase test names, **no underscores** — GTest treats them specially
  - Good: `TEST(DeckTest, DealReducesSize)`
  - Bad: `TEST(DeckTest, Deal_Reduces_Size)`
- Name describes the *behaviour*, not the method
  - Good: `StraightFlushBeatsFullHouse`
  - Bad: `Test1`, `ItWorks`, `HandEvaluatorTest`

### One test, one scenario

A scenario can have multiple checkpoints (ASSERT then EXPECT) but tests one *logical path*.
If "and" appears naturally in the test name, split it.

```cpp
// GOOD — one scenario, multiple checkpoints
TEST(BettingRoundTest, RaiseReopensActionForOtherPlayers) {
  BettingRound round(3, 10);
  // ... setup ...
  round.apply(0, {Action::Raise, 30}, stack, bet);
  EXPECT_EQ(round.current_bet(), 30);
  EXPECT_TRUE(round.is_action_valid(1, {Action::Call, 0}));
  EXPECT_TRUE(round.is_action_valid(2, {Action::Fold, 0}));
}
```

### Test data

- **`const auto`** for all locals
- Randomise every value the assertion does not depend on
- Only hardcode what is load-bearing for the assertion

```cpp
// GOOD — only the rank relationship matters, not the suits
const auto suit = random_suit();
EXPECT_GT(static_cast<int>(Rank::Ace), static_cast<int>(Rank::Two));

// BAD — hardcoded values when the test doesn't care about them
const Card a{Rank::Ace, Suit::Spades};
```

### Assertions

- **ASSERT** when the rest of the test is meaningless if this fails (e.g., `ASSERT_FALSE(deck.empty())` before calling `deal()`)
- **EXPECT** for the actual checks — lets other failures surface
- Add `<< context` inside loops so failures identify which iteration broke

### What NOT to test

- Compiler-generated operators (`= default`)
- Default member initializers (language guarantee)
- That `std::vector` stores what you pushed
- Determinism of pure functions (same input → same output is a language guarantee)
- Trivial getters — they will be exercised by tests of real behaviour

### Depth over breadth

Test the *hardest* behaviour thoroughly. Simple methods that can only break in obvious ways need at most one test. For `HandEvaluator`, this means 15+ tests on hand classification and kicker comparison — not 1 test on each of 15 methods.

### No anti-patterns

| Anti-pattern | Fix |
|---|---|
| Mirrored assertion (expected computed with same formula as code) | Hand-calculate expected from known inputs |
| Copy-paste tests differing only in one value | Use `TEST_P` parameterised tests |
| Test-only method added to production class | Move to test utilities |
| Magic numbers in assertions | Named constant or inline comment explaining the derivation |
| Section separator comments (`// --- Section ---`) | Descriptive test names instead |

---

## refine — Changing Existing Code

When a working function needs redesign (not a bug fix), follow **ANALYZE → PROPOSE → IMPLEMENT**.

1. **ANALYZE** — Read the target code. Search the codebase for similar patterns (at least 3 angles). Identify misalignments. Present findings — do not propose solutions yet.
2. **PROPOSE** — Present exactly 3 alternatives, each grounded in a pattern that already exists in this codebase (cite `file:line`). Rank them with a recommendation. Wait for the user to pick one.
3. **IMPLEMENT** — Apply exactly the chosen option. Update callers if the signature changed. Build and run all tests.

### Hard rules for refine

- Every proposal needs a `file:line` reference to where that pattern already exists
- Never change observable behaviour during a refine
- Never touch code outside the target function unless callers need updating
- Never skip the proposal step even if one option is obviously best
- Always present complete, compilable code in proposals — not pseudocode

---

## Commit discipline

- One commit per TDD cycle (or per logical group of related cycles)
- Message format: `feat(component): short description`
- Never commit failing tests or a red build
