//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

EFGNode::EFGNode(int player, unique_ptr<State> state, const vector<double>& rewards, const vector<InfSet>& aohistories):
    player_(player), state_(move(state)), rewards_(rewards), aohistories_(aohistories) {}

vector<shared_ptr<Action>> EFGNode::GetActions() {
  if(player_ == -1)
    return {};

  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

ChanceNode::ChanceNode(int player, unique_ptr<State> state, const vector<double>& rewards, const vector<InfSet> &aohistories) :
    EFGNode(player, move(state),rewards, aohistories) {}



ProbDistribution ChanceNode::GetDistribution() {
  vector<std::pair<Outcome, double>> pairs = vector<std::pair<Outcome, double>>();
  int count = 2;
  for (int k = 0; k < count; ++k) {
    vector<double> rew = vector<double>(state_->GetPlace().size());
    for (int i = 1; i < state_->GetPlace().size(); ++i) {
      if (/*(state_->GetPlace()[0].x == place_[i].x && state_->GetPlace()[0].y == place_[i].y &&
           state_->GetPlace()[i].x == place_[0].x &&
           state_->GetPlace()[i].y == place_[0].y) ||*/
          (state_->GetPlace()[0].x == state_->GetPlace()[i].x
           && state_->GetPlace()[0].y == state_->GetPlace()[i].y)) {
        ++rew[0];  // TODO: spatne - chybi protnuti cest -> i proto je potreba pamatovat si stary stav
        --rew[i];
      }
    }
    int index;
    int id;
    vector<unique_ptr<Observation>> obs = vector<unique_ptr<Observation>>();
    vector<int> ob = vector<int>();
    for (int m = 0; m < state_->GetPlace().size(); ++m) {
      id = 0;
      for (int i = 0; i < state_->GetPlace().size(); ++i) {
        if (m == i)
          continue;
        index = 0;
        int id2 = 1;
        for (int l = 1; l < state_->GetEight().size(); ++l) {
          if ((state_->GetPlace()[m].x + state_->GetEight()[l].x) >= 0 &&
              (state_->GetPlace()[m].x + state_->GetEight()[l].x)
                                                    < PursuitDomain::width_ &&
              (state_->GetPlace()[m].y + state_->GetEight()[l].y) >= 0 &&
              (state_->GetPlace()[m].y + state_->GetEight()[l].y)
                                                    < PursuitDomain::height_) {
            id2++;
          }

          if (state_->GetPlace()[m].x + state_->GetEight()[l].x == state_->GetPlace()[i].x &&
              state_->GetPlace()[m].y + state_->GetEight()[l].y == state_->GetPlace()[i].y) {
            index = l;
          }
        }
        ob.push_back(index);
        id += index * pow(id2, i);
      }
      obs.push_back(MakeUnique<PursuitObservation>(id, ob));
      ob.clear();
    }
    unique_ptr<PursuitState>s = MakeUnique<PursuitState>(state_->GetPlace(), state_->GetPro());
    double p2 = state_->GetPro();
    vector<Pos> loc = {{0, 0},
                       {PursuitDomain::height_-1, PursuitDomain::width_-1}};
    Outcome p(move(s), move(obs), rew);
    pairs.emplace_back(move(p), p2*k);
  }
  ProbDistribution prob(move(pairs));
  return prob;
}



void EFGTreewalk(const unique_ptr<Domain>& domain, const unique_ptr<EFGNode> &node,
                 int depth, int players) {
//  for(int i = 0; i < node->GetIS().aohistory_.size(); i++) {
//    cout << node->GetIS().aohistory_[i] << " ";
//  }
//  cout << '\n';
 // cout << depth << " ";
  count++;

  if (node == nullptr) {
    cout << "state is NULL\n";
    return;
  }
  if (depth == 0) {
 //   cout << '\n';
    return;
  }

  vector<shared_ptr<Action>> actions = node->GetActions();
 // cout << actions.size() << '\n';

  for (auto &i : actions) {
    unique_ptr<ChanceNode> cn = node->PerformAction<PursuitAction>(i);
    ProbDistribution prob = cn->GetDistribution();
    for (Outcome &o : prob.GetOutcomes()) {
      unique_ptr<State> st = o.GetState();
      vector<InfSet> inf = cn->GetAOH();
      inf[cn->GetPlayer()].aohistory_.push_back(o.GetObs()[cn->GetPlayer()]->GetID());
      vector<double> rews = cn->GetRewards();
      rews[0] += o.GetReward()[0];
      rews[1] += o.GetReward()[1];
      unique_ptr<EFGNode> n = MakeUnique<EFGNode>(1-cn->GetPlayer(), move(st),rews, inf);
      EFGTreewalk(domain, n, depth-1, players);
    }
  }
}
