//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

EFGNode::EFGNode(int player, unique_ptr<State> state,
                 const vector<double>& rewards,
                 const vector<InfSet>& aohistories):
    player_(player), state_(move(state)), rewards_(rewards),
    aohistories_(aohistories) {}

vector<shared_ptr<Action>> EFGNode::GetActions() {
  if (player_ == -1)
    return {};

  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

ChanceNode::ChanceNode(int player, unique_ptr<State> state,
                       const vector<double>& rewards,
                       const vector<InfSet> &aohistories) :
    EFGNode(player, move(state), rewards, aohistories) {}



ProbDistribution ChanceNode::GetDistribution(const vector<Pos>& loc) {
  vector<std::pair<Outcome, double>> pairs = vector<std::pair<Outcome, double>>();
  unique_ptr<PursuitState> s = MakeUnique<PursuitState>(state_->GetPlace());
  int count = 2;
  for (int k = 0; k < count; ++k) {
    vector<double> rew = vector<double>(s->GetPlace().size());
    for (int i = 1; i < s->GetPlace().size(); ++i) {
      if (s->GetPlace()[0].x == s->GetPlace()[i].x
           && s->GetPlace()[0].y == s->GetPlace()[i].y) {
        ++rew[0];  // TODO: spatne - chybi protnuti cest
        --rew[i];
      }
    }
    int index;
    int id;
    vector<unique_ptr<Observation>> obs = vector<unique_ptr<Observation>>();
    vector<int> ob = vector<int>();
    for (int m = 0; m < s->GetPlace().size(); ++m) {
      id = 0;
      for (int i = 0, p = 0; i < s->GetPlace().size(); ++i, ++p) {
        if (m == i) {
          --p;
          continue;
        }
        index = 0;
        int id2 = 1;
        for (int l = 1; l < s->GetEight().size(); ++l) {
          if ((s->GetPlace()[m].x + s->GetEight()[l].x) >= 0 &&
              (s->GetPlace()[m].x + s->GetEight()[l].x)
                                                    < PursuitDomain::width_ &&
              (s->GetPlace()[m].y + s->GetEight()[l].y) >= 0 &&
              (s->GetPlace()[m].y + s->GetEight()[l].y)
                                                    < PursuitDomain::height_) {
            id2++;
          }

          if (s->GetPlace()[m].x + s->GetEight()[l].x == s->GetPlace()[i].x &&
              s->GetPlace()[m].y + s->GetEight()[l].y == s->GetPlace()[i].y) {
            index = l;
          }
        }
        ob.push_back(index);
        id += index * pow(id2, p);
      }
      obs.push_back(MakeUnique<PursuitObservation>(id, ob));
      ob.clear();
    }
    double p2 = s->GetPro();
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2*k);
    s = MakeUnique<PursuitState>(loc);
  }
  ProbDistribution prob(move(pairs));
  return prob;
}



void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players) {
  count++;
  if (node == nullptr) {
    std::cerr << "state is NULL\n";
    return;
  }
  if (depth == 0) {
    return;
  }

  vector<shared_ptr<Action>> actions = node->GetActions();

  for (auto &i : actions) {
    unique_ptr<ChanceNode> cn = node->PerformAction<PursuitAction>(i);
    ProbDistribution prob = cn->GetDistribution(node->GetState()->GetPlace());
    for (Outcome &o : prob.GetOutcomes()) {
      unique_ptr<State> st = o.GetState();
      vector<InfSet> inf = cn->GetAOH();
      inf[cn->GetPlayer()].aohistory_.push_back(
          o.GetObs()[cn->GetPlayer()]->GetID());
      auto rews = vector<double>({cn->GetRewards()[0] + o.GetReward()[0],
                                  cn->GetRewards()[1] + o.GetReward()[1]});
      unique_ptr<EFGNode> n =
          MakeUnique<EFGNode>(1-cn->GetPlayer(), move(st), rews, inf);
      EFGTreewalk(domain, n.get(), depth-1, players);
    }
  }
}
