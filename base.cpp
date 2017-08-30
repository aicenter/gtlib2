//
// Created by rozliv on 07.08.2017.
//

#include "base.h"



Action::Action(int id): id_(id) {}

Observation::Observation(int id): id_(id) {}

Outcome::Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
                 vector<double> rew):
    st_(move(s)), ob_(move(ob)), rew_(move(rew)) {}

ProbDistribution::ProbDistribution(vector<std::pair<Outcome, double>> pairs):
    pairs_(move(pairs)) {}

Outcome ProbDistribution::GetRandom() {
  int r = rand() % pairs_.size();
  return move(pairs_[r].first);
}

vector<Outcome>  ProbDistribution::GetOutcomes() {
  vector<Outcome> list;
  for (auto &pair : pairs_) {
    list.push_back(pair.first);
  }
  return list;
}

AOH::AOH(int player, const vector<int> &hist):
    player_(player), aohistory_(hist) {}

size_t AOH::GetHash() {
  if (seed_ == 0) {
    for (int i : aohistory_) {
      seed_ ^= i + 0x9e3779b9 + (seed_ << 6) + (seed_ >> 2);
    }
    seed_ ^= player_ + 0x9e3779b9 + (seed_ << 6) + (seed_ >> 2);
  }
  return seed_;
}

State::State() = default;

Domain::Domain(unsigned int max, unsigned int maxplayers):
    maxdepth_(max), maxplayers_(maxplayers) {}


int Domain::depth_ = 0;  // TODO(rozlijak)


void TreewalkStart(const unique_ptr<Domain>& domain, int depth) {
  if (depth == 0)
    depth = domain->GetMaxDepth();
  vector<Outcome> outcomes = domain->GetRoot()->GetOutcomes();
  for (Outcome &o : outcomes) {
    Treewalk(domain, o.GetState().get(), depth, domain->GetMaxPlayers());
  }
}

void Treewalk(const unique_ptr<Domain>& domain, State *state,
              int depth, int players) {
  if (state == nullptr) {
    throw("State is NULL");
  }
  if (depth == 0)
    return;

  auto v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->GetActions(i));
  }

  auto action = CartProduct<shared_ptr<Action>>(v);
  for (const auto &k : action) {
    ProbDistribution prob = state->PerformAction(k);
    vector<Outcome> outcomes = prob.GetOutcomes();
    for (Outcome &o : outcomes) {
      Treewalk(domain, o.GetState().get(), depth - 1, players);
    }
  }
}
