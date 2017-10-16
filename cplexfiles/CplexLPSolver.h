//
// Created by rozliv on 16.10.17.
//

#ifndef CPLEXLPSOLVER_H_
#define CPLEXLPSOLVER_H_

#define IL_STD

#include <ilcplex/ilocplex.h>
#include "../LPSolver.h"



class CplexLPSolver : public LPSolver {
 public:
  CplexLPSolver() = default;

  ~CplexLPSolver() override = default;

  void CleanModel() final;

  double SolveGame() final;

  void BuildModel(int rows, int cols, const vector<double>* utility_matrix, bool OUTPUT) final;

  double const GetValue(int index) const final;

  double const GetDual(int index) const final;

  void SaveLP(const char* file) final;

  void SetConstraintCoefForVariable(int constraint, int variable, double new_utility) final;

  void AddRows(int cols, const vector<vector<double>>& utility_for_cols) final;

  void AddCols(int rows, const vector<vector<double>>& utility_for_rows) final;


 protected:
  IloEnv env_;
  IloModel model_;
  IloCplex cplex_;

  IloNumVarArray x_;
  IloRangeArray c_;
  IloRange prob_;


};


#endif //CPLEXLPSOLVER_H_
