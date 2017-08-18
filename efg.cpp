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

unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
  vector<InfSet> aoh = aohistories_;
  aoh[player_].aohistory_.push_back(action2->GetID());
  unique_ptr<PursuitState> s = MakeUnique<PursuitState>(state_->GetPlace());
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(
      player_+1, move(s), rewards_, aoh);

  return node;
}


ChanceNode::ChanceNode(int player, unique_ptr<State> state,
                       const vector<double>& rewards,
                       const vector<InfSet> &aohistories) :
    EFGNode(player, move(state), rewards, aohistories) {}


void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players, const vector<shared_ptr<Action>>& list) {
  if (node == nullptr) {
    throw("Node is NULL");
  }
  if (depth == 0) {
    return;
  }
  vector<shared_ptr<Action>> actions = node->GetActions();
  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> l = list;
    l.push_back(i);
    if (players == node->GetState()->GetPlayers()) {
      ProbDistribution prob = n->GetState()->PerformAction(l);
      vector<Outcome> outcomes = prob.GetOutcomes();
      for (Outcome &o : outcomes) {
        vector<InfSet> inf = n->GetAOH();
        vector<unique_ptr<Observation>> obs = o.GetObs();
        for (int j = 0; j < inf.size(); ++j) {
          inf[j].aohistory_.push_back(obs[j]->GetID());
        }
        vector<double> rews = vector<double>({n->GetRewards()[0] + o.GetReward()[0],
                                              n->GetRewards()[1] + o.GetReward()[1]});
        unique_ptr<EFGNode> cn = MakeUnique<EFGNode>(0, move(o.GetState()), rews, inf);
        count++;
        for (int k = 0; k < reward.size(); ++k) {
          reward[k] += o.GetReward()[k];
        }
        EFGTreewalk(domain, cn.get(), depth - 1, 1, {});
      }
    } else {
      EFGTreewalk(domain, n.get(), depth, players + 1, l);
    }
  }
}
