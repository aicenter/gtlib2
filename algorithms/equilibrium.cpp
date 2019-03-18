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

#include <vector>
#include "algorithms/equilibrium.h"
#include "algorithms/utility.h"
#include "algorithms/common.h"
#if GUROBIFOUND == 1
#include "LPsolvers/GurobiLPSolver.h"
#elif CPLEXFOUND == 1
#include "LPsolvers/CplexLPSolver.h"
#elif GLPKFOUND == 1
#include "LPsolvers/GlpkLPSolver.h"
#else
#define SOLVERNOTFOUND 1
#include "LPsolvers/LPSolver.h"
#endif


namespace GTLib2 {
namespace algorithms {
tuple<double, BehavioralStrategy> findEquilibriumTwoPlayersZeroSum(const GTLib2::Domain &domain) {
  int player1 = domain.getPlayers()[0];
  int player2 = domain.getPlayers()[1];
  auto player1InfSetsAndActions = generateInformationSetsAndAvailableActions(domain, player1);
  auto player2InfSetsAndActions = generateInformationSetsAndAvailableActions(domain, player2);
  auto player1PureStrats = generateAllPureStrategies(player1InfSetsAndActions);
  auto player2PureStrats = generateAllPureStrategies(player2InfSetsAndActions);
  auto utilityMatrixPlayer1 =
      constructUtilityMatrixFor(domain, player1, player1PureStrats, player2PureStrats);
  // Solution is a probability distribution over player1PureStrats
  vector<double> solution(std::get<2>(utilityMatrixPlayer1), 0.0);

  double val = solveLP(std::get<1>(utilityMatrixPlayer1), std::get<2>(utilityMatrixPlayer1),
                       std::get<0>(utilityMatrixPlayer1), solution);
  auto equilibriumStrat = mixedToBehavioralStrategy(player1PureStrats, player1, solution,
                                                    domain);
  return tuple<double, BehavioralStrategy>(val, equilibriumStrat);
}
}  // namespace algorithms
}  // namespace GTLib2
#pragma clang diagnostic pop
