//
// Created by Jakub Rozlivek on 02.08.2017.
//

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

// #include <boost/test/unit_test.hpp>

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

void goofSpiel5() {
  // domains::GoofSpielDomain domain(5, 2);
  domains::GenericPokerDomain domain;
  // PhantomTTTDomain domain(5);
  // PursuitDomain domain(4,3,3);
  int numberOfNodes = algorithms::countNodesInfSetsSequencesStates(domain);
  cout << "Number of nodes: " << numberOfNodes << endl;
}

void goofSpiel() {
  domains::GoofSpielDomain gsd(4, 4, nullopt);

  int player1 = gsd.getPlayers()[0];
  int player2 = gsd.getPlayers()[1];

  BehavioralStrategy player2Strat;

  auto lowestCardAction = make_shared<domains::GoofSpielAction>(1, 1);
  auto secondLowestCardAction = make_shared<domains::GoofSpielAction>(2, 2);
  auto thirdLowestCardAction = make_shared<domains::GoofSpielAction>(3, 3);
  auto fourthLowestCardAction = make_shared<domains::GoofSpielAction>(4, 4);

  auto setAction = [&player2Strat, &lowestCardAction, &secondLowestCardAction,
      &thirdLowestCardAction, &fourthLowestCardAction](shared_ptr<EFGNode> node) {
    if (node->getDistanceFromRoot() == 0) {
      player2Strat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
    } else if (node->getDistanceFromRoot() == 2) {
      player2Strat[node->getAOHInfSet()] = {{secondLowestCardAction, 1.0}};
    } else if (node->getDistanceFromRoot() == 4) {
      player2Strat[node->getAOHInfSet()] = {{thirdLowestCardAction, 1.0}};
    } else if (node->getDistanceFromRoot() == 6) {
      player2Strat[node->getAOHInfSet()] = {{fourthLowestCardAction, 1.0}};
    }
  };

  algorithms::treeWalkEFG(gsd, setAction);

  auto player1BestResponse = algorithms::bestResponseTo(player2Strat, player2, player1, gsd);

  cout << "Value of the best response: " << player1BestResponse.second << endl;

  auto val = algorithms::computeUtilityTwoPlayersGame(gsd, player2Strat,
                                                      player1BestResponse.first, player2, player1);

  cout << "val" << val.first << endl;
}

void goofSpiel2() {
  domains::GoofSpielDomain gsd(3, nullopt);

  int player1 = gsd.getPlayers()[0];
  int player2 = gsd.getPlayers()[1];
  auto p1InfSetsAndActions = algorithms::generateInformationSetsAndAvailableActions(gsd, player1);
  auto p2InfSetsAndActions = algorithms::generateInformationSetsAndAvailableActions(gsd, player2);

  cout << "Goof spiel number of infSets of p1: " << p1InfSetsAndActions.size() << endl;
  cout << "Goof spiel number of infSets of p2: " << p2InfSetsAndActions.size() << endl;
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
      domain.getPlayers()[1], domain).second <<"\n";
  cout << algorithms::bestResponseToPrunning(strat2, domain.getPlayers()[1],
      domain.getPlayers()[0], domain).second <<"\n";
//  cout << algorithms::computeUtilityTwoPlayersGame(domain, strat1, strat2, 0, 1).first <<"\n";
//  bestRespAndEqui();
  auto end = std::chrono::high_resolution_clock::now();
  using ms = std::chrono::duration<int, std::milli>;
  cout << "Time " << std::chrono::duration_cast<ms>(end - start).count() << " ms" << '\n';
  return 0;
}
