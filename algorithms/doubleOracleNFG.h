//
// Created by bbosansky on 8/17/17.
//

#ifndef ALGORITHMS_DOUBLEORACLENFG_H_
#define ALGORITHMS_DOUBLEORACLENFG_H_

#include "normalFormLP.h"
#include "../base/base.h"

class DoubleOracleNFG {
  explicit DoubleOracleNFG(const int _p1_actions, const int _p2_actions, shared_ptr<Domain> _game);
  explicit DoubleOracleNFG(const int _p1_actions,
                           const int _p2_actions,
                           const vector<double> &_utilities);
  explicit DoubleOracleNFG(const int _p1_actions,
                           const int _p2_actions,
                           const vector<vector<double>> &_utilities);
  explicit DoubleOracleNFG(const int _p1_actions,
                           const int _p2_actions,
                           const vector<vector<double>> &_utilities,
                           const vector<int> _init_for_p1,
                           const vector<int> _init_for_p2);

  double SolveGame();
  shared_ptr<vector<double>> GetStrategy(int _player);

 protected:
  NormalFormLP n_lp_;
  double value_of_the_game_ = NAN;
};

#endif  // ALGORITHMS_DOUBLEORACLENFG_H_
