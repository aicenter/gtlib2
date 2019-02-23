//
// Created by Jakub Rozlivek on 16.10.17.
//

#include "LPsolvers/CplexLPSolver.h"

CplexLPSolver::~CplexLPSolver() {
  if (model_.isValid()) {
    model_.end();
    env_->end();
  }
  delete env_;
}

void CplexLPSolver::CleanModel() {
  model_.end();
  env_->end();
}

double CplexLPSolver::SolveGame() {
  if (!cplex_.solve()) {
    env_->error() << "Failed to optimize LP" << std::endl;
    throw(-1);
  }
  return cplex_.getObjValue();
}

void CplexLPSolver::BuildModel(unsigned int rows, unsigned int cols,
                               const vector<double> *utility_matrix,
                               bool OUTPUT) {
  env_ = new IloEnv();
  model_ = IloModel(*env_);

  x_ = IloNumVarArray(*env_, rows, 0, 1, ILOFLOAT);
  c_ = IloRangeArray(*env_);
  IloNumVar V(*env_, "V");

  model_.add(IloMaximize(*env_, V));

  IloExpr prob(*env_);
  for (int i = 0; i < rows; ++i) {
    prob += x_[i];
  }
  prob_ = IloRange(prob == 1);
  model_.add(prob_);

  for (int i = 0; i < cols; ++i) {
    IloExpr sum(*env_);
    for (int j = 0; j < rows; ++j) {
      sum += utility_matrix->operator[](j * cols + i) * x_[j];
    }
    c_.add(IloRange(*env_, 0, sum - V));
  }

  model_.add(c_);

  cplex_ = IloCplex(model_);

  if (!OUTPUT) {
    cplex_.setOut(env_->getNullStream());
  }
}

double const CplexLPSolver::GetValue(int index) const {
  return cplex_.getValue(x_[index]);
}

double const CplexLPSolver::GetDual(int index) const {
  return cplex_.getDual(c_[index]);
}

void CplexLPSolver::SaveLP(const char *file) {
  cplex_.exportModel(file);
}

void CplexLPSolver::SetConstraintCoefForVariable(int constraint, int variable, double new_utility) {
  c_[constraint].setLinearCoef(x_[variable], new_utility);
}

void CplexLPSolver::AddRows(int cols, const vector<vector<double>> &utility_for_cols) {
  int new_rows = utility_for_cols.size();

  IloExpr V = cplex_.getObjective().getExpr();

  for (int i = 0; i < new_rows; ++i) {
    IloExpr sum(*env_);
    for (int j = 0; j < cols; ++j) {
      sum += utility_for_cols[i][j] * x_[j];
    }

    c_.add(IloRange(*env_, 0, sum - V));
  }
  model_.add(c_);
}

void CplexLPSolver::AddCols(int rows, const vector<vector<double>> &utility_for_rows) {
  int new_cols = utility_for_rows.size();
  IloNumVarArray new_x = IloNumVarArray(*env_, new_cols, 0, 1, ILOFLOAT);

  for (int i = 0; i < new_cols; i++) {
    prob_.setLinearCoef(new_x[i], 1);
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < new_cols; j++) {
      c_[i].setLinearCoef(new_x[j], utility_for_rows[j][i]);
    }
  }
  x_.add(new_x);
}
