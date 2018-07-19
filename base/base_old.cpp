//
// Created by Pavel Rytir on 1/20/18.
//

#include "base_old.h"



#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {


  OldOutcome::OldOutcome(shared_ptr<State> s, vector<shared_ptr<Observation>> ob,
                   vector<double> rew) :
          state(move(s)), ob_(move(ob)), rew_(move(rew)) {}

  OutcomeDistributionOld::OutcomeDistributionOld(vector<pair<Outcome, double>> pairs) :
          distribution(move(pairs)) {}

  vector<Outcome> OutcomeDistributionOld::GetOutcomes() {
      vector<Outcome> list;
      for (auto &pair : distribution) {
          list.push_back(pair.first);
      }
      return list;
  }

  vector<double> OutcomeDistributionOld::GetProb() {
      vector<double> list;
      for (auto &pair : distribution) {
          list.push_back(pair.second);
      }
      return list;
  }

  OutcomeDistributionOld::OutcomeDistributionOld() {

  }

  OutcomeDistributionOld OldState::PerformAction(const vector<shared_ptr<Action>> &actions) {
      assert(false);
      return OutcomeDistributionOld(vector<pair<Outcome, double>>());
  }

  const vector<bool> &OldState::OldGetPlayers() const {
      assert(false);
      return vector<bool>();
  }

    double BestResponse(int player, const shared_ptr<vector<double>> &strategies,
                        int rows, int cols, const vector<double> &utilities) {
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
        for (int i = 0; i < cols; i++) {
            suma = 0;
            for (unsigned int j = 0; j < rows; j++) {
                suma += utilities[j * cols + i] * strategies->operator[](j);
            }
            if (suma > max) max = suma;
        }
        return max;
    }


    void OldTreewalkStart(const shared_ptr<Domain> &domain, unsigned int depth) {
        if (depth == 0)
            depth = domain->getMaxDepth();
        vector<Outcome> outcomes = domain->getRootStateDistributionPtr()->GetOutcomes();
        for (Outcome &o : outcomes) {
            OldTreewalk(domain, o.GetState().get(), depth, domain->getNumberOfPlayers());
        }
    }

    void OldTreewalk(shared_ptr<Domain> domain, State *state,
                     unsigned int depth, int players,
                     std::function<void(State *)> FunctionForState) {
        if (state == nullptr) {
            return;
        }

        FunctionForState(state);

        if (depth == 0)
            return;

        auto v = vector<vector<shared_ptr<Action>>>();
        for (int i = 0; i < players; ++i) {
            v.emplace_back(state->getAvailableActionsFor(i));
        }

        auto action = CartProduct(v);
        for (const auto &k : action) {
            OutcomeDistributionOld prob = state->PerformAction(k);
            vector<Outcome> outcomes = prob.GetOutcomes();
            for (Outcome &o : outcomes) {
                OldTreewalk(domain, o.GetState().get(), depth - 1, players, FunctionForState);
            }
        }
    }


    void OldTreewalk(shared_ptr<Domain> domain, State *state,
                     unsigned int depth, int players) {
        OldTreewalk(domain, state, depth, players, [](State *s) {});
    }


      double Domain::CalculateUtility(const vector<PureStrategy> &pure_strategies) {
        vector<Outcome> outcomes = rootStatesDistributionPtr->GetOutcomes();
        vector<double> p = rootStatesDistributionPtr->GetProb();
        double suma = 0;
        int z = 0;
        for (Outcome &o : outcomes) {
            if (!o.GetReward().empty())
                suma += o.GetReward()[0] * p[z];
            suma += ComputeUtility(o.GetState().get(), maxDepth, numberOfPlayers,
                                   pure_strategies, vector<vector<int>>(pure_strategies.size())) * p[z];
            ++z;
        }
        return suma;
    }

    double Domain::ComputeUtility(State *state, unsigned int depth,
                                  unsigned int players,
                                  const vector<PureStrategy> &pure_strategies,
                                  const vector<vector<int>> &aoh) {
        if (state == nullptr) {
            throw ("State is NULL");
        }

        if (depth == 0) {
            return 0;
        }

        auto search = vector<shared_ptr<Action>>(aoh.size());
        auto player = state->OldGetPlayers();
        for (int i = 0; i < aoh.size(); ++i) {
            if (player[i]) {
                auto ptr = make_shared<AOH>(i, aoh[i]);
                auto action = pure_strategies[i].Find(ptr);
                if (action != pure_strategies[i].End())
                    search[i] = action->second[0].second;
                else
                    throw ("No action found!");
            } else {
                search[i] = make_shared<Action>(NoA);
            }
        }
        OutcomeDistributionOld prob = state->PerformAction(search);
        vector<Outcome> outcomes = prob.GetOutcomes();
        vector<double> p = prob.GetProb();
        double suma = 0;
        int z = 0;
        for (Outcome &o : outcomes) {
            vector<vector<int>> locallist = aoh;
            for (int i = 0; i < search.size(); ++i) {
                locallist[i].push_back(search[i]->getId());
                locallist[i].push_back(o.GetObs()[i]->getId());
            }
            if (!o.GetReward().empty())
                suma += o.GetReward()[0] * p[z];
            suma += ComputeUtility(o.GetState().get(), depth - 1, players,
                                   pure_strategies, locallist) * p[z];
            ++z;
        }
        return suma;
    }
  vector<double> reward;


  void PursuitStart(const shared_ptr<Domain>& domain, unsigned int depth) {
      if (depth == 0)
          depth = domain->getMaxDepth();
      auto distr = domain->getRootStatesDistribution(); //domain->getRootStateDistributionPtr()->GetOutcomes();
      for (auto &o : distr) {
          for(int j = 0; j < reward.size(); ++j) {
              reward[j] += o.first.rewards[j];
          }
          Pursuit(domain, o.first.state.get(), depth, domain->getNumberOfPlayers());
      }
  }


  void Pursuit(const shared_ptr<Domain>& domain, State *state,
               unsigned int depth, int players) {
      if (state == nullptr) {
          return;
      }

      if (depth == 0) {
          cout <<"leaf\n";
          return;
      }
      auto v = vector<vector<shared_ptr<Action>>>();
      for (int i = 0; i < players; ++i) {
          v.emplace_back(state->getAvailableActionsFor(i));
      }
      auto action = CartProduct(v);
      for (const auto &k : action) {
          unordered_map<int, shared_ptr<Action>> mp;
          for(int x = 0; x < k.size(); ++x) {
              mp[x] = k[x];
          }
          OutcomeDistribution distr = state->performActions(mp);
          for (auto &o : distr) {
              for(int j = 0; j < reward.size(); ++j) {
                  reward[j] += o.first.rewards[j];
              }
              Pursuit(domain, o.first.state.get(), depth - 1, players);
          }
      }
  }
}
#pragma clang diagnostic pop