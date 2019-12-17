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


#ifndef LPSOLVERS_LPSOLVER_H_
#define LPSOLVERS_LPSOLVER_H_

#include "base/base.h"
#include <cmath>

namespace GTLib2::algorithms {

class AbstractLPSolver {
 public:
    // constructor
    AbstractLPSolver() = default;

    // destructor
    virtual ~AbstractLPSolver() = default;

    virtual void CleanModel() = 0;

    virtual double SolveGame() = 0;

    virtual void BuildModel(unsigned int rows, unsigned int cols,
                            const vector<double> *utility_matrix,
                            bool OUTPUT) = 0;

    virtual double GetValue(int index) = 0;

    virtual double GetDual(int index) = 0;

    virtual void SaveLP(const char *file) = 0;

    virtual void SetConstraintCoefForVariable(int constraint, int variable, double new_utility) = 0;

    virtual void AddRows(int cols, const vector<vector<double>> &utility_for_cols) = 0;

    virtual void AddCols(int rows, const vector<vector<double>> &utility_for_rows) = 0;
};

}  // namespace GTLib2


#endif  // LPSOLVERS_LPSOLVER_H_

