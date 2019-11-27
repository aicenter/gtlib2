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

    You should have received a copy of the GNU Lesser General Public
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GTLIB2_NN_TRANSFORM_DATA_H
#define GTLIB2_NN_TRANSFORM_DATA_H
#define EPSILON numeric_limits<double>::epsilon()

#include <cmath>
#include <vector>
#include "base/base.h"
#include <limits>

using namespace GTLib2;

using namespace std;

class TransformCFRDataToNN {
protected:
    bool is_reachable(double range_probability);

public:
    long input_nn_matrix_dim_x = 0, input_nn_matrix_dim_y = 0;
    long output_nn_matrix_dim_x = 0, output_nn_matrix_dim_y = 0;

    long number_of_rounds = 0;

    vector<unsigned int> ***information_set_mapping;
    vector<unsigned int> ***augmented_information_set_mapping;

    vector<vector<double>> features_public_state;

    unsigned long num_possible_actions_sequences_p0_;

    unsigned long num_possible_actions_sequences_p1_;

    explicit TransformCFRDataToNN(
            vector<unsigned int> ***information_set_mapping,
            vector<unsigned int> ***augmented_information_set_mapping,
            const vector<vector<double>> &features_public_state,
            unsigned long num_possible_actions_sequences_p0, unsigned long num_possible_actions_sequences_p1);

    inline explicit TransformCFRDataToNN() = default;

    vector<vector<double>>
    to_nn_input(const vector<double> &range_acting_player, const vector<double> &range_opponent_player);

    vector<double>
    from_nn_output(const vector<double> &range_acting_player, const vector<double> &range_opponent_player,
                   const vector<vector<double>> &nn_output_matrix, Player updatingPlayer);

};

#endif //GTLIB2_NN_TRANSFORM_DATA_H
