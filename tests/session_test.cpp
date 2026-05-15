#include "Session.hpp"
#include "Table.hpp"

#include <gtest/gtest.h>
#include <memory>

// Stub policy: check when no bet is outstanding, otherwise call.
class CallPolicy : public PlayerPolicy {
public:
    BetAction act(int seat, const Table& table) override {
        if (table.current_round().is_action_valid(seat, {Action::Check, 0}))
            return {Action::Check, 0};
        return {Action::Call, 0};
    }
};

// Stub policy: always fold.
class FoldPolicy : public PlayerPolicy {
public:
    BetAction act(int /*seat*/, const Table& /*table*/) override {
        return {Action::Fold, 0};
    }
};

static Session make_session(int stacks = 10000) {
    Table table(2, 5, 10);
    table.seat_player(0, "Alice", stacks);
    table.seat_player(1, "Bob",   stacks);
    const auto policy = std::make_shared<CallPolicy>();
    return Session(std::move(table), {policy, policy});
}

TEST(SessionTest, DealerButtonRotates) {
  auto session = make_session();

  EXPECT_EQ(session.dealer_seat(), 0);
  session.run_one_hand();
  EXPECT_EQ(session.dealer_seat(), 1);
  session.run_one_hand();
  EXPECT_EQ(session.dealer_seat(), 0);
}

TEST(SessionTest, ChipsConservedAcrossHands) {
  auto session = make_session();
  session.run(5);

  const int total = session.table().player(0).stack
                  + session.table().player(1).stack;
  EXPECT_EQ(total, 20000);
}

TEST(SessionTest, StopsAtMaxHands) {
  auto session = make_session();
  session.run(5);
  EXPECT_EQ(session.hands_played(), 5);
}

TEST(SessionTest, BustedPlayerEliminated) {
  // Bob always folds — he bleeds blinds each hand and eventually busts.
  Table table(2, 5, 10);
  table.seat_player(0, "Alice", 1000);
  table.seat_player(1, "Bob",    50); // Bob can survive ~5 hands as BB (pays 10 each)

  Session session(std::move(table), {
      std::make_shared<CallPolicy>(),
      std::make_shared<FoldPolicy>()
  });

  session.run(20); // run enough hands for Bob to bust

  EXPECT_EQ(session.active_seats().size(), 1u);
  EXPECT_EQ(session.active_seats()[0], 0); // Alice survives
}
