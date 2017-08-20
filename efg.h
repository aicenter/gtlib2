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
  EFGNode(int player,const shared_ptr<State>& state, const vector<double>& rewards,
          const vector<InfSet>& aohistories);

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

  // GetAOH returns aohistories
  inline const vector<InfSet>& GetAOH() const {  // TODO(rozlijak): rewrite
    return aohistories_;
  }

  // GetState returns a game state
  inline const shared_ptr<State>& GetState() const {
    return state_;
  }

  // GetIS returns the player's information set
  inline const InfSet& GetIS() const {
    return aohistories_[player_];
  }

 protected:
  vector<double> rewards_;
  int player_;
  shared_ptr<State> state_;
  vector<InfSet> aohistories_;
};

/**
 * ChanceNode is a derived class of EFGNode. */
class ChanceNode: public EFGNode {  // TODO(rozlijak): rewrite
 public:
  // constructor
  ChanceNode(int player, const shared_ptr<State>& state,
             const vector<double>& rewards, const vector<InfSet>& aohistories);
};


// Domain independent extensive form game treewalk algorithm
void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players, const vector<shared_ptr<Action>>& list);

#endif  // EFG_H_
