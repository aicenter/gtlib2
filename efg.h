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

class EFGNode {
 public:
  EFGNode(int player, unique_ptr<PursuitState> state, const vector<InfSet>& aohistories);

  virtual vector<shared_ptr<Action>>  GetActions();

  template<typename T>
  unique_ptr<EFGNode> PerformAction(const shared_ptr<Action>& action2) {
    shared_ptr<T> action = std::dynamic_pointer_cast<T>(action2); // TODO: musi to bejt obecny

    vector<Pos> moves = state_->GetPlace();
    moves[player_].x += state_->GetMoves()[action->GetMove()].x;// TODO: dostat se nejak do PursuitState pro posuny - zatim beru rovnou PursuitState
    moves[player_].y += state_->GetMoves()[action->GetMove()].y;
    unique_ptr<PursuitState>s = MakeUnique<PursuitState>(moves);

    vector<InfSet> aoh = aohistories_;
    aoh[player_].aohistory_.push_back(action->GetID());
    unique_ptr<EFGNode> node = MakeUnique<EFGNode>(1-player_,move(s),aoh);
    return node;
  }

  inline int GetPlayer() const {
    return player_;
  }

  inline vector<double> GetRewards() const {
    return rewards_;
  }

  inline const unique_ptr<PursuitState>& GetState() const {
    return state_;
  }

  inline const InfSet& GetIS() const {
    return aohistories_[player_];
  }

 protected:
  vector<double> rewards_;
  int player_;
  unique_ptr<PursuitState> state_;

 private:


  vector<InfSet> aohistories_;
};

class ChanceNode: public EFGNode {
public:
  ChanceNode(int player, unique_ptr<PursuitState> state, const vector<InfSet>& aohistories);

};

void EFGTreewalk(const unique_ptr<Domain>& domain, const unique_ptr<EFGNode> &node,
                 int depth, int players);

#endif  // EFG_H_
