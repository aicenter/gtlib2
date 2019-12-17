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


#ifndef ALGORITHMS_NORMALFORMLP_H_
#define ALGORITHMS_NORMALFORMLP_H_

#include "LPsolvers/AbstractLPSolver.h"
#include "algorithms/utility.h"
#include "algorithms/common.h"


namespace GTLib2::algorithms {

/**
 * This class represents the algorithm for solving two-player zero-sum game
 * using a linear program (LP) and normal form.
 * The class requires number of actions for both players  and a reference to the utility matrix.
 * The class does not hold the copy of the utility matrix and directly builds the LP.
 * The class also rebuilds the LP whenever there is a change to the utility matrix.
 */
class NormalFormLP {
 public:
    explicit NormalFormLP(const Domain &domain, unique_ptr<AbstractLPSolver> _lp_solver);
    explicit NormalFormLP(const unsigned int p1_actions, const unsigned int p2_actions,
                          const vector<double> &utilities, unique_ptr<AbstractLPSolver> lp_solver);
    explicit NormalFormLP(const unsigned int p1_actions, const unsigned int p2_actions,
                          const vector<vector<double>> &utilities,
                          unique_ptr<AbstractLPSolver> lp_solver);
    ~NormalFormLP();

    double SolveGame();
    void AddActions(const int player, const vector<vector<double>> &utility_for_opponent);
    void ChangeOutcome(const int action_for_p1, const int action_for_p2, double new_utility);
    vector<double> GetStrategy(int player);

    void UpdateUtilityMatrix(const vector<double> &utilities);
    void UpdateUtilityMatrix(const vector<vector<double>> &utilities);

    void SaveLP(const char *_file);

 protected:
    unique_ptr<AbstractLPSolver> lp_solver_;
    double value_of_the_game_ = NAN;
    unsigned int rows_;
    unsigned int cols_;
    const bool OUTPUT = true;
    bool model_ready_ = false;
    bool model_solved_ = false;

    void CleanModel();
    void BuildModel(const vector<double> *_utility_matrix);
    bool ValidateInput(const int p1_actions, const int p2_actions, const vector<double> &utilities);
    void AddRows(const vector<vector<double>> &_utility_for_cols);
    void AddCols(const vector<vector<double>> &_utility_for_rows);
};
}  // namespace GTLib2

#endif  // ALGORITHMS_NORMALFORMLP_H_

