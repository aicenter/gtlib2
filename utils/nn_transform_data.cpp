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

#include "nn_transform_data.h"

using namespace GTLib2;

TransformCFRDataToNN::TransformCFRDataToNN(
        vector<unsigned int> ***information_set_mapping,
        vector<unsigned int> ***augmented_information_set_mapping,
        const vector<vector<double>> &features_public_state,
        const unsigned long num_possible_actions_sequences_p0, const unsigned long num_possible_actions_sequences_p1) {

    this->information_set_mapping = information_set_mapping;

    this->augmented_information_set_mapping = augmented_information_set_mapping;

    this->features_public_state = features_public_state;

    this->number_of_rounds = this->features_public_state.at(0).size();

    this->num_possible_actions_sequences_p0_ = num_possible_actions_sequences_p0;

    this->num_possible_actions_sequences_p1_ = num_possible_actions_sequences_p1;

    this->input_nn_matrix_dim_x =
            this->number_of_rounds + num_possible_actions_sequences_p0 + num_possible_actions_sequences_p1;
    this->input_nn_matrix_dim_y = features_public_state.size();

    this->output_nn_matrix_dim_x = num_possible_actions_sequences_p0 + num_possible_actions_sequences_p1;
    this->output_nn_matrix_dim_y = features_public_state.size();

}

bool TransformCFRDataToNN::is_reachable(double range_probability) {
    // equals to !(fabs(range_probability) < 1e-6) <=> (range_probability != 0) for doubles
    return fabs(range_probability) > EPSILON;
}

vector<vector<double>> TransformCFRDataToNN::to_nn_input(const vector<double> &range_acting_player,
                                                         const vector<double> &range_opponent_player) {
    vector<vector<double>> returning_matrix;

    // init returning matrix by 0's
    for (int y = 0; y < this->input_nn_matrix_dim_y; y++) {
        vector<double> row_vector(this->input_nn_matrix_dim_x, 0);

        returning_matrix.push_back(row_vector);
    }

    // fill public states into the returning matrix
    for (int y = 0; y < this->input_nn_matrix_dim_y; y++) {
        vector<double> feature_public_state = this->features_public_state.at(y);

        for (int x = 0; x < this->number_of_rounds; x++) {
            returning_matrix.at(y).at(x) = feature_public_state.at(x);
        }
    }
    // fill the returning matrix - acting player
    for (int y = 0; y < this->input_nn_matrix_dim_y; y++) {
        for (int x = 0; x < this->num_possible_actions_sequences_p0_; x++) {
            if (information_set_mapping[y][x]) {
                unsigned long z_dimension = information_set_mapping[y][x]->size();

                for (int z = 0; z < z_dimension; z++) {
                    int index = information_set_mapping[y][x]->at(z);

                    if (this->is_reachable(range_acting_player[index])) {
                        returning_matrix.at(y).at(x + this->number_of_rounds) = range_acting_player[index];
                        break;
                    }
                }
            }
        }
    }

    for (int y = 0; y < this->input_nn_matrix_dim_y; y++) {
        for (int x = 0; x < this->num_possible_actions_sequences_p1_; x++) {
            if (augmented_information_set_mapping[y][x]) {
                unsigned long z_dimension = augmented_information_set_mapping[y][x]->size();

                for (int z = 0; z < z_dimension; z++) {
                    int index = augmented_information_set_mapping[y][x]->at(z);

                    if (this->is_reachable(range_opponent_player[index])) {
                        returning_matrix.at(y).at(x + this->number_of_rounds +
                                                  this->num_possible_actions_sequences_p1_) = range_opponent_player[index];
                        break;
                    }
                }
            }
        }
    }

    return returning_matrix;
}

vector<double> TransformCFRDataToNN::from_nn_output(const vector<double> &range_acting_player,
                                                    const vector<double> &range_opponent_player,
                                                    const vector<vector<double>> &nn_output_matrix,
                                                    Player updatingPlayer) {
    vector<double> returning_vector(range_acting_player.size(), 0); // init with zeros

    if (updatingPlayer == Player(0)) {
        for (int y = 0; y < this->output_nn_matrix_dim_y; y++) {
            for (int x = 0; x < this->num_possible_actions_sequences_p0_; x++) {
                if (this->information_set_mapping[y][x]) {
                    unsigned long z_dimension = information_set_mapping[y][x]->size();

                    for (int z = 0; z < z_dimension; z++) {
                        int index = information_set_mapping[y][x]->at(z);

                        if (this->is_reachable(range_opponent_player[index])) {
                            returning_vector.at(index) = nn_output_matrix[y][x];
                            break;
                        }
                    }

                }
            }
        }
    } else if (updatingPlayer == Player(1)) {
        for (int y = 0; y < this->output_nn_matrix_dim_y; y++) {
            for (int x = 0; x < this->num_possible_actions_sequences_p1_; x++) {
                if (this->augmented_information_set_mapping[y][x]) {
                    unsigned long z_dimension = augmented_information_set_mapping[y][x]->size();

                    for (int z = 0; z < z_dimension; z++) {
                        int index = augmented_information_set_mapping[y][x]->at(z);

                        if (this->is_reachable(range_acting_player[index])) {
                            returning_vector.at(index) = nn_output_matrix[y][num_possible_actions_sequences_p1_ + x];
                            break;
                        }
                    }

                }
            }
        }
    }

    return returning_vector;
}