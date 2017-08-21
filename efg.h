//
// Created by rozliv on 14.08.2017.
//

#ifndef EFG_H_
#define EFG_H_

#include "pursuit.h"

/**
 * EFGNode is a class that represents node in an extensive form game,
 * which contains action-observation history, state,
 * rewards (utility) and Information set.  */
class EFGNode {
 public:
  // constructor
  EFGNode(int player, const shared_ptr<State>& state,
          const vector<double>& rewards);

  // GetAction returns a possible actions for the player in the node.
  virtual vector<shared_ptr<Action>>  GetAction();

  // PerformAction performs the player's action.
  unique_ptr<EFGNode> PerformAction(const shared_ptr<Action>& action2);

  // GetPlayer returns player on the turn (player in this node).
  inline int GetPlayer() const {
    return player_;
  }

  // GetRewards returns rewards for all players in this node.
  inline const vector<double>& GetRewards() const {
    return rewards_;
  }

  // GetState returns a game state
  inline const shared_ptr<State>& GetState() const {
    return state_;
  }

  // GetIS returns the player's information set
  inline std::size_t GetIS() const {
    InfSet i(state_->GetAOH()[player_]);
    return i.GetIS();
  }

 protected:
  vector<double> rewards_;
  int player_;
  shared_ptr<State> state_;
  // vector<InfSet> aohistories_;
};

/**
 * ChanceNode is a class which encapsulates probability distribution,
 * making new EFGNodes from outcomes */
class ChanceNode {
 public:
  // constructor
  explicit ChanceNode(ProbDistribution* prob,
                      const vector<shared_ptr<Action>>& list,
                      const unique_ptr<EFGNode>& node):
      prob_(prob), list_(list), node_(node) {}

  // GetRandom returns a random new EFGNode.
  unique_ptr<EFGNode> GetRandom() {
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

  // GetALL returns a vector of all new EFGNodes.
  vector<unique_ptr<EFGNode>> GetAll() {
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

 private:
  ProbDistribution* prob_;  // probability distribution over the new state
  const vector<shared_ptr<Action>>& list_;  // actions made in the last state
  const unique_ptr<EFGNode>& node_;  // a present node
};


// Domain independent extensive form game treewalk algorithm
void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players,
                 const vector<shared_ptr<Action>>& list);

#endif  // EFG_H_
