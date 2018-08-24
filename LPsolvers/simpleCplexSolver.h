//
// Created by Pavel Rytir on 22/01/2018.
//

#ifndef LPSOLVERS_SIMPLECPLEXSOLVER_H_
#define LPSOLVERS_SIMPLECPLEXSOLVER_H_
#include <vector>

using std::vector;

double solveLP(int rows, int cols,
               const vector<double> &utility_matrix,
               vector<double> &solution);

#endif  // LPSOLVERS_SIMPLECPLEXSOLVER_H_
