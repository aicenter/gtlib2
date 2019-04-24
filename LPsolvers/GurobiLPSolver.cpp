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

#include "base/base.h"
#include "LPsolvers/GurobiLPSolver.h"
namespace GTLib2::algorithms {


GurobiLPSolver::GurobiLPSolver(): env_(GRBEnv()), model_(GRBModel(env_)) {}

GurobiLPSolver::~GurobiLPSolver() {
  model_.reset(1);
  env_.resetParams();
}

void GurobiLPSolver::CleanModel() {
  model_.getEnv().resetParams();
  model_.reset(1);
}

double GurobiLPSolver::SolveGame() {
  model_.optimize();
  int optimstatus = model_.get(GRB_IntAttr_Status);
  if (optimstatus == GRB_INF_OR_UNBD) {
    model_.set(GRB_IntParam_Presolve, 0);
    model_.optimize();
    optimstatus = model_.get(GRB_IntAttr_Status);
  }

  if (optimstatus != GRB_OPTIMAL && optimstatus != GRB_SUBOPTIMAL &&
      optimstatus != GRB_USER_OBJ_LIMIT) {
    cout << "Failed to optimize LP\n";
    throw(-1);
  }
  return model_.get(GRB_DoubleAttr_ObjVal);
}

void GurobiLPSolver::BuildModel(unsigned int rows,
                                unsigned int cols,
                                const vector<double> *utility_matrix,
                                bool OUTPUT) {
  GRBVar V = model_.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "V");
  GRBLinExpr linexpr;
  auto x = model_.addVars(&vector<double>(rows)[0], &vector<double>(rows, 1)[0],
                     nullptr, &vector<char>(rows, GRB_CONTINUOUS)[0], nullptr, rows);
  model_.setObjective(GRBLinExpr(V), GRB_MAXIMIZE);
  model_.update();
  for (int i = 0; i < rows; ++i) {
    linexpr += x[i];
  }
  model_.addConstr(linexpr, GRB_EQUAL, 1, "prob");

  for (int i = 0; i < cols; ++i) {
    GRBLinExpr sum;
    for (int j = 0; j < rows; ++j) {
      sum += utility_matrix->operator[](j * cols + i) *  x[j];
    }
    model_.addConstr(sum - V, GRB_GREATER_EQUAL, 0);
  }
  model_.update();
  delete[](x);
}

double const GurobiLPSolver::GetValue(int index) const {
  return model_.getVar(index+1).get(GRB_DoubleAttr_X);
}

double const GurobiLPSolver::GetDual(int index) const {
  return model_.getConstr(index+1).get(GRB_DoubleAttr_Pi);
}

void GurobiLPSolver::SaveLP(const char *file) {
  model_.write(file);
}

void GurobiLPSolver::SetConstraintCoefForVariable(int constraint,
                                                  int variable,
                                                  double new_utility) {
  model_.chgCoeff(model_.getConstr(constraint+1), model_.getVar(variable+1), new_utility);
}

void GurobiLPSolver::AddRows(int cols, const vector<vector<double>> &utility_for_cols) {
  unsigned int new_rows = utility_for_cols.size();
  GRBLinExpr V = model_.getObjective().getLinExpr();
  for (int i = 0; i < new_rows; ++i) {
    GRBLinExpr sum;
    for (int j = 0; j < cols; ++j) {
      sum += utility_for_cols[i][j] * model_.getVar(j+1);
    }
    model_.addConstr(sum-V, GRB_GREATER_EQUAL , 0);
  }
  model_.update();
}

void GurobiLPSolver::AddCols(int rows, const vector<vector<double>> &utility_for_rows) {
  unsigned int new_cols = utility_for_rows.size();
  auto new_x = model_.addVars(&vector<double>(new_cols)[0], &vector<double>(new_cols, 1)[0],
      nullptr, &vector<char>(new_cols, GRB_CONTINUOUS)[0], nullptr, new_cols);
  model_.update();
  for (int i = 0; i < new_cols; i++) {
    model_.chgCoeff(model_.getConstr(0), new_x[i], 1);
  }
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < new_cols; j++) {
      model_.chgCoeff(model_.getConstr(i+1), new_x[j], utility_for_rows[j][i]);
    }
  }
  delete[](new_x);
}

}
