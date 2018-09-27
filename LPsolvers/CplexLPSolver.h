//
// Created by Jakub Rozlivek on 16.10.17.
//

#ifndef LPSOLVERS_CPLEXLPSOLVER_H_
#define LPSOLVERS_CPLEXLPSOLVER_H_

#define IL_STD

#include <ilcplex/ilocplex.h>
#include <vector>
#include "LPsolvers/LPSolver.h"

class CplexLPSolver : public LPSolver {
 public:
  CplexLPSolver() = default;

  ~CplexLPSolver() override;

  void CleanModel() final;

  double SolveGame() final;

  void BuildModel(unsigned int rows,
                  unsigned int cols,
                  const vector<double> *utility_matrix,
                  bool OUTPUT) final;

  double const GetValue(int index) const final;

  double const GetDual(int index) const final;

  void SaveLP(const char *file) final;

  void SetConstraintCoefForVariable(int constraint, int variable, double new_utility) final;

  void AddRows(int cols, const vector<vector<double>> &utility_for_cols) final;

  void AddCols(int rows, const vector<vector<double>> &utility_for_rows) final;

 protected:
  IloEnv* env_;
  IloModel model_;
  IloCplex cplex_;

  IloNumVarArray x_;
  IloRangeArray c_;
  IloRange prob_;
};

inline double solveLP(const unsigned int rows, const unsigned int cols,
               const vector<double> &utility_matrix,
               vector<double> &solution) {
  // Matrix's rows are indexed by player1. Cols are indexed by player2
  // Utility matrix is for player1. It's zero sum, so utility matrix of player2 is -utility_matrix

  try {
    IloEnv env;
    IloModel model(env);
    IloNumVarArray x(env, cols, 0, 1, ILOFLOAT);
    IloRangeArray c(env);
    IloNumVar V(env, "V");  // V is utility of player1
    model.add(IloMaximize(env, V));


    // X's are probabilities constraints
    IloExpr probExpr(env);
    for (int i = 0; i < cols; ++i) {
      probExpr += x[i];
    }
    IloRange prob(probExpr == 1);
    model.add(prob);

    for (int i = 0; i < cols; ++i) {
      IloExpr sum(env);
      for (int j = 0; j < rows; ++j) {
        sum += utility_matrix[j * cols + i] * x[j];
      }
      c.add(IloRange(env, 0, sum - V));
    }
    model.add(c);

    IloCplex cplex(model);

    if (!cplex.solve()) {
      env.error() << "Failed to optimize LP." << "\n";
      throw(-1);
    }
    IloNumArray vals(env);
    env.out() << "Solution status = " << cplex.getStatus() << "\n";
    env.out() << "Solution value = " << cplex.getObjValue() << "\n";
    cplex.getValues(vals, x);

    for (int i = 0; i < cols; ++i) {
      solution[i] = vals[i];
    }
    return cplex.getObjValue();
  } catch (IloException &e) {
    std::cerr << "Concert exception caught: " << e << "\n";
    return NAN;
  }
}

#endif  // LPSOLVERS_CPLEXLPSOLVER_H_
