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

#include "open_spiel/policy.h"

#include <list>
#include <memory>

#include "open_spiel/spiel_utils.h"

namespace open_spiel {

double GetProb(const ActionsAndProbs& action_and_probs, Action action) {
  for (const auto& action_and_prob : action_and_probs) {
    if (action_and_prob.first == action) {
      return action_and_prob.second;
    }
  }
  return -1.0;
}

TabularPolicy::TabularPolicy(const Game& game)
    : TabularPolicy(GetRandomPolicy(game)) {}

TabularPolicy GetEmptyTabularPolicy(const Game& game) {
  std::unordered_map<std::string, ActionsAndProbs> policy;
  if (game.GetType().dynamics != GameType::Dynamics::kSequential) {
    SpielFatalError("Game is not sequential.");
    return TabularPolicy(policy);
  }
  std::list<std::unique_ptr<State>> to_visit;
  to_visit.push_back(game.NewInitialState());
  while (!to_visit.empty()) {
    std::unique_ptr<State> state = std::move(to_visit.back());
    to_visit.pop_back();
    if (state->IsTerminal()) {
      continue;
    }
    if (state->IsChanceNode()) {
      for (const auto& outcome_and_prob : state->ChanceOutcomes()) {
        to_visit.emplace_back(state->Child(outcome_and_prob.first));
      }
    } else {
      ActionsAndProbs infostate_policy;
      infostate_policy.reserve(game.NumDistinctActions());
      Action action_index = 0;
      for (int is_legal : state->LegalActionsMask()) {
        if (is_legal == 1) to_visit.push_back(state->Child(action_index));
        infostate_policy.push_back({action_index, is_legal});
        ++action_index;
      }
      if (infostate_policy.empty()) {
        SpielFatalError("State has zero legal actions.");
      }
      policy.insert({state->InformationState(), infostate_policy});
    }
  }
  return TabularPolicy(policy);
}

TabularPolicy GetUniformPolicy(const Game& game) {
  TabularPolicy policy = GetEmptyTabularPolicy(game);
  std::unordered_map<std::string, ActionsAndProbs>& policy_table =
      policy.PolicyTable();
  for (const auto& kv : policy_table) {
    ActionsAndProbs state_policy;
    if (kv.second.empty()) {
      SpielFatalError("State has zero legal actions.");
    }
    state_policy.reserve(kv.second.size());
    double num_legal_actions = 0.;
    for (const auto& a_and_p : kv.second) {
      // GetEmptyTabularPolicy assigns a probability of 1 to all legal actions
      // and a probability of 0 to all illegal actions.
      num_legal_actions += a_and_p.second;
    }
    SPIEL_CHECK_GT(num_legal_actions, 0.);
    double prob = 1. / num_legal_actions;

    // action_and_prob.second is equal to 1 if the action is legal, 0 otherwise.
    for (const auto& action_and_prob : kv.second) {
      state_policy.push_back(
          {action_and_prob.first, prob * action_and_prob.second});
    }
    policy_table[kv.first] = std::move(state_policy);
  }
  return policy;
}

TabularPolicy GetRandomPolicy(const Game& game, int seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution<double> dist(0, 1);
  TabularPolicy policy = GetEmptyTabularPolicy(game);
  std::unordered_map<std::string, ActionsAndProbs>& policy_table =
      policy.PolicyTable();
  for (auto& kv : policy_table) {
    ActionsAndProbs state_policy;
    if (kv.second.empty()) {
      SpielFatalError("State has zero legal actions.");
    }
    state_policy.reserve(kv.second.size());
    double sum = 0;
    double prob;
    for (const auto& action_and_prob : kv.second) {
      // We multiply the original probability by a random number between 0
      // and 1. We then normalize. This has the effect of randomly permuting the
      // policy but all illegal actions still have zero probability.
      prob = dist(gen) * action_and_prob.second;
      sum += prob;
      state_policy.push_back({action_and_prob.first, prob});
    }
    // We normalize the policy to ensure it sums to 1.
    for (auto& action_and_prob : state_policy) {
      action_and_prob.second /= sum;
    }
    // This is included as a sanity check.
    double normalized_sum = 0;
    for (auto& action_and_prob : state_policy) {
      normalized_sum += action_and_prob.second;
    }
    SPIEL_CHECK_FLOAT_EQ(normalized_sum, 1.0);
    kv.second = state_policy;
  }
  return policy;
}

TabularPolicy GetFirstActionPolicy(const Game& game) {
  TabularPolicy policy = GetEmptyTabularPolicy(game);
  std::unordered_map<std::string, ActionsAndProbs>& policy_table =
      policy.PolicyTable();
  for (auto& kv : policy_table) {
    ActionsAndProbs state_policy;
    bool first_legal_action_found = false;
    for (int i = 0; i < kv.second.size(); ++i) {
      Action action = kv.second[i].first;
      // We set the first legal action to have unit probability, setting
      // everything else to 0.
      if (Near(kv.second[i].second, 1.0) && !first_legal_action_found) {
        first_legal_action_found = true;
        state_policy.push_back({action, 1.});
      } else {
        state_policy.push_back({action, 0.});
      }
    }
    if (state_policy.empty()) {
      SpielFatalError("State has zero legal actions.");
    }
    kv.second = state_policy;
  }
  return policy;
}

}  // namespace open_spiel
