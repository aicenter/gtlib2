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


unique_ptr<EFGNode> ChanceNode::GetRandom() {
  Outcome o = prob_->GetRandom();
  shared_ptr<State> st = o.GetState();
  vector<vector<int>> aoh = node_->GetState()->GetAOH();
  vector<unique_ptr<Observation>> obs = o.GetObs();
  for (unsigned int j = 0; j < list_.size(); ++j) {
    aoh[j].push_back(list_[j]->GetID());
  }
  for (unsigned int j = 0; j < obs.size(); ++j) {
    aoh[j].push_back(obs[j]->GetID());
  }
  st->SetAOH(aoh);
  vector<double> rews = vector<double>({node_->GetRewards()[0] + o.GetReward()[0],
                                        node_->GetRewards()[1] + o.GetReward()[1]});
  unique_ptr<EFGNode> cn = MakeUnique<EFGNode>(0, move(st), rews);
  return cn;
}


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
    vector<unique_ptr<Observation>> obs = o.GetObs();
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


vector<shared_ptr<InfSet>> arrIS;

void EFGTreewalk(const shared_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players,
                 const vector<shared_ptr<Action>>& list,
                 std::function<void(EFGNode*)> FunctionForState) {
  ProbDistribution* prob2 = domain->GetProb();
  if (depth == domain->GetMaxDepth() && prob2 != nullptr) {
    ChanceNode chan(prob2);
    vector<unique_ptr<EFGNode>> vec = chan.GetAll();
    for (auto &j : vec) {
      count++;
      for (unsigned int k = 0; k < reward.size(); ++k) {
        reward[k] += j->GetRewards()[k];
      }
      EFGTreewalk(domain, j.get(), depth - 1, 1, {});
    }
    return;
  }
  if (node == nullptr) {
    throw("Node is NULL");
  }

  if (depth == 0) {
//    cout << arrIS.size() <<" ";
    return;
  }

  FunctionForState(node);

  unsigned int l = 0;
  vector<shared_ptr<Action>> actions = node->GetAction();
  for (l = 0; l < arrIS.size(); ++l) {
    if (*arrIS[l] == *node->GetIS()) {
      node->IS = l;
      break;
    }
  }
  if (arrIS.empty() || l == arrIS.size()) {
    int player = node->GetPlayer();
    arrIS.push_back(std::make_shared<AOH>(player,
                                          node->GetState()->GetAOH()[player]));
    node->IS = arrIS.size() - 1;
  }

  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> locallist = list;
    locallist.push_back(i);
    unique_ptr<EFGNode> n2;
    int actionssize = locallist.size();
    if (players == n->GetState()->GetNumPlayers()) {
      while (node->GetPlayer() >= actionssize) {
        locallist.insert(locallist.begin(), std::make_shared<Action>(NoA));
        ++actionssize;
      }
      while (domain->GetMaxPlayers() > actionssize) {
        locallist.push_back(std::make_shared<Action>(NoA));
        ++actionssize;
      }

      // if all players play in this turn, returns a ProbDistribution
      ProbDistribution prob = n->GetState()->PerformAction(locallist);
      ChanceNode chan(&prob, locallist, n);
      vector<unique_ptr<EFGNode>> vec = chan.GetAll();
      for (auto &j : vec) {
        count++;
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