//
// Created by rozliv on 07.08.2017.
//


#include <cassert>
#include "base.h"


Action::Action(int id): id_(id) {}

Observation::Observation(int id): id_(id) {}

Outcome::Outcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
                 vector<double> rew):
    st_(move(s)), ob_(move(ob)), rew_(move(rew)) {}

ProbDistribution::ProbDistribution(vector<pair<Outcome, double>> pairs):
    pairs_(move(pairs)) {}

vector<Outcome>  ProbDistribution::GetOutcomes() {
  vector<Outcome> list;
  for (auto &pair : pairs_) {
    list.push_back(pair.first);
  }
  return list;
}

vector<double> ProbDistribution::GetProb() {
  vector<double> list;
  for (auto &pair : pairs_) {
    list.push_back(pair.second);
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

double Domain::CalculateUtility(const vector<PureStrategy>& pure_strategies) {
  vector<Outcome> outcomes = root_->GetOutcomes();
  vector<double> p = root_->GetProb();
  double suma = 0;
  int z = 0;
  for (Outcome &o : outcomes) {
    if (!o.GetReward().empty())
      suma += o.GetReward()[0] * p[z];
    suma += ComputeUtility(o.GetState().get(), maxdepth_, maxplayers_,
                           pure_strategies, vector<vector<int>>(pure_strategies.size())) * p[z];
    ++z;
  }
  return suma;
}

double Domain::ComputeUtility(State* state, unsigned int depth,
                              unsigned int players,
                              const vector<PureStrategy>& pure_strategies,
                              const vector<vector<int>>& aoh) {
  if (state == nullptr) {
    throw("State is NULL");
  }

  if (depth == 0) {
    return 0;
  }

  auto search = vector<shared_ptr<Action>>(aoh.size());
  auto player = state->GetPlayers();
  for (int i = 0; i < aoh.size(); ++i) {
    if (player[i]) {
      auto ptr = make_shared<AOH>(i, aoh[i]);
      auto action = pure_strategies[i].Find(ptr);
      if (action != pure_strategies[i].End())
        search[i] = action->second[0].second;
      else
        throw("No action found!");
    } else {
      search[i] = make_shared<Action>(NoA);
    }
  }
  ProbDistribution prob = state->PerformAction(search);
  vector<Outcome> outcomes = prob.GetOutcomes();
  vector<double> p = prob.GetProb();
  double suma = 0;
  int z = 0;
  for (Outcome &o : outcomes) {
    vector<vector<int>> locallist = aoh;
    for (int i = 0; i < search.size(); ++i) {
      locallist[i].push_back(search[i]->GetID());
      locallist[i].push_back(o.GetObs()[i]->GetID());
    }
    if (!o.GetReward().empty())
      suma += o.GetReward()[0] *p[z];
    suma += ComputeUtility(o.GetState().get(), depth - 1, players,
                           pure_strategies, locallist) * p[z];
    ++z;
  }
  return suma;
}


double BestResponse(int player, const shared_ptr<vector<double>>& strategies,
                    int rows, int cols, const vector<double>& utilities) {
  assert(rows * cols == utilities.size());
  double suma;
  if (player == 1) {
    double min = INT32_MAX;
    for (int i = 0; i < rows; i++) {
      suma = 0;
      for (unsigned int j = 0; j < cols; j++) {
        suma += utilities[i * cols + j] * strategies->operator[](j);
      }
      if (suma < min) min = suma;
    }
    return min;
  }
  double max = INT32_MIN;
  for (int i=0; i < cols; i++) {
    suma = 0;
    for (unsigned int j = 0; j < rows; j++) {
      suma +=utilities[j*cols + i] * strategies->operator[](j);
    }
    if (suma > max) max = suma;
  }
  return max;
}


void TreewalkStart(const shared_ptr<Domain>& domain, unsigned int depth) {
  if (depth == 0)
    depth = domain->GetMaxDepth();
  vector<Outcome> outcomes = domain->GetRoot()->GetOutcomes();
  for (Outcome &o : outcomes) {
    Treewalk(domain, o.GetState().get(), depth, domain->GetMaxPlayers());
  }
}

void Treewalk(const shared_ptr<Domain> domain, State *state,
              unsigned int depth, int players,
              std::function<void(State*)> FunctionForState) {
  if (state == nullptr) {
    return;
  }

  FunctionForState(state);

  if (depth == 0)
    return;

  auto v = vector<vector<shared_ptr<Action>>>();
  for (int i = 0; i < players; ++i) {
    v.emplace_back(state->GetActions(i));
  }

  auto action = CartProduct(v);
  for (const auto &k : action) {
    ProbDistribution prob = state->PerformAction(k);
    vector<Outcome> outcomes = prob.GetOutcomes();
    for (Outcome &o : outcomes) {
      Treewalk(domain, o.GetState().get(), depth - 1, players, FunctionForState);
    }
  }
}


void Treewalk(const shared_ptr<Domain> domain, State *state,
              unsigned int depth, int players) {
  Treewalk(domain, state, depth, players, [](State* s){});
}
