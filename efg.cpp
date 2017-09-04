//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"


EFGNode::EFGNode(int player, const shared_ptr<State>& state,
                 const vector<double>& rewards, EFGNode* node):
    player_(player), state_(state), rewards_(rewards), infset_(nullptr), node_(node)  {}

EFGNode::EFGNode(int player, const shared_ptr<State> &state,
                 const vector<double> &rewards, EFGNode *node,
                 vector<int> list):
    player_(player), state_(state), rewards_(rewards), infset_(nullptr), node_(node), last_(move(list)) {}

EFGNode::EFGNode() {
  player_ = -1;
  node_ = nullptr;
}

vector<shared_ptr<Action>> EFGNode::GetAction() {
  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
  int player = player_ + 1;
  if (player >= rewards_.size())
    player -= rewards_.size();
  return MakeUnique<EFGNode>(player, state_, rewards_, this);
}

vector<int> EFGNode::GetAOH(int player) const {
  if (node_ == nullptr) {
    if (player == player_) {
      if (!last_.empty()) {
        return {last_[0], last_[1]};
      }
    }
    return {};
  }
  vector<int> list = node_->GetAOH(player);
  if (player == player_) {
    if (!last_.empty()) {
      list.push_back(last_[0]);
      list.push_back(last_[1]);
    }
  }
  return list;
}


ChanceNode::ChanceNode(ProbDistribution* prob,
                       const vector<shared_ptr<Action>> &actions,
                       const unique_ptr<EFGNode>& node):
    prob_(prob), actions_(actions),  node_(node) {}


vector<unique_ptr<EFGNode>> ChanceNode::GetAll() {
  vector<Outcome> outcomes = prob_->GetOutcomes();
  vector<unique_ptr<EFGNode>> vec;
  vec.reserve(outcomes.size());
  if (node_->GetPlayer() >= 0) {  // preparing first states
    for (Outcome &o : outcomes) {
      vector<double> rews = vector<double>(o.GetReward().size());
      for (int j = 0; j < rews.size(); ++j) {
        rews[j] = node_->GetRewards()[j] + o.GetReward()[j];
      }
      for (unsigned int i = 0; i < o.GetState()->GetPlayers().size(); ++i) {
        if (o.GetState()->GetPlayers()[i]) {
          vector<int> aoh{actions_[i]->GetID(), o.GetObs()[i]->GetID()};
          for (unsigned int j = 0; j < o.GetState()->GetPlayers().size(); ++j) {
            if (j == i)
              continue;
            aoh.push_back(actions_[j]->GetID());
            aoh.push_back(o.GetObs()[j]->GetID());
          }
          auto n = MakeUnique<EFGNode>(i, o.GetState(), rews, node_.get(), aoh);
          vec.push_back(move(n));
          break;
        }
      }
    }
    return vec;
  }
  for (Outcome &o : outcomes) {
    vec.push_back(MakeUnique<EFGNode>(0, o.GetState(),
                                      vector<double>(o.GetReward().size()), node_.get()));
  }
  return vec;
}


unordered_map<size_t, vector<EFGNode>> mapa;

void EFGTreewalkStart(const shared_ptr<Domain>& domain,
                      std::function<void(EFGNode*)> FunctionForState,
                      unsigned int depth) {
  if (depth == 0)
    depth = domain->GetMaxDepth();
  auto node = MakeUnique<EFGNode>();
  ChanceNode chan(domain->GetRoot().get(), {}, node);
  vector<unique_ptr<EFGNode>> vec = chan.GetAll();
  for (auto &j : vec) {
    ++countStates;
    for (unsigned int k = 0; k < reward.size(); ++k) {
      reward[k] += j->GetRewards()[k];
    }
    EFGTreewalk(domain, j.get(), depth, 1, {}, FunctionForState);
  }
}

void EFGTreewalkStart(const shared_ptr<Domain>& domain, unsigned int depth) {
  EFGTreewalkStart(domain, [](EFGNode* s){}, depth);
}


void EFGTreewalk(const shared_ptr<Domain>& domain, EFGNode *node,
                 unsigned int depth, int players,
                 const vector<shared_ptr<Action>>& list,
                 std::function<void(EFGNode*)> FunctionForState) {
  if (node == nullptr) {
    throw("Node is NULL");
  }

  if (depth == 0) {
    return;
  }

  FunctionForState(node);

  vector<shared_ptr<Action>> actions = node->GetAction();

  auto search = mapa.find(node->GetIS()->GetHash());
  if (search != mapa.end()) {
    search->second.emplace_back(node->GetPlayer(), node->GetState(),
                                node->GetRewards(), node->GetParent(),
                                node->GetLast());
  } else {
    auto n2 = EFGNode(node->GetPlayer(), node->GetState(),
                      node->GetRewards(), node->GetParent(), node->GetLast());
    mapa.emplace(n2.GetIS()->GetHash(), vector<EFGNode>{n2});
  }

  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> locallist = list;
    locallist.push_back(i);

    if (players == n->GetState()->GetNumPlayers()) {
      int actionssize = locallist.size();
      int index = 1;
      while (node->GetPlayer() >= actionssize) {
        n->PushLast(node->GetLast()[2 * index], node->GetLast()[2 * index + 1]);
        locallist.insert(locallist.begin(), make_shared<Action>(NoA));
        ++actionssize;
        ++index;
      }
      while (domain->GetMaxPlayers() > locallist.size()) {
        locallist.push_back(make_shared<Action>(NoA));
        if (!node->GetLast().empty()) {
          n->PushLast(node->GetLast()[2* index], node->GetLast()[2* index + 1]);
        }
        ++index;
      }
     // if all players play in this turn, returns a ProbDistribution
      ProbDistribution prob = n->GetState()->PerformAction(locallist);
      ChanceNode chan(&prob, locallist, n);
      vector<unique_ptr<EFGNode>> vec = chan.GetAll();
      for (auto &j : vec) {
        ++countStates;
        for (unsigned int k = 0; k < reward.size(); ++k) {
          reward[k] += j->GetRewards()[k];
        }
        EFGTreewalk(domain, j.get(), depth - 1, 1, {}, FunctionForState);
      }
    } else {
      if (depth < domain->GetMaxDepth()) {
        int index = n->GetPlayer();
        while (index < domain->GetMaxPlayers()) {
          n->PushLast(node->GetLast()[2 * index], node->GetLast()[2 * index + 1]);
          ++index;
        }
    }
      EFGTreewalk(domain, n.get(), depth, players + 1, locallist, FunctionForState);
    }
  }
}

void EFGTreewalk(const shared_ptr<Domain>& domain, EFGNode *node,
                 unsigned int depth, int players,
                 const vector<shared_ptr<Action>>& list) {
  EFGTreewalk(domain, node, depth, players, list, [](EFGNode* s){});
}
