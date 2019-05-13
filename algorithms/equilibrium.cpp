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

#include "base/base.h"
#include "algorithms/equilibrium.h"

#include <vector>

#include "algorithms/utility.h"
#include "algorithms/common.h"
#include "LPsolvers/LPSolver.h"


namespace GTLib2::algorithms {

StrategyValue findEquilibriumTwoPlayersZeroSum(const Domain &domain) {
    const array<vector<BehavioralStrategy>, 2> pureStrats = {
        generateAllPureStrategies(createInfosetsAndActions(domain, Player(0))),
        generateAllPureStrategies(createInfosetsAndActions(domain, Player(1)))
    };
    const auto utils = constructUtilityMatrixFor(domain, Player(0), pureStrats);

    // Solution is a probability distribution over player1PureStrats
    vector<double> solution = utils.u;
    auto value = solveLP(utils.rows, utils.cols, utils.u, solution);
    auto equilibriumStrat = mixedToBehavioralStrategy(domain, pureStrats[0], solution, Player(0));
    return StrategyValue(equilibriumStrat, value);
}

}  // namespace GTLib2
#pragma clang diagnostic pop
