//
// Created by bbosansky on 8/11/17.
//

#include "algorithms/normalFormLP.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {
namespace algorithms {
NormalFormLP::NormalFormLP(const unsigned int _p1_actions, const unsigned int _p2_actions,
                           const vector<double> &_utilities,
                           unique_ptr<LPSolver> _lp_solver) {
  ValidateInput(_p1_actions, _p2_actions, _utilities);
  lp_solver = std::move(_lp_solver);
  rows_ = _p2_actions;
  cols_ = _p1_actions;
  BuildModel(&_utilities);
}

NormalFormLP::NormalFormLP(const unsigned int _p1_actions, const unsigned int _p2_actions,
                           const vector<vector<double>> &_utilities,
                           unique_ptr<LPSolver> _lp_solver) {
  lp_solver = std::move(_lp_solver);
  rows_ = _p2_actions;
  cols_ = _p1_actions;

  assert(rows_ == _utilities.size() && rows_ >= 0);
  assert(cols_ == _utilities[0].size() && cols_ >= 0);

  vector<double> tmp(rows_ * cols_);

  for (int i = 0; i < rows_; i++) {
    for (int j = 0; j < cols_; j++) {
      tmp[i * cols_ + j] = _utilities[i][j];
    }
  }
  BuildModel(&tmp);
}

NormalFormLP::NormalFormLP(const shared_ptr<Domain> _game,
                           unique_ptr<LPSolver> _lp_solver) {
  lp_solver = std::move(_lp_solver);

  int player1 = _game->getPlayers()[0];
  int player2 = _game->getPlayers()[1];
  auto player1InfSetsAndActions = generateInformationSetsAndAvailableActions(*_game, player1);
  auto player2InfSetsAndActions = generateInformationSetsAndAvailableActions(*_game, player2);
  auto player1PureStrats = generateAllPureStrategies(player1InfSetsAndActions);
  auto player2PureStrats = generateAllPureStrategies(player2InfSetsAndActions);
  auto utilityMatrixPlayer1 =
      constructUtilityMatrixFor(*_game, player1, player1PureStrats, player2PureStrats);
  rows_ = std::get<1>(utilityMatrixPlayer1);
  cols_ = std::get<2>(utilityMatrixPlayer1);
  BuildModel(&std::get<0>(utilityMatrixPlayer1));
}

NormalFormLP::~NormalFormLP() {
  CleanModel();
}

void NormalFormLP::CleanModel() {
  model_ready_ = false;
  model_solved_ = false;
  value_of_the_game_ = NAN;
  lp_solver->CleanModel();
}

double NormalFormLP::SolveGame() {
  if (!model_ready_)
    throw(-1);

  value_of_the_game_ = lp_solver->SolveGame();
  model_solved_ = true;
  return value_of_the_game_;
}

void NormalFormLP::BuildModel(const vector<double> *_utility_matrix) {
  assert(_utility_matrix != nullptr);
  lp_solver->BuildModel(rows_, cols_, _utility_matrix, OUTPUT);

  model_ready_ = true;
}

shared_ptr<vector<double>> NormalFormLP::GetStrategy(int _player) {
  assert(_player == 0 || _player == 1);

  if (!model_solved_) {
    if (SolveGame() == NAN) {
      return nullptr;
    }
  }

  if (_player == 0) {
    vector<double> result(cols_);
    for (int i = 0; i < cols_; ++i) {
      result[i] = lp_solver->GetValue(i);
    }
    return make_shared<vector<double>>(result);
  } else {
    vector<double> result(rows_);
    for (int i = 0; i < rows_; ++i) {
      result[i] = -lp_solver->GetDual(i);
    }
    return make_shared<vector<double>>(result);
  }
}

void NormalFormLP::AddActions(const int _player,
                              const vector<vector<double>> &_utility_for_opponent) {
  if (_player == 0) {
    AddCols(_utility_for_opponent);
  } else if (_player == 1) {
    AddRows(_utility_for_opponent);
  } else {
    throw(-1);
  }

  model_ready_ = true;
  model_solved_ = false;
}

void NormalFormLP::UpdateUtilityMatrix(const vector<double> &_utilities) {
  ValidateInput(rows_, cols_, _utilities);
  CleanModel();
  BuildModel(&_utilities);
}

void NormalFormLP::UpdateUtilityMatrix(const vector<vector<double>> &_utilities) {
  assert(rows_ == _utilities.size() && rows_ >= 0);
  assert(cols_ == _utilities[0].size() && cols_ >= 0);

  vector<double> tmp(rows_ * cols_);

  for (int i = 0; i < rows_; i++) {
    for (int j = 0; j < cols_; j++) {
      tmp[i * cols_ + j] = _utilities[i][j];
    }
  }

  CleanModel();
  BuildModel(&tmp);
}

bool NormalFormLP::ValidateInput(const int _p1_actions, const int _p2_actions,
                                 const vector<double> &_utilities) {
  if (!(_p1_actions >= 1 && _p2_actions >= 1
      && _utilities.size() == _p1_actions * _p2_actions)) {
    throw("Illegal Argument in NormalFormLP");
  }
  return true;
}

void NormalFormLP::ChangeOutcome(const int _action_for_p1,
                                 const int _action_for_p2,
                                 double _new_utility) {
  if (!(_action_for_p1 >= 0 && _action_for_p1 < cols_ && _action_for_p2 >= 0
      && _action_for_p2 < rows_)) {
    throw("Illegal Argument in NormalFormLP - Change Outcome");
  }

  if (!model_ready_) {
    throw(-1);
  }
  // constrain. variable
  lp_solver->SetConstraintCoefForVariable(_action_for_p2,
                                          _action_for_p1, _new_utility);
  model_solved_ = false;
}

void NormalFormLP::SaveLP(const char *_file) {
  if (!model_ready_) {
    throw(-1);
  }
  lp_solver->SaveLP(_file);
}

void NormalFormLP::AddRows(const vector<vector<double>> &_utility_for_cols) {
  auto new_rows = static_cast<unsigned int>(_utility_for_cols.size());
  if (new_rows == 0) return;
  assert(_utility_for_cols[0].size() == cols_);

  if (!model_ready_) {
    throw(-1);
  }

  lp_solver->AddRows(cols_, _utility_for_cols);

  rows_ += new_rows;
}

void NormalFormLP::AddCols(const vector<vector<double>> &_utility_for_rows) {
  auto new_cols = static_cast<unsigned int>(_utility_for_rows.size());
  if (new_cols == 0) return;
  assert(_utility_for_rows[0].size() == rows_);

  if (!model_ready_) {
    throw(-1);
  }

  lp_solver->AddCols(rows_, _utility_for_rows);

  cols_ += new_cols;
}
}  // namespace algorithms
}  // namespace GTLib2

#pragma clang diagnostic pop
