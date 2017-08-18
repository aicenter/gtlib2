//
// Created by bbosansky on 8/10/17.
//

#ifndef NORMALFORMLP_H_
#define NORMALFORMLP_H_

#define IL_STD

#include <bits/shared_ptr.h>
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <vector>
#include <memory>
#include "base.h"

using std::vector;
using std::shared_ptr;

/**
 * This class represents the algorithm for solving two-player zero-sum game using a linear program (LP) and normal form.
 * The class requires number of actions for both players and a reference to the utility matrix.
 * The class does not hold the copy of the utility matrix and directly builds the LP.
 * The class also rebuilds the LP whenever there is a change to the utility matrix.
 */
class NormalFormLP {
 public:
    explicit NormalFormLP(shared_ptr<Domain> _game);
    explicit NormalFormLP(const int _p1_actions, const int _p2_actions, const vector<double>& _utilities);
    explicit NormalFormLP(const int _p1_actions, const int _p2_actions, const vector<vector<double>>& _utilities);

    virtual ~NormalFormLP();

    double SolveGame();

    void AddActions(const int _player, const vector<vector<double>>& _utility_for_opponent);
    void ChangeOutcome(const int _action_for_p1, const int _action_for_p2, double _new_utility);

    shared_ptr<vector<double>> GetStrategy(int _player);

    void UpdateUtilityMatrix(const vector<double> &_utilities);
    void UpdateUtilityMatrix(const vector<vector<double>> &_utilities);
    void SaveLP(const char* _file);


 protected:
    int rows_;
    int cols_;
    const bool OUTPUT = true;

    IloEnv env_;
    IloModel model_;
    IloCplex cplex_;

    IloNumVarArray x_;
    IloRangeArray c_;
    IloRange prob_;

    double value_of_the_game_ = NAN;

    bool model_ready_ = false;
    bool model_solved_ = false;

    void CleanModel();
    void BuildModel(const vector<double>* _utility_matrix);
    bool ValidateInput(const int _p1_actions, const int _p2_actions, const vector<double>& _utilities);
    void AddRows(const vector<vector<double>>& _utility_for_cols);
    void AddCols(const vector<vector<double>>& _utility_for_rows);
};


#endif  // NORMALFORMLP_H_
