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

    You should have received a copy of the GNU Lesser General Public 
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef GTLIB2_LPSOLVER_H
#define GTLIB2_LPSOLVER_H

#include "base/base.h"
#include <iostream>
#include <cassert>
#include <cmath>

#define NO_LP_SOLVER 0
#define GUROBI_SOLVER 1
#define CPLEX_SOLVER 2
#define GLPK_SOLVER 3

#if GUROBIFOUND == 1
    #define LP_SOLVER GUROBI_SOLVER
    #include "LPsolvers/GurobiLPSolver.h"
#elif CPLEXFOUND == 1
    #define LP_SOLVER CPLEX_SOLVER
    #include "LPsolvers/CplexLPSolver.h"
#elif GLPKFOUND == 1
    #define LP_SOLVER GLPK_SOLVER
    #include "LPsolvers/GlpkLPSolver.h"
#else
    #define LP_SOLVER NO_LP_SOLVER

namespace GTLib2::algorithms {
    inline double solveLP(const unsigned int rows,
                          const unsigned int cols,
                          const vector<double> &utility_matrix,
                          vector<double> &solution) {
        assert(("No LP solver included to project", false));
        return NAN;
    }
}  // namespace GTLib2

#endif

#endif //GTLIB2_LPSOLVER_H
