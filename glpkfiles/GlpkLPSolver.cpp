//
// Created by rozliv on 19.10.17.
//

#include "GlpkLPSolver.h"

void GlpkLPSolver::CleanModel() {
  glp_delete_prob(lp);
  glp_free_env();
}

double GlpkLPSolver::SolveGame() {
  if (glp_simplex(lp, nullptr)) {
    glp_error("Failed to optimize LP");
    throw(-1);
  }
  return glp_get_obj_val(lp);
}

void GlpkLPSolver::BuildModel(int rows, int cols,
                               const vector<double>* utility_matrix,
                               const bool OUTPUT) {
  ++cols;
  vector<int> ia = vector<int>(1+cols*rows+cols);
  vector<int> ja = vector<int>(1+cols*rows+cols);
  vector<double> ar = vector<double>(1+cols*rows+cols);
  lp = glp_create_prob();
  glp_set_obj_dir(lp, GLP_MAX);
  glp_add_rows(lp, rows+1);
  glp_add_cols(lp, cols);

  for (int i = 2; i <= rows+1; ++i) {
    glp_set_row_bnds(lp, i, GLP_LO, 0.0, 0.0);
  }

  for (int i = 1; i <= cols; ++i) {
    glp_set_col_bnds(lp, i, GLP_DB, 0.0, 1.0);
  }
  glp_set_obj_coef(lp, 1, 1);
  glp_set_col_bnds(lp, 1, GLP_FR, 0.0, 1.0);

  for (int i = 1; i <= cols; ++i) {
    ar[i] = 1;
    ja[i] = i;
    ia[i] = 1;
  }
  ar[1] = 0;

  for (int i = cols+1, j = 0; i <= cols*rows+cols; ++i, ++j) {
    if ((i - 1) % cols == 0) {
      ar[i] = -1;
      --j;
    } else {
      ar[i] = utility_matrix->operator[](j);
    }
    ia[i] = (i - 1) / cols + 1;
    ja[i] = (i - 1) % cols + 1;
  }
  glp_set_row_bnds(lp, 1, GLP_FX, 1.0, 1.0);

  int* a = &ia[0];
  int* b = &ja[0];
  double* c = &ar[0];

  glp_load_matrix(lp, cols*rows+cols, a, b, c);
}

double const GlpkLPSolver::GetValue(int index) const {
  return glp_get_col_prim(lp, index+2);
}

double const GlpkLPSolver::GetDual(int index) const {
  return glp_get_row_dual(lp, index+2);
}

void GlpkLPSolver::SaveLP(const char* file) {
  glp_write_lp(lp, nullptr, file);
}

void GlpkLPSolver::SetConstraintCoefForVariable(int constraint, int variable,
                                                 double new_utility) {
  int cols = glp_get_num_cols(lp);
  constraint+=2;
  variable+=2;
  int ind[cols+1];
  int count = glp_get_mat_row(lp, constraint, NULL, NULL);
  int ac[count+1];
  double ab[count+1];
  glp_get_mat_row(lp, constraint, ac, ab);
  vector<double> aB{ab, ab+count+1};
  vector<int> aC{ac, ac+count+1};
  for (int i = 1; i <= count; ++i) {
    ind[aC[i]] = 1;
    if (aC[i] == variable) {
      aB[i] = new_utility;
    }
  }

  for (int i = 1; i <= cols; ++i) {
    if (ind[i] != 1) {
      aC.push_back(i);
      aB.push_back(0);
      if (i == variable) {
        aB.back() = new_utility;
      }
    }
  }

  int* AC = &aC[0];
  double* AB = &aB[0];
  glp_set_mat_row(lp, constraint, cols, AC, AB);
}

void GlpkLPSolver::AddRows(int cols,
                           const vector<vector<double>>& utility_for_cols) {
  int new_rows = utility_for_cols.size();
  int rows = glp_get_num_rows(lp);
  ++cols;
  vector<double> ar = vector<double>(cols+1);
  auto ja = vector<int>(cols+1);

  glp_add_rows(lp, new_rows);

  for (int i=0; i < new_rows; ++i) {
    ar[1] = -1;
    ja[1] = 1;
    for (int j=0; j < cols-1; ++j) {
      ar[j+2] = utility_for_cols[i][j];
      ja[j+2] = j+2;
    }
    int* JA = &ja[0];
    double* AR = &ar[0];
    glp_set_row_bnds(lp, i+1+rows, GLP_LO, 0.0, 0.0);
    glp_set_mat_row(lp, i+1+rows, cols, JA, AR);
  }
}

void GlpkLPSolver::AddCols(int rows,
                           const vector<vector<double>>& utility_for_rows) {
  int new_cols = utility_for_rows.size();
  int cols = glp_get_num_cols(lp);
  ++rows;
  vector<double> ar = vector<double>(rows+1);
  auto ja = vector<int>(rows+1);

  glp_add_cols(lp, new_cols);

  for (int i=0; i < new_cols; ++i) {
    for (int j=0; j < rows-1; ++j) {
      ar[j+2] = utility_for_rows[i][j];
      ja[j+2] = j+2;
    }
    ar[1] = 1;
    ja[1] = 1;
    int* JA = &ja[0];
    double* AR = &ar[0];
    glp_set_col_bnds(lp, i+1+cols, GLP_DB, 0.0, 1.0);
    glp_set_mat_col(lp, i+1+cols, rows, JA, AR);
  }
}
