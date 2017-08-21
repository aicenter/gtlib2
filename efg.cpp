//
// Created by rozliv on 14.08.2017.
//

#include "efg.h"

EFGNode::EFGNode(int player, const shared_ptr<State>& state,
                 const vector<double>& rewards):
    player_(player), state_(state), rewards_(rewards)  {}

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

ChanceNode::ChanceNode(ProbDistribution* prob,
                       const vector<shared_ptr<Action>>& list,
                       const unique_ptr<EFGNode>& node):
    prob_(prob), list_(list), node_(node) {}


unique_ptr<EFGNode> ChanceNode::GetRandom() {
  Outcome o = prob_->GetRandom();
  unique_ptr<State> st = o.GetState();
  vector<vector<int>> aoh = node_->GetState()->GetAOH();
  vector<unique_ptr<Observation>> obs = o.GetObs();
  for (int j = 0; j < list_.size(); ++j) {
    aoh[j].push_back(list_[j]->GetID());
  }
  for (int j = 0; j < obs.size(); ++j) {
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
  for (Outcome &o : outcomes) {
    unique_ptr<State> st = o.GetState();
    vector<vector<int>> aoh = node_->GetState()->GetAOH();
    vector<unique_ptr<Observation>> obs = o.GetObs();
    for (int j = 0; j < list_.size(); ++j) {
      aoh[j].push_back(list_[j]->GetID());
    }
    if(list_.size() < obs.size()) aoh[1].push_back(-1);
    for (int j = 0; j < obs.size(); ++j) {
      aoh[j].push_back(obs[j]->GetID());
    }
    st->SetAOH(aoh);
    vector<double> rews = vector<double>({node_->GetRewards()[0] + o.GetReward()[0],
                                          node_->GetRewards()[1] + o.GetReward()[1]});
    vec.push_back(MakeUnique<EFGNode>(0, move(st), rews));
  }
  return vec;
}

vector<shared_ptr<AOH>> arrIS;

void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players,
                 const vector<shared_ptr<Action>>& list) {
  if (node == nullptr) {
    throw("Node is NULL");
  }
  if (depth == 0) {
    return;
  }
  int l = 0;
  vector<shared_ptr<Action>> actions = node->GetAction();
  for (l = 0; l < arrIS.size(); ++l) {
    if(arrIS[l]->player_ == node->GetPlayer() &&
        arrIS[l]->GetIS() == node->GetIS() &&
        arrIS[l]->aohistory_ == node->GetState()->GetAOH()[node->GetPlayer()]) {
      node->IS = l;
      break;
    }
  }
  if(arrIS.empty() || l == arrIS.size()) {
    arrIS.push_back(std::make_shared<AOH>(node->GetPlayer(),actions.size(), node->GetState()->GetAOH()[node->GetPlayer()]));
    node->IS = arrIS.size() - 1;
  }

  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> locallist = list;
    locallist.push_back(i);
//    node->GetState()->SetPlayers(1);
//    locallist.push_back(std::make_shared<Action>(NoA));
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
