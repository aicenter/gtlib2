//
// Created by Pavel Rytir on 22/01/2018.
//

#ifndef GTLIB2_CPLEXSOLVER_H
#define GTLIB2_CPLEXSOLVER_H
#include <vector>

using std::vector;

double solveLP(int rows, int cols,
               const vector<double> &utility_matrix,
               vector<double> &solution);

#endif //GTLIB2_CPLEXSOLVER_H
