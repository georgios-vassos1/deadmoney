#include "HumanPolicy.hpp"
#include "Table.hpp"

#include <gtest/gtest.h>
#include <sstream>

// HumanPolicy ignores the Table — the human reads board state from printed output.
// We pass an unstarted Table purely to satisfy the interface.
static Table dummy_table() {
    Table t(2, 5, 10);
    t.seat_player(0, "A", 1000);
    t.seat_player(1, "B", 1000);
    return t;
}

TEST(HumanPolicyTest, ParseFold) {
  std::istringstream in("fold\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::Fold);
}

TEST(HumanPolicyTest, ParseCheck) {
  std::istringstream in("check\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::Check);
}

TEST(HumanPolicyTest, ParseCall) {
  std::istringstream in("call\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::Call);
}

TEST(HumanPolicyTest, ParseRaiseWithAmount) {
  std::istringstream in("raise 120\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::Raise);
  EXPECT_EQ(action.amount, 120);
}

TEST(HumanPolicyTest, ParseAllin) {
  std::istringstream in("allin\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::AllIn);
}

TEST(HumanPolicyTest, ParseCaseInsensitive) {
  std::istringstream in("FOLD\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::Fold);
}

TEST(HumanPolicyTest, ParseInvalidInputFallsBackToFold) {
  std::istringstream in("garbage\n");
  HumanPolicy policy(in);
  const auto action = policy.act(0, dummy_table());
  EXPECT_EQ(action.action, Action::Fold);
}
