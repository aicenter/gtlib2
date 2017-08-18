//
// Created by rozliv on 14.08.2017.
//

#ifndef EFG_H_
#define EFG_H_

#include "pursuit.h"

class InfSet {  // TODO(rozlijak): make it abstract
 public:
  vector<int> aohistory_;
};

class EFGNode {
 public:
  EFGNode(int player, unique_ptr<State> state, const vector<double>& rewards,
          const vector<InfSet>& aohistories);

  virtual vector<shared_ptr<Action>>  GetActions();

  unique_ptr<EFGNode> PerformAction(const shared_ptr<Action>& action2);

  inline int GetPlayer() const {
    return player_;
  }

  inline const vector<double>& GetRewards() const {
    return rewards_;
  }

  inline const vector<InfSet>& GetAOH() const {
    return aohistories_;
  }

  inline const unique_ptr<State>& GetState() const {
    return state_;
  }

  inline const InfSet& GetIS() const {
    return aohistories_[player_];
  }

 protected:
  vector<double> rewards_;
  int player_;
  unique_ptr<State> state_;
  vector<InfSet> aohistories_;
};


class ChanceNode: public EFGNode {
 public:
  ChanceNode(int player, unique_ptr<State> state,
             const vector<double>& rewards, const vector<InfSet>& aohistories);
};

void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players, const vector<shared_ptr<Action>>& list);

#endif  // EFG_H_
