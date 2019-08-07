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


#ifndef LPSOLVERS_GLPKLPSOLVER_H_
#define LPSOLVERS_GLPKLPSOLVER_H_

#include "LPsolvers/AbstractLPSolver.h"
#include <glpk.h>

namespace GTLib2::algorithms {


class GlpkLPSolver: public AbstractLPSolver {
 public:
    GlpkLPSolver() = default;

    ~GlpkLPSolver() override = default;

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
    glp_prob *lp = nullptr;
};

inline double solveLP(const unsigned int rows,
                      const unsigned int cols,
                      const vector<double> &utility_matrix,
                      vector<double> &solution) {
    // Matrix's rows are indexed by player1. Cols are indexed by player2
    // Utility matrix is for player1. It's zero sum, so utility matrix of player2 is -utility_matrix

    try {
        const unsigned int new_cols = cols + 1;
        const unsigned int new_rows = rows + 1;
        vector<int> ia = vector<int>(1 + new_cols * new_rows);
        vector<int> ja = vector<int>(1 + new_cols * new_rows);
        vector<double> ar = vector<double>(1 + new_cols * new_rows);
        glp_prob *lp = glp_create_prob();
        glp_set_obj_dir(lp, GLP_MAX);
        glp_add_rows(lp, new_rows);
        glp_add_cols(lp, new_cols);

        for (int i = 2; i <= new_rows; ++i) {
            glp_set_row_bnds(lp, i, GLP_LO, 0.0, 0.0);
        }

        for (int i = 1; i <= new_cols; ++i) {
            glp_set_col_bnds(lp, i, GLP_DB, 0.0, 1.0);
        }
        glp_set_obj_coef(lp, 1, 1);
        glp_set_col_bnds(lp, 1, GLP_FR, 0.0, 1.0);

        for (int i = 1; i <= new_cols; ++i) {
            ar[i] = 1;
            ja[i] = i;
            ia[i] = 1;
        }
        ar[1] = 0;
        for (int i = new_cols + 1, j = 0, k = -1; i <= new_cols * new_rows; ++i, ++j) {
            if ((i - 1) % new_cols == 0) {
                ar[i] = -1;
                j = -1;
                ++k;
            } else {
                ar[i] = utility_matrix[j * cols + k];
            }
            ia[i] = (i - 1) / new_cols + 1;
            ja[i] = (i - 1) % new_cols + 1;
        }
        glp_set_row_bnds(lp, 1, GLP_FX, 1.0, 1.0);

        int *a = &ia[0];
        int *b = &ja[0];
        double *c = &ar[0];

        glp_load_matrix(lp, new_cols * new_rows, a, b, c);

        if (glp_simplex(lp, nullptr)) {
            glp_error("Failed to optimize LP");
            throw (-1);
        }
        cout << "Solution status = " << glp_get_status(lp) << "\n";
        cout << "Solution value = " << glp_get_obj_val(lp) << "\n";

        for (int i = 0; i < cols; ++i) {
            solution[i] = glp_get_col_prim(lp, i + 2);
        }
        glp_write_lp(lp, nullptr, "e.lp");
        return glp_get_obj_val(lp);
    } catch (glp_errfunc &e) {
        LOG_ERROR("GLPK exception caught: " << e)
        return NAN;
    }
}

}
#endif  // LPSOLVERS_GLPKLPSOLVER_H_
