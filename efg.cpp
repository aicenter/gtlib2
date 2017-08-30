//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"



EFGNode::EFGNode(int player, const shared_ptr<State>& state,
                 const vector<double>& rewards):
    player_(player), state_(state), rewards_(rewards), aoh_(nullptr)  {}

vector<shared_ptr<Action>> EFGNode::GetAction() {
  if (player_ == -1)
    return {};
  vector<shared_ptr<Action>> list = state_->GetActions(player_);
  return list;
}

unique_ptr<EFGNode> EFGNode::PerformAction(const shared_ptr<Action> &action2) {
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(player_+1, state_, rewards_);
  return node;
}

ChanceNode::ChanceNode(ProbDistribution* prob,
                       const vector<shared_ptr<Action>>& list,
                       const unique_ptr<EFGNode>& node):
    prob_(prob), list_(list), node_(node) {}


ChanceNode::ChanceNode(ProbDistribution* prob):
    ChanceNode(prob, {}, nullptr) {}


vector<unique_ptr<EFGNode>> ChanceNode::GetAll() {
  vector<Outcome> outcomes =  prob_->GetOutcomes();
  vector<unique_ptr<EFGNode>> vec;
  vec.reserve(outcomes.size());
  if (list_.empty()) {  // preparing first states
    for (Outcome &o : outcomes) {
      vec.push_back(MakeUnique<EFGNode>(0, o.GetState(), vector<double>(2)));
    }
    return vec;
  }
  for (Outcome &o : outcomes) {
    vector<vector<int>> aoh = node_->GetState()->GetAOH();
    vector<shared_ptr<Observation>> obs = o.GetObs();
    for (unsigned int j = 0; j < list_.size(); ++j) {
      aoh[j].push_back(list_[j]->GetID());
    }
    if (list_.size() < obs.size()) aoh[1].push_back(-1);
    for (unsigned int j = 0; j < obs.size(); ++j) {
      aoh[j].push_back(obs[j]->GetID());
//      o.GetState()->AddString(node_->GetState()->GetString(j) + "\nACTION: " +
//                    list_[j]->ToString() + "\nOBS: " + obs[j]->ToString(), j);
      o.GetState()->AddString(node_->GetState()->GetString(j) + "  ||  ACTION: " +
                    list_[j]->ToString() + "  | OBS: " + obs[j]->ToString(), j);
    }
    o.GetState()->SetAOH(aoh);
    vector<double> rews = vector<double>({node_->GetRewards()[0] + o.GetReward()[0],
                                          node_->GetRewards()[1] + o.GetReward()[1]});

    for (unsigned int i = 0; i < o.GetState()->GetPlayers().size(); ++i) {
      if (o.GetState()->GetPlayers()[i]) {
        vec.push_back(MakeUnique<EFGNode>(i, o.GetState(), rews));
        break;
      }
    }
  }
  return vec;
}
unordered_map<size_t, vector<EFGNode>> mapa;
void EFGTreewalkStart(const unique_ptr<Domain>& domain, int depth) {
  if (depth == 0)
    depth = domain->GetMaxDepth();
  ChanceNode chan(domain->GetRoot().get());
  vector<unique_ptr<EFGNode>> vec = chan.GetAll();
  for (auto &j : vec) {
    ++countStates;
    for (unsigned int k = 0; k < reward.size(); ++k) {
      reward[k] += j->GetRewards()[k];
    }
    EFGTreewalk(domain, j.get(), depth, 1, {});
  }
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

  auto search = mapa.find(node->GetIS()->GetHash());
  auto n2 = EFGNode(node->GetPlayer(), node->GetState(), node->GetRewards());
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
        locallist.insert(locallist.begin(), make_shared<Action>(NoA));
        ++actionssize;
      }
      while (domain->GetMaxPlayers() > locallist.size()) {
        locallist.push_back(make_shared<Action>(NoA));
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
        EFGTreewalk(domain, j.get(), depth - 1, 1, {});
      }
    } else {
      EFGTreewalk(domain, n.get(), depth, players + 1, locallist);
    }
  }
}
