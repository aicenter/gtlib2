//
// Created by rozliv on 14.08.2017.
//

#ifndef EFG_H_
#define EFG_H_

#include "pursuit.h"

class InfSet {
 public:
  vector<int> aohistory_;
};

class ChanceNode;

class EFGNode {
 public:
  EFGNode(int player, unique_ptr<State> state, const vector<double>& rewards,
          const vector<InfSet>& aohistories);

  virtual vector<shared_ptr<Action>>  GetActions();

  template<typename T>
  unique_ptr<ChanceNode> PerformAction(const shared_ptr<Action>& action2) {
    shared_ptr<T> action = std::dynamic_pointer_cast<T>(action2);  // TODO: musi to bejt obecny
    unique_ptr<PursuitState> st = MakeUnique<PursuitState>(state_->GetPlace());  // TODO: taky se to musi resit nejak obecne
    vector<Pos> moves = st->GetPlace();
    moves[player_].x += st->GetMoves()[action->GetMove()].x;  // TODO: dostat se nejak do PursuitState pro posuny - zatim beru State a menim na PursuitState
    moves[player_].y += st->GetMoves()[action->GetMove()].y;
    unique_ptr<PursuitState>s = MakeUnique<PursuitState>(moves);

    vector<InfSet> aoh = aohistories_;
    aoh[player_].aohistory_.push_back(action->GetID());
    unique_ptr<ChanceNode> node = MakeUnique<ChanceNode>(
        player_, move(s), rewards_, aoh);
    return node;
  }

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

  ProbDistribution GetDistribution(const vector<Pos>& loc);
};

void EFGTreewalk(const unique_ptr<Domain>& domain, EFGNode *node,
                 int depth, int players);

#endif  // EFG_H_
