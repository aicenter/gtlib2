//
// Created by rozliv on 07.08.2017.
//

#include "base.h"


Action::Action(int id): id_(id) {}

string Action::ToString() {
  return to_string(id_);
}

Observation::Observation(int id): id_(id) {}

string Observation::ToString() {
  return to_string(id_);
}

State::State() = default;

Domain::Domain(int maxplayers, int max):
    maxplayers_(maxplayers), maxdepth_(max) {}

string Domain::GetInfo() {}

Outcome::Outcome(unique_ptr<State> s, vector<unique_ptr<Observation>> ob,
                 const vector<double> &rew):
    st_(move(s)), ob_(move(ob)), rew_(rew) {}

ProbDistribution::ProbDistribution(vector<std::pair<Outcome, double>> pairs):
    pairs_(move(pairs)) {}

Outcome ProbDistribution::GetRandom() {
  int r = rand() % pairs_.size();
  return move(pairs_[r].first);
}

vector<Outcome>  ProbDistribution::GetOutcomes() {
  vector<Outcome> list;
  for (auto &pair : pairs_) {
    list.push_back(move(pair.first));
  }
  return list;
}

int Domain::depth_ = 0;  // TODO(rozlijak)



void Treewalk(const unique_ptr<Domain>& domain, State *state,
              int depth, int players) {
  if (state == nullptr) {
    throw("State is NULL");
  }
  if (depth == 0)
    return;

  vector<vector<shared_ptr<Action>>> v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->GetActions(i));
  }

  vector<vector<shared_ptr<Action>>> action = CartProduct<Action>(v);
  for (const auto &k : action) {
    ProbDistribution prob = state->PerformAction(k);
    vector<Outcome> outcomes = prob.GetOutcomes();
    cout << outcomes.size() << "\n";
    for (Outcome &o : outcomes) {
      Treewalk(domain, o.GetState().get(), depth - 1, players);
    }
  }
}
