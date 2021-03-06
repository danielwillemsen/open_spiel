// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "open_spiel/games/oware.h"

#include "open_spiel/tests/basic_tests.h"

namespace open_spiel {
namespace oware {
namespace {

namespace testing = open_spiel::testing;

void BasicOwareTests() {
  testing::LoadGameTest("oware");
  testing::RandomSimTest(*LoadGame("oware"), 10);
  testing::NoChanceOutcomesTest(*LoadGame("oware"));

  testing::RandomSimTest(
      *LoadGame("oware", {{"num_houses_per_player", GameParameter(2)},
                          {"num_seeds_per_house", GameParameter(2)}}),
      10);
}

void LegalActionsNoConstraintsTest() {
  OwareState state =
      OwareState(OwareBoard(0, {0, 0}, {1, 0, 9, 0, 0, 1, 1, 0, 0, 0, 0, 0}));
  SPIEL_CHECK_EQ(state.LegalActions(), std::vector<Action>({0, 2, 5}));
}

void LegalActionsLeaveTheOpponentSeedsTest() {
  OwareState state =
      OwareState(OwareBoard(0, {0, 0}, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}));
  // Playing action 0 would leave the opponent with no seeds, which is not
  // allowed.
  SPIEL_CHECK_EQ(state.LegalActions(), std::vector<Action>({5}));
}

void CaptureOpponentHousesTillBeginTest() {
  OwareState state(OwareBoard(0, {0, 0}, {0, 0, 8, 0, 0, 1, 1, 1, 1, 1, 2, 3}));
  state.ApplyAction(2);
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(1, {11, 0}, {0, 0, 0, 1, 1, 2, 0, 0, 0, 0, 0, 3}));
}

void CaptureMostOpponentHousesTest() {
  OwareState state(OwareBoard(0, {0, 0}, {0, 0, 8, 0, 0, 1, 3, 1, 1, 1, 2, 0}));
  state.ApplyAction(2);
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(1, {9, 0}, {0, 0, 0, 1, 1, 2, 4, 0, 0, 0, 0, 0}));
}

void NoCaptureBecauseTooFewSeedsTest() {
  OwareState state(OwareBoard(1, {0, 0}, {3, 1, 1, 1, 0, 0, 0, 0, 8, 0, 0, 1}));
  state.ApplyAction(2);
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(0, {0, 0}, {4, 2, 2, 2, 1, 0, 0, 0, 0, 1, 1, 2}));
}

void NoCaptureBecauseTooManySeedsTest() {
  OwareState state(OwareBoard(0, {0, 0}, {0, 0, 8, 0, 0, 1, 3, 1, 1, 1, 3, 0}));
  state.ApplyAction(2);
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(1, {0, 0}, {0, 0, 0, 1, 1, 2, 4, 2, 2, 2, 4, 0}));
}

void NoCaptureBecauseGrandSlamTest() {
  OwareState state(OwareBoard(1, {0, 0}, {1, 1, 1, 1, 1, 0, 0, 0, 8, 0, 0, 1}));
  state.ApplyAction(2);
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(0, {0, 0}, {2, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1, 2}));
}

void GameEndsByRepetitionTest() {
  OwareState state(
      OwareBoard(0, {23, 23}, {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}));

  state.ApplyAction(0);  // Player 0.
  state.ApplyAction(0);  // Player 1.
  state.ApplyAction(1);  // Player 0.
  state.ApplyAction(1);  // Player 1.
  state.ApplyAction(2);  // Player 0.
  state.ApplyAction(2);  // Player 1.
  state.ApplyAction(3);  // Player 0.
  state.ApplyAction(3);  // Player 1.
  state.ApplyAction(4);  // Player 0.
  state.ApplyAction(4);  // Player 1.

  SPIEL_CHECK_FALSE(state.IsTerminal());
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(0, {23, 23}, {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}));

  state.ApplyAction(5);  // Player 0.
  state.ApplyAction(5);  // Player 1.

  SPIEL_CHECK_TRUE(state.IsTerminal());
  SPIEL_CHECK_EQ(state.Board(),
                 OwareBoard(0, {24, 24}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
}

}  // namespace
}  // namespace oware
}  // namespace open_spiel

int main(int argc, char** argv) {
  open_spiel::oware::BasicOwareTests();
  open_spiel::oware::LegalActionsNoConstraintsTest();
  open_spiel::oware::LegalActionsLeaveTheOpponentSeedsTest();
  open_spiel::oware::GameEndsByRepetitionTest();
  open_spiel::oware::CaptureOpponentHousesTillBeginTest();
  open_spiel::oware::CaptureMostOpponentHousesTest();
  open_spiel::oware::NoCaptureBecauseTooFewSeedsTest();
  open_spiel::oware::NoCaptureBecauseTooManySeedsTest();
  open_spiel::oware::NoCaptureBecauseGrandSlamTest();
}
