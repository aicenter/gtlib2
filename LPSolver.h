//
// Created by rozliv on 16.10.2017.
//

#ifndef LPSOLVER_H_
#define LPSOLVER_H_

#include <vector>
#include <iostream>


using std::vector;

class LPSolver {
 public:
  // constructor
  LPSolver() = default;

  // destructor
  virtual ~LPSolver() = default;

  virtual void CleanModel() = 0;

  virtual double SolveGame() = 0;

  virtual void BuildModel(int rows, int cols,
                          const vector<double>* utility_matrix,
                          bool OUTPUT) = 0;

  virtual double const GetValue(int index) const = 0;

  virtual double const GetDual(int index) const = 0;

  virtual void SaveLP(const char* file) = 0;

  virtual void SetConstraintCoefForVariable(int constraint, int variable,
                                            double new_utility) = 0;

  virtual void AddRows(int cols,
                       const vector<vector<double>>& utility_for_cols) = 0;

  virtual void AddCols(int rows,
                       const vector<vector<double>>& utility_for_rows) = 0;
};

#endif  // LPSOLVER_H_
