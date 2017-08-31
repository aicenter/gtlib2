//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"


EFGNode::EFGNode(int player, const shared_ptr<State>& state,
                 const vector<double>& rewards, EFGNode* node):
    player_(player), state_(state), rewards_(rewards), infset_(nullptr), node_(node)  {
}

EFGNode::EFGNode() {
  player_ = -1;
  node_ = nullptr;
}

vector<shared_ptr<Action>> EFGNode::GetAction() {
  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
  return MakeUnique<EFGNode>(player_+1, state_, rewards_, this);
}

vector<int> EFGNode::GetAOH(int player) const {
  if (node_ == nullptr) {
    if (player == (player_ & 1))
      return last_;
    return {};
  }
  vector<int> list = node_->GetAOH(player & 1);
  if (player == (player_ & 1))
    list.insert(list.end(), last_.begin(), last_.end());
  return list;
}


ChanceNode::ChanceNode(ProbDistribution* prob,
                       const unique_ptr<EFGNode>& node):
    prob_(prob),  node_(node) {}


vector<unique_ptr<EFGNode>> ChanceNode::GetAll() {
  vector<Outcome> outcomes = prob_->GetOutcomes();
  vector<unique_ptr<EFGNode>> vec;
  vec.reserve(outcomes.size());
  if (node_->GetPlayer() >= 0) {  // preparing first states
    for (Outcome &o : outcomes) {
      vector<double> rews = vector<double>({node_->GetRewards()[0] + o.GetReward()[0],
                                            node_->GetRewards()[1] + o.GetReward()[1]});
      for (unsigned int i = 0; i < o.GetState()->GetPlayers().size(); ++i) {
        if (o.GetState()->GetPlayers()[i]) {
          vec.push_back(MakeUnique<EFGNode>(i, o.GetState(), rews, node_.get()));
          break;
        }
      }
    }
    return vec;
  }
  for (Outcome &o : outcomes) {
    vec.push_back(MakeUnique<EFGNode>(0, o.GetState(), vector<double>(2), node_.get()));
  }
  return vec;
}


unordered_map<size_t, vector<EFGNode>> mapa;

void EFGTreewalkStart(const shared_ptr<Domain>& domain, unsigned int depth) {
  if (depth == 0)
    depth = domain->GetMaxDepth();
  auto node = MakeUnique<EFGNode>();
  ChanceNode chan(domain->GetRoot().get(), node);
  vector<unique_ptr<EFGNode>> vec = chan.GetAll();
  for (auto &j : vec) {
    ++countStates;
    for (unsigned int k = 0; k < reward.size(); ++k) {
      reward[k] += j->GetRewards()[k];
    }
    EFGTreewalk(domain, j.get(), depth, 1, {});
  }
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

  unsigned int l = 0;
  vector<shared_ptr<Action>> actions = node->GetAction();

  if (depth < domain->GetMaxDepth()) {
    node->AddAOH(node->GetState()->GetLast()[2 * node->GetPlayer()]);
    node->AddAOH(node->GetState()->GetLast()[2 * node->GetPlayer() + 1]);
  }

  auto search = mapa.find(node->GetIS()->GetHash());
  auto n2 = EFGNode(node->GetPlayer(), node->GetState(), node->GetRewards(), node);
  if (search != mapa.end()) {
    search->second.push_back(n2);
  } else {
    mapa.emplace(node->GetIS()->GetHash(), vector<EFGNode>{n2});
  }

  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> locallist = list;
    locallist.push_back(i);

    if (players == n->GetState()->GetNumPlayers()) {
      int actionssize = locallist.size();
      while (node->GetPlayer() >= actionssize) {
        n->AddAOH(n->GetState()->GetLast()[2 * (n->GetPlayer() & 1)]);
        n->AddAOH(n->GetState()->GetLast()[2 * (n->GetPlayer() &1) + 1]);
        locallist.insert(locallist.begin(), make_shared<Action>(NoA));
        ++actionssize;
      }
      while (domain->GetMaxPlayers() > locallist.size()) {
        locallist.push_back(make_shared<Action>(NoA));
        if (!n->GetState()->GetLast().empty()) {
          n->AddAOH(n->GetState()->GetLast()[2 * (n->GetPlayer() & 1)]);
          n->AddAOH(n->GetState()->GetLast()[2 * (n->GetPlayer() & 1) + 1]);
        }
      }
      // if all players play in this turn, returns a ProbDistribution
      ProbDistribution prob = n->GetState()->PerformAction(locallist);
      ChanceNode chan(&prob, n);
      vector<unique_ptr<EFGNode>> vec = chan.GetAll();
      for (auto &j : vec) {
        ++countStates;
        for (unsigned int k = 0; k < reward.size(); ++k) {
          reward[k] += j->GetRewards()[k];
        }
        EFGTreewalk(domain, j.get(), depth - 1, 1, {}, FunctionForState);
      }
    } else {
      EFGTreewalk(domain, n.get(), depth, players + 1, locallist, FunctionForState);
    }
  }
}

void EFGTreewalk(const shared_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players,
                 const vector<shared_ptr<Action>>& list) {
  EFGTreewalk(domain, node, depth, players, list, [](EFGNode* s){});
}