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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef ALGORITHMS_UTILITY_H_
#define ALGORITHMS_UTILITY_H_

#include <tuple>
#include <utility>
#include <vector>
#include "base/base.h"

using std::tuple;

namespace GTLib2 {
namespace algorithms {
pair<double, double> computeUtilityTwoPlayersGame(const Domain &domain,
                                                  const BehavioralStrategy &player1Strat,
                                                  const BehavioralStrategy &player2Strat,
                                                  const Player player1,
                                                  const Player player2);

tuple<vector<double>, unsigned int, unsigned int>
constructUtilityMatrixFor(const Domain &domain,
                          int player,
                          const vector<BehavioralStrategy> &player1PureStrats,
                          const vector<BehavioralStrategy> &player2PureStrats);

unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>
generateInformationSetsAndAvailableActions(const Domain &domain,
                                           int player);

vector<BehavioralStrategy>
generateAllPureStrategies(const unordered_map<shared_ptr<AOH>,
                                              vector<shared_ptr<Action>>> &infSetsAndActions);

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_UTILITY_H_

#pragma clang diagnostic pop
