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

#include "algorithms/normalFormLP.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::algorithms {

NormalFormLP::NormalFormLP(const unsigned int p1_actions, const unsigned int p2_actions,
                           const vector<double> &utilities,
                           unique_ptr<AbstractLPSolver> lp_solver) {
    ValidateInput(p1_actions, p2_actions, utilities);
    lp_solver_ = move(lp_solver);
    rows_ = p2_actions;
    cols_ = p1_actions;
    BuildModel(&utilities);
}

NormalFormLP::NormalFormLP(const unsigned int p1_actions, const unsigned int p2_actions,
                           const vector<vector<double>> &utilities,
                           unique_ptr<AbstractLPSolver> lp_solver) {
    lp_solver_ = move(lp_solver);
    rows_ = p2_actions;
    cols_ = p1_actions;

    assert(rows_ == utilities.size() && rows_ >= 0);
    assert(cols_ == utilities[0].size() && cols_ >= 0);

    vector<double> tmp(rows_ * cols_);

    for (int i = 0; i < rows_; i++) {
        for (int j = 0; j < cols_; j++) {
            tmp[i * cols_ + j] = utilities[i][j];
        }
    }
    BuildModel(&tmp);
}

NormalFormLP::NormalFormLP(const Domain &domain, unique_ptr<AbstractLPSolver> _lp_solver) {
    lp_solver_ = move(_lp_solver);

    const array<vector<BehavioralStrategy>, 2> pureStrats = {
        generateAllPureStrategies(createInfosetsAndActions(domain, Player(0))),
        generateAllPureStrategies(createInfosetsAndActions(domain, Player(1)))
    };
    const auto utils = constructUtilityMatrixFor(domain, Player(0), pureStrats);
    rows_ = utils.rows;
    cols_ = utils.cols;
    BuildModel(&utils.u);
}

NormalFormLP::~NormalFormLP() {
    CleanModel();
}

void NormalFormLP::CleanModel() {
    model_ready_ = false;
    model_solved_ = false;
    value_of_the_game_ = NAN;
    lp_solver_->CleanModel();
}

double NormalFormLP::SolveGame() {
    if (!model_ready_) throw (-1);

    value_of_the_game_ = lp_solver_->SolveGame();
    model_solved_ = true;
    return value_of_the_game_;
}

void NormalFormLP::BuildModel(const vector<double> *_utility_matrix) {
    assert(_utility_matrix != nullptr);
    lp_solver_->BuildModel(rows_, cols_, _utility_matrix, OUTPUT);
    model_ready_ = true;
}

vector<double> NormalFormLP::GetStrategy(int player) {
    assert(player == 0 || player == 1);
    vector<double> result;
    if (!model_solved_ && SolveGame() == NAN) {
        return result;
    }

    if (player == 0) {
        result.reserve(cols_);
        for (int i = 0; i < cols_; ++i) {
            result.emplace_back(lp_solver_->GetValue(i));
        }
    } else {
        result.reserve(rows_);
        for (int i = 0; i < rows_; ++i) {
            result.emplace_back(-lp_solver_->GetDual(i));
        }
    }
    return result;
}

void NormalFormLP::AddActions(const int player,
                              const vector<vector<double>> &utility_for_opponent) {
    if (player == 0) {
        AddCols(utility_for_opponent);
    } else if (player == 1) {
        AddRows(utility_for_opponent);
    } else {
        throw (-1);
    }

    model_ready_ = true;
    model_solved_ = false;
}

void NormalFormLP::UpdateUtilityMatrix(const vector<double> &utilities) {
    ValidateInput(rows_, cols_, utilities);
    CleanModel();
    BuildModel(&utilities);
}

void NormalFormLP::UpdateUtilityMatrix(const vector<vector<double>> &utilities) {
    assert(rows_ == utilities.size() && rows_ >= 0);
    assert(cols_ == utilities[0].size() && cols_ >= 0);

    vector<double> tmp(rows_ * cols_);

    for (int i = 0; i < rows_; i++) {
        for (int j = 0; j < cols_; j++) {
            tmp[i * cols_ + j] = utilities[i][j];
        }
    }

    CleanModel();
    BuildModel(&tmp);
}

bool NormalFormLP::ValidateInput(const int p1_actions, const int p2_actions,
                                 const vector<double> &utilities) {
    if (!(p1_actions >= 1 && p2_actions >= 1
        && utilities.size() == p1_actions * p2_actions)) {
        throw ("Illegal Argument in NormalFormLP");
    }
    return true;
}

void NormalFormLP::ChangeOutcome(const int action_for_p1,
                                 const int action_for_p2,
                                 double new_utility) {
    if (!(action_for_p1 >= 0 && action_for_p1 < cols_
        && action_for_p2 >= 0 && action_for_p2 < rows_)) {
        throw ("Illegal Argument in NormalFormLP - Change Outcome");
    }

    if (!model_ready_) throw (-1);
    // constrain. variable
    lp_solver_->SetConstraintCoefForVariable(action_for_p2, action_for_p1, new_utility);
    model_solved_ = false;
}

void NormalFormLP::SaveLP(const char *_file) {
    if (!model_ready_) throw (-1);
    lp_solver_->SaveLP(_file);
}

void NormalFormLP::AddRows(const vector<vector<double>> &_utility_for_cols) {
    auto new_rows = static_cast<unsigned int>(_utility_for_cols.size());
    if (new_rows == 0) return;
    assert(_utility_for_cols[0].size() == cols_);

    if (!model_ready_) throw (-1);
    lp_solver_->AddRows(cols_, _utility_for_cols);

    rows_ += new_rows;
}

void NormalFormLP::AddCols(const vector<vector<double>> &_utility_for_rows) {
    auto new_cols = static_cast<unsigned int>(_utility_for_rows.size());
    if (new_cols == 0) return;
    assert(_utility_for_rows[0].size() == rows_);

    if (!model_ready_) throw (-1);

    lp_solver_->AddCols(rows_, _utility_for_rows);
    cols_ += new_cols;
}
}  // namespace GTLib2

#pragma clang diagnostic pop
