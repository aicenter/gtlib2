//
// Created by rozliv on 19.10.17.
//

#ifndef GLPKLPSOLVER_H_
#define GLPKLPSOLVER_H_

#include <glpk.h>
#include "../LPSolver.h"



class GlpkLPSolver: public LPSolver {
 public:
  GlpkLPSolver() = default;

  ~GlpkLPSolver() override = default;

  void CleanModel() final;

  double SolveGame() final;

  void BuildModel(int rows, int cols, const vector<double>* utility_matrix,
                  bool OUTPUT) final;

  double const GetValue(int index) const final;

  double const GetDual(int index) const final;

  void SaveLP(const char* file) final;

  void SetConstraintCoefForVariable(int constraint, int variable,
                                    double new_utility) final;

  void AddRows(int cols, const vector<vector<double>>& utility_for_cols) final;

  void AddCols(int rows, const vector<vector<double>>& utility_for_rows) final;


 protected:
  glp_prob *lp;
};


#endif  // GLPKLPSOLVER_H_
