//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

EFGNode::EFGNode(int player, const shared_ptr<State>& state,
                 const vector<double>& rewards,
                 const vector<InfSet>& aohistories):
    player_(player), state_(state), rewards_(rewards),
    aohistories_(aohistories) {}

vector<shared_ptr<Action>> EFGNode::GetAction() {
  if (player_ == -1)
    return {};
  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
  vector<InfSet> aoh = aohistories_;
  aoh[player_].PushToHistory(action2->GetID());
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(
      player_+1, state_, rewards_, aoh);

  return node;
}


ChanceNode::ChanceNode(int player, const shared_ptr<State>& state,
                       const vector<double>& rewards,
                       const vector<InfSet> &aohistories) :
    EFGNode(player, state, rewards, aohistories) {}


void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players, const vector<shared_ptr<Action>>& list) {
  if (node == nullptr) {
    throw("Node is NULL");
  }
  if (depth == 0) {
    return;
  }
  vector<shared_ptr<Action>> actions = node->GetAction();
  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> locallist = list;
    locallist.push_back(i);
    if (players == node->GetState()->GetPlayers()) {
      // if all players play in this turn, returns a ProbDistribution
      ProbDistribution prob = n->GetState()->PerformAction(locallist);
      vector<Outcome> outcomes = prob.GetOutcomes();
      for (Outcome &o : outcomes) {
        unique_ptr<State> st = o.GetState();
        vector<InfSet> aoh = n->GetAOH();
        vector<unique_ptr<Observation>> obs = o.GetObs();
        for (int j = 0; j < aoh.size(); ++j) {
          aoh[j].PushToHistory(obs[j]->GetID());
        }
        vector<double> rews = vector<double>({n->GetRewards()[0] + o.GetReward()[0],
                                              n->GetRewards()[1] + o.GetReward()[1]});
        unique_ptr<EFGNode> cn = MakeUnique<EFGNode>(0, move(st), rews, aoh);
        count++;
        for (int k = 0; k < reward.size(); ++k) {
          reward[k] += o.GetReward()[k];
        }
        EFGTreewalk(domain, cn.get(), depth - 1, 1, {});
      }
    } else {
      EFGTreewalk(domain, n.get(), depth, players + 1, locallist);
    }
  }
}
