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


#ifndef ALGORITHMS_UTILITY_H_
#define ALGORITHMS_UTILITY_H_

#include "base/base.h"

namespace GTLib2::algorithms {

vector<double> computeUtilitiesTwoPlayerGame(const Domain &domain,
                                             const StrategyProfile &stratProfile);


struct UtilityMatrix {
    vector<double> u;
    unsigned long rows;
    unsigned long cols;
};

UtilityMatrix constructUtilityMatrixFor(
    const Domain &domain, const Player player,
    const array<vector <BehavioralStrategy>, 2> &playersPureStrats);

unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>
createInfosetsAndActions(const Domain &domain, const Player player);

vector <BehavioralStrategy> generateAllPureStrategies(
    const unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>> &infSetsAndActions);

}  // namespace GTLib2

#endif  // ALGORITHMS_UTILITY_H_

