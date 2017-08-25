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

  // GetNumPlayers returns player on the turn (player in this node).
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
  inline shared_ptr<InfSet> GetIS() {
    if (aoh_ == nullptr)
      aoh_ = std::make_shared<AOH>(player_ & 1, state_->GetAOH()[player_ & 1]);
    return aoh_;
  }

  int IS = -1;  // information set id

 protected:
  int player_;
  shared_ptr<State> state_;
  vector<double> rewards_;
  shared_ptr<AOH> aoh_;
};

/**
 * ChanceNode is a class which encapsulates probability distribution,
 * making new EFGNodes from outcomes */
class ChanceNode {
 public:
  // constructor
  ChanceNode(ProbDistribution* prob,
             const vector<shared_ptr<Action>>& list,
             const unique_ptr<EFGNode>& node);

  explicit ChanceNode(ProbDistribution* prob, const unique_ptr<EFGNode>& node);

  // GetRandom returns a random new EFGNode.
  unique_ptr<EFGNode> GetRandom();

  // GetALL returns a vector of all new EFGNodes.
  vector<unique_ptr<EFGNode>> GetAll();

 private:
  ProbDistribution* prob_;  // probability distribution over the new state
  const vector<shared_ptr<Action>>& list_;  // actions made in the last state
  const unique_ptr<EFGNode>& node_;  // a current node
};


extern vector<shared_ptr<InfSet>> arrIS;  // temporary for testing information sets

// Domain independent extensive form game treewalk algorithm
void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players,
                 const vector<shared_ptr<Action>>& list);


#endif  // EFG_H_
