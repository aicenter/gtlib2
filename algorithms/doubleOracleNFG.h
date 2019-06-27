/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ALGORITHMS_DOUBLEORACLENFG_H_
#define ALGORITHMS_DOUBLEORACLENFG_H_

#include "algorithms/normalFormLP.h"

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
