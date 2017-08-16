//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

EFGNode::EFGNode(int player, unique_ptr<PursuitState> state, const vector<InfSet> &aohistories):
    player_(player), state_(move(state)), aohistories_(aohistories) {}

vector<shared_ptr<Action>> EFGNode::GetActions() {
  if(player_ == -1)
    return {};

  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

ChanceNode::ChanceNode(int player, unique_ptr<PursuitState> state, const vector<InfSet> &aohistories) :
    EFGNode(player, move(state), aohistories) {}


void EFGTreewalk(const unique_ptr<Domain>& domain, const unique_ptr<EFGNode> &node,
                 int depth, int players) {
//  for(int i = 0; i < node->GetIS().aohistory_.size(); i++) {
//    cout << node->GetIS().aohistory_[i] << " ";
//  }
//  cout << '\n';
  cout << depth << " ";
  count++;
  if (node == nullptr) {
    cout << "state is NULL\n";
    return;
  }
  if (depth == 0) {
    cout << '\n';
    return;
  }

  vector<shared_ptr<Action>> actions = node->GetActions();
  cout << actions.size() << '\n';

  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction<PursuitAction>(i);
    if(n->GetPlayer() == -1);

    EFGTreewalk(domain, n, depth-1, players);
  }
}
