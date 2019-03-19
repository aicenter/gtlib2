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


#include <chrono>
#if GUROBIFOUND == 1
#include "LPsolvers/GurobiLPSolver.h"
#elif CPLEXFOUND == 1
#include "LPsolvers/CplexLPSolver.h"
#elif GLPKFOUND == 1
#include "LPsolvers/GlpkLPSolver.h"
#endif
#include "base/efg.h"
#include "algorithms/utility.h"
#include "algorithms/cfr.h"
#include "algorithms/common.h"
#include "algorithms/normalFormLP.h"
#include "algorithms/treeWalk.h"
#include "algorithms/bestResponse.h"
#include "algorithms/equilibrium.h"
#include "domains/goofSpiel.h"
#include "domains/phantomTTT.h"
#include "domains/pursuit.h"
#include "domains/matching_pennies.h"
#include "domains/RhodeIslandPoker.h"
#include "domains/genericPoker.h"


using std::endl;
using std::cout;
using namespace GTLib2;

void bestRespAndEqui() {
    domains::IIGoofSpielDomain domain(6, 1, 1);
    int player1 = domain.getPlayers()[0];
    int player2 = domain.getPlayers()[1];
    auto player1InfSetsAndActions =
        algorithms::generateInformationSetsAndAvailableActions(domain, player1);
    cout << player1InfSetsAndActions.size() << "\n";
    auto player2InfSetsAndActions =
        algorithms::generateInformationSetsAndAvailableActions(domain, player2);
    cout << player2InfSetsAndActions.size() << "\n";
    auto player1PureStrats = algorithms::generateAllPureStrategies(player1InfSetsAndActions);
    cout << player1InfSetsAndActions.size() << "  " << player1PureStrats.size() << "\n";
    auto player2PureStrats = algorithms::generateAllPureStrategies(player2InfSetsAndActions);
    cout << player2InfSetsAndActions.size() << "  " << player2PureStrats.size() << "\n";
//  for (auto &i : player2PureStrats) {
//    auto x = algorithms::bestResponseToPrunning(i, player2, player1, domain);
//    cout << "Value is: " << x.second << "\n";
//  }
    auto vysl = algorithms::findEquilibriumTwoPlayersZeroSum(domain);
//  cout << std::get<0>(vysl);
//  cout << "\n";
//  for (auto &j : std::get<1>(vysl)) {
//    auto aoh = std::dynamic_pointer_cast<AOH>(j.first);
//    for (auto &x : aoh->getAOHistory()) {
//      cout << x.first << " " << x.second << "   ";
//    }
//    for (auto &k : j.second) {
//      cout << k.first->toString() << ", prob: " << k.second << "  |  ";
//    }
//    cout << "\n";
//  }
}

int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    auto domain = domains::GenericPokerDomain(5, 3, 1, 2, 2);
//  auto domain2 = domains::PursuitDomain(2, 2, 2);
//  cout << algorithms::countNodes(domain2) <<"\n";
//  auto domain2 = domains::GoofSpielDomain(4, 4, nullopt);
//  cout << domain2.getMaxUtility() <<"\n";
//  domains::GoofSpielDomain domain2(3,3, nullopt);
    auto regrets = algorithms::CFRiterations(domain, 20);
    auto strat1 = algorithms::getStrategyFor(domain, domain.getPlayers()[0], regrets);
    auto strat2 = algorithms::getStrategyFor(domain, domain.getPlayers()[1], regrets);
    cout << algorithms::bestResponseToPrunning(strat1, domain.getPlayers()[0],
                                               domain.getPlayers()[1], domain).second << "\n";
    cout << algorithms::bestResponseToPrunning(strat2, domain.getPlayers()[1],
                                               domain.getPlayers()[0], domain).second << "\n";
//  cout << algorithms::computeUtilityTwoPlayersGame(domain, strat1, strat2, 0, 1).first <<"\n";
//  bestRespAndEqui();
    auto end = std::chrono::high_resolution_clock::now();
    using ms = std::chrono::duration<int, std::milli>;
    cout << "Time " << std::chrono::duration_cast<ms>(end - start).count() << " ms" << '\n';
    return 0;
}
