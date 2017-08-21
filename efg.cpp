//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

EFGNode::EFGNode(int player, const shared_ptr<State>& state,
                 const vector<double>& rewards):
    player_(player), state_(state), rewards_(rewards) {
}

vector<shared_ptr<Action>> EFGNode::GetAction() {
  if (player_ == -1)
    return {};
  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(
      player_+1, state_, rewards_);
  return node;
}


void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players,
                 const vector<shared_ptr<Action>>& list) {
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
      ChanceNode chan(&prob, locallist, n);
      vector<unique_ptr<EFGNode>> vec = chan.GetAll();
      for (auto &j : vec) {
        count++;
        for (int k = 0; k < reward.size(); ++k) {
          reward[k] += j->GetRewards()[k];
        }
        EFGTreewalk(domain, j.get(), depth - 1, 1, {});
      }
    } else {
      EFGTreewalk(domain, n.get(), depth, players + 1, locallist);
    }
  }
}
