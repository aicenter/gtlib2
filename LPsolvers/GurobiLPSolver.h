//
// Created by Jakub Rozlivek on 9/19/18.
//

#ifndef LPSOLVERS_GUROBILPSOLVER_H_
#define LPSOLVERS_GUROBILPSOLVER_H_



#include <gurobi_c++.h>
#include <vector>
#include <cmath>
#include "LPsolvers/LPSolver.h"

class GurobiLPSolver : public LPSolver {
 public:
  GurobiLPSolver();

  ~GurobiLPSolver() override;

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
  GRBEnv env_;
  GRBModel model_;
  GRBVar* x_;
  vector<GRBConstr> c_;
};

inline double solveLP(const unsigned int rows, const unsigned int cols,
                      const vector<double> &utility_matrix,
                      vector<double> &solution) {
  // Matrix's rows are indexed by player1. Cols are indexed by player2
  // Utility matrix is for player1. It's zero sum, so utility matrix of player2 is -utility_matrix

  try {
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);
    GRBVar V = model.addVar(-GRB_INFINITY, GRB_INFINITY, 1.0, GRB_CONTINUOUS, "V");

    GRBLinExpr linexpr;
    GRBVar* x_ = model.addVars(&vector<double>(cols, 0)[0], &vector<double>(cols, 1)[0],
                        nullptr, &vector<char>(cols, GRB_CONTINUOUS)[0], nullptr, cols);
    model.setObjective(GRBLinExpr(V), GRB_MAXIMIZE);
    for (int i = 0; i < cols; ++i) {
      linexpr += x_[i];
    }
    model.addConstr(linexpr, GRB_EQUAL, 1, "prob");

    for (int i = 0; i < cols; ++i) {
      GRBLinExpr sum;
      for (int j = 0; j < rows; ++j) {
        sum += utility_matrix[j * cols + i] * x_[j];
      }
      model.addConstr(sum - V, GRB_GREATER_EQUAL, 0);
    }
    std::cout << model.get(GRB_IntAttr_NumConstrs) << " " << model.get(GRB_IntAttr_NumVars) <<"\n";

    model.optimize();
    int optimstatus = model.get(GRB_IntAttr_Status);
    if (optimstatus == GRB_INF_OR_UNBD) {
      model.set(GRB_IntParam_Presolve, 0);
      model.optimize();
      optimstatus = model.get(GRB_IntAttr_Status);
    }

    if (optimstatus != GRB_OPTIMAL && optimstatus != GRB_SUBOPTIMAL &&
    optimstatus != GRB_USER_OBJ_LIMIT) {
      std::cout << "Failed to optimize LP\n";
      throw(-1);
    }
    std::cout << "Solution status = " << model.get(GRB_IntAttr_Status) << "\n";
    std::cout << "Solution value = " << model.get(GRB_DoubleAttr_ObjVal) << "\n";

    for (int i = 0; i < cols; ++i) {
      solution[i] = x_[i].get(GRB_DoubleAttr_X);
    }
    double value = model.get(GRB_DoubleAttr_ObjVal);
    delete[] x_;
    return value;
  } catch(GRBException &e) {
    std::cout << "Error code = " << e.getErrorCode() << std::endl;
    std::cout << e.getMessage() << std::endl;
    return NAN;
  } catch (...) {
    std::cout << "Error during optimization" << std::endl;
    throw(-1);
  }
}

#endif  // LPSOLVERS_GUROBILPSOLVER_H_
