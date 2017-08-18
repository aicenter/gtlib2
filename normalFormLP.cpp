//
// Created by bbosansky on 8/11/17.
//

#include <vector>
#include <memory>
#include "normalFormLP.h"

NormalFormLP::NormalFormLP(const int _p1_actions, const int _p2_actions, const vector<double>& _utilities) {
    ValidateInput(_p1_actions, _p2_actions, _utilities);

    rows_ = _p2_actions;
    cols_ = _p1_actions;
    BuildModel(&_utilities);
}

NormalFormLP::NormalFormLP(const int _p1_actions, const int _p2_actions, const vector<vector<double>> &_utilities) {
    rows_ = _p2_actions;
    cols_ = _p1_actions;

    assert(rows_ == _utilities.size() && rows_ >= 0);
    assert(cols_ == _utilities[0].size() && cols_ >= 0);

    vector<double> tmp(rows_*cols_);

    for (int i=0; i < rows_; i++) {
        for (int j=0; j < cols_; j++) {
            tmp[i*cols_ + j] = _utilities[i][j];
        }
    }

    BuildModel(&tmp);
}

NormalFormLP::NormalFormLP(shared_ptr<Domain> _game) {
    vector<vector<int>> aohistories = {{}};
    throw(-1);  //TODO(bbosansky) implement, once State has an AO History
}

NormalFormLP::~NormalFormLP() {
    CleanModel();
}

void NormalFormLP::CleanModel() {
    model_ready_ = false;
    model_solved_ = false;
    value_of_the_game_ = NAN;
    env_.end();
}

double NormalFormLP::SolveGame() {
    if (!model_ready_)
        throw(-1);
    if ( !cplex_.solve() ) {
        env_.error() << "Failed to optimize LP" << std::endl;
        throw(-1);
    }

    value_of_the_game_ = cplex_.getObjValue();
    model_solved_ = true;
    return value_of_the_game_;
}

void NormalFormLP::BuildModel(const vector<double>* _utility_matrix) {
    assert(_utility_matrix != nullptr);

    env_ = IloEnv();
    model_ = IloModel(env_);

    x_ = IloNumVarArray(env_, cols_, 0, 1, ILOFLOAT);
    c_ = IloRangeArray(env_);
    IloNumVar V(env_, "V");

    model_.add(IloMaximize(env_, V));

    IloExpr prob(env_);
    for (int i=0; i < cols_; ++i) {
        prob += x_[i];
    }
    prob_ = IloRange(prob == 1);
    model_.add(prob_);

    for (int i=0; i < rows_; ++i) {
        IloExpr sum(env_);
        for (int j=0; j < cols_; ++j) {
            sum += _utility_matrix->at(i*cols_+j)*x_[j];
        }
        c_.add(IloRange(env_, 0, sum - V));
    }
    model_.add(c_);

    cplex_ = IloCplex(model_);
    if (!OUTPUT) {
        cplex_.setOut(env_.getNullStream());
    }

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
        for (int i=0; i < cols_; ++i) {
            result[i] = cplex_.getValue(x_[i]);
        }
        return std::make_shared<vector<double>>(result);
    } else {
        vector<double> result(rows_);
        for (int i=0; i < rows_; ++i) {
            result[i] = -cplex_.getDual(c_[i]);
        }
        return std::make_shared<vector<double>>(result);
    }
}

void NormalFormLP::AddActions(const int _player, const vector<vector<double>>& _utility_for_opponent) {
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

    vector<double> tmp(rows_*cols_);

    for (int i=0; i < rows_; i++) {
        for (int j=0; j < cols_; j++) {
            tmp[i*cols_ + j] = _utilities[i][j];
        }
    }

    CleanModel();
    BuildModel(&tmp);
}

bool NormalFormLP::ValidateInput(const int _p1_actions, const int _p2_actions, const vector<double>& _utilities) {
    if (!(_p1_actions >= 1 && _p2_actions >= 1 && _utilities.size() == _p1_actions*_p2_actions)) {
        throw("Illegal Argument in NormalFormLP");
    }
    return true;
}

void NormalFormLP::ChangeOutcome(const int _action_for_p1, const int _action_for_p2, double _new_utility) {
    if (!(_action_for_p1 >= 0 && _action_for_p1 < cols_&& _action_for_p2 >=0 && _action_for_p2 < rows_ )) {
        throw("Illegal Argument in NormalFormLP - Change Outcome");
    }

    if (!model_ready_) {
        throw(-1);
    }

    c_[_action_for_p2].setLinearCoef(x_[_action_for_p1], _new_utility);
    model_solved_ = false;
}

void NormalFormLP::SaveLP(const char* _file) {
    if (!model_ready_) {
        throw(-1);
    }
    cplex_.exportModel(_file);
}

void NormalFormLP::AddRows(const vector<vector<double>>& _utility_for_cols) {
    int new_rows = _utility_for_cols.size();
    if (new_rows == 0) return;
    assert(_utility_for_cols[0].size() == cols_);

    if (!model_ready_) {
        throw(-1);
    }

    IloExpr V = cplex_.getObjective().getExpr();

    for (int i=0; i < new_rows; ++i) {
        IloExpr sum(env_);
        for (int j=0; j < cols_; ++j) {
            sum += _utility_for_cols[i][j]*x_[j];
        }

        c_.add(IloRange(env_, 0, sum - V));
    }
    model_.add(c_);
    rows_ += new_rows;
}
void NormalFormLP::AddCols(const vector<vector<double>>& _utility_for_rows) {
    int new_cols = _utility_for_rows.size();
    if (new_cols == 0) return;
    assert(_utility_for_rows[0].size() == rows_);

    if (!model_ready_) {
        throw(-1);
    }

    IloNumVarArray new_x = IloNumVarArray(env_, new_cols, 0, 1, ILOFLOAT);

    for (int i=0; i < new_cols; i++) {
        prob_.setLinearCoef(new_x[i], 1);
    }

    for (int i=0; i < rows_; i++) {
        for (int j=0; j < new_cols; j++) {
            c_[i].setLinearCoef(new_x[j], _utility_for_rows[j][i]);
        }
    }
    x_.add(new_x);
    cols_ += new_cols;
}


