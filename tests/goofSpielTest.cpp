//
// Created by Pavel Rytir on 30/01/2018.
//


#include "../algorithms/treeWalk.h"
#include "../algorithms/bestResponse.h"
#include "../algorithms/common.h"
#include "../algorithms/equilibrium.h"
#include "../domains/goofSpiel.h"
#include "../algorithms/utility.h"

#define BOOST_TEST_DYN_LINK  // For linking with dynamic libraries.

#include <boost/test/unit_test.hpp>

using namespace GTLib2;

BOOST_AUTO_TEST_SUITE(GoofSpiel)

BOOST_AUTO_TEST_CASE(computeUtilityFullDepthCard4) {
  domains::GoofSpielDomain gsd(4, 4, nullopt);

  int player1 = gsd.getPlayers()[1];
  int player2 = gsd.getPlayers()[0];


  // Create strategy that plays the lowest card
  BehavioralStrategy player2Strat;
  auto lowestCardAction = make_shared<domains::GoofSpielAction>(0, 1);
  auto secondLowestCardAction = make_shared<domains::GoofSpielAction>(0, 2);
  auto thirdLowestCardAction = make_shared<domains::GoofSpielAction>(0, 3);
  auto fourthLowestCardAction = make_shared<domains::GoofSpielAction>(0, 4);

  auto setAction =
      [&player2Strat, &lowestCardAction, &secondLowestCardAction,
       &thirdLowestCardAction, &fourthLowestCardAction](
          shared_ptr<EFGNode> node) {
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

  // Create strategy that plays the lowest card
  BehavioralStrategy player1Strat;
  auto lowestCardAction1 = make_shared<domains::GoofSpielAction>(0, 1);
  auto secondLowestCardAction1 = make_shared<domains::GoofSpielAction>(0, 2);
  auto thirdLowestCardAction1 = make_shared<domains::GoofSpielAction>(0, 3);
  auto fourthLowestCardAction1 = make_shared<domains::GoofSpielAction>(0, 4);

  auto setAction1 = [&player1Strat, &lowestCardAction1, &secondLowestCardAction1,
      &thirdLowestCardAction1, &fourthLowestCardAction1](shared_ptr<EFGNode> node) {
    if (node->getDistanceFromRoot() == 1) {
      player1Strat[node->getAOHInfSet()] = {{lowestCardAction1, 1.0}};
    } else if (node->getDistanceFromRoot() == 3) {
      player1Strat[node->getAOHInfSet()] = {{secondLowestCardAction1, 1.0}};
    } else if (node->getDistanceFromRoot() == 5) {
      player1Strat[node->getAOHInfSet()] = {{thirdLowestCardAction1, 1.0}};
    } else if (node->getDistanceFromRoot() == 7) {
      player1Strat[node->getAOHInfSet()] = {{fourthLowestCardAction1, 1.0}};
    }
  };
  algorithms::treeWalkEFG(gsd, setAction1);

  auto utility =
      algorithms::computeUtilityTwoPlayersGame(gsd, player2Strat, player1Strat, player2, player1);
  cout << utility.second << "\n";
  // Value of the utility should be 5.
  BOOST_CHECK(std::abs(utility.second - 5) <= 0.001);

}

BOOST_AUTO_TEST_CASE(bestResponseFullDepthCard4) {
  domains::GoofSpielDomain gsd(4, 4, nullopt);

  int player1 = gsd.getPlayers()[1];
  int opponent = gsd.getPlayers()[0];


  // Create strategy that plays the lowest card
  BehavioralStrategy opponentStrat;
  auto lowestCardAction = make_shared<domains::GoofSpielAction>(0, 1);
  auto secondLowestCardAction = make_shared<domains::GoofSpielAction>(0, 2);
  auto thirdLowestCardAction = make_shared<domains::GoofSpielAction>(0, 3);
  auto fourthLowestCardAction = make_shared<domains::GoofSpielAction>(0, 4);

  auto setAction = [&opponentStrat, &lowestCardAction, &secondLowestCardAction,
                    &thirdLowestCardAction, &fourthLowestCardAction](
          shared_ptr<EFGNode> node) {
        if (node->getDistanceFromRoot() == 0) {
          opponentStrat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 2) {
          opponentStrat[node->getAOHInfSet()] = {{secondLowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 4) {
          opponentStrat[node->getAOHInfSet()] = {{thirdLowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 6) {
          opponentStrat[node->getAOHInfSet()] = {{fourthLowestCardAction, 1.0}};
        }
      };
  algorithms::treeWalkEFG(gsd, setAction);

  auto player1BestResponse = algorithms::bestResponseTo(opponentStrat, opponent, player1, gsd);
  cout << player1BestResponse.second << "\n";
  // Value of the best response should be 7.625. TODO: Double check this
  BOOST_CHECK(std::abs(player1BestResponse.second - 7.625) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseDepth2Card4) {
  domains::GoofSpielDomain gsd(4, 2, nullopt);

  int player1 = gsd.getPlayers()[1];
  int oponent = gsd.getPlayers()[0];


  // Create strategy that plays the lowest card
  BehavioralStrategy oponentStrat;
  auto lowestCardAction = make_shared<domains::GoofSpielAction>(0, 1);
  auto secondLowestCardAction = make_shared<domains::GoofSpielAction>(0, 2);

  auto setAction =
      [&oponentStrat, &lowestCardAction, &secondLowestCardAction](shared_ptr<EFGNode> node) {
        if (node->getDistanceFromRoot() == 0) {
          oponentStrat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 2) {
          oponentStrat[node->getAOHInfSet()] = {{secondLowestCardAction, 1.0}};
        }
      };
  algorithms::treeWalkEFG(gsd, setAction);

  auto player1BestResponse = algorithms::bestResponseTo(oponentStrat, oponent, player1, gsd);
  // Value of the best response should be 5.
  BOOST_CHECK(std::abs(player1BestResponse.second - 5) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseDepth2Card13) {
  domains::GoofSpielDomain gsd(1, nullopt);

  int player1 = gsd.getPlayers()[1];
  int opponent = gsd.getPlayers()[0];


  // Create strategy that plays the lowest card
  BehavioralStrategy player2Strat;
  auto lowestCardAction = make_shared<domains::GoofSpielAction>(0, 1);
  auto setAction = [&player2Strat, &lowestCardAction](shared_ptr<EFGNode> node) {
    if (node->getDistanceFromRoot() == 0) {
      player2Strat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
    }
  };
  algorithms::treeWalkEFG(gsd, setAction);

  auto player1BestResponse = algorithms::bestResponseTo(player2Strat, opponent, player1, gsd);

  BOOST_CHECK(std::abs(player1BestResponse.second - 7) <= 0.001);
}

BOOST_AUTO_TEST_CASE(numberOfFirstAndRootLevelNodes) {
  domains::GoofSpielDomain gsd(1, nullopt);

  int numberOfNodes = algorithms::countNodes(gsd);

  // 13 root state + 13*13 next level states
  BOOST_CHECK(numberOfNodes == 182);
}

BOOST_AUTO_TEST_CASE(numberOfSecondAndFirstAndRootLevelNodes) {
  domains::GoofSpielDomain gsd(2, nullopt);

  int numberOfNodes = algorithms::countNodes(gsd);

  // 182 previous levels + (13*13)*13*12 ====
  // 13*13 previous level nodes 13 player2 choices and 12 nature choices
  BOOST_CHECK(numberOfNodes == 26546);
}

BOOST_AUTO_TEST_CASE(numberOfInformationSetsDepth3) {  // TODO: check number of InfSets
  domains::GoofSpielDomain domain(3, nullopt);

  int player1 = domain.getPlayers()[0];
  int player2 = domain.getPlayers()[1];
  auto player1InfSetsAndActions =
      algorithms::generateInformationSetsAndAvailableActions(domain, player1);
  cout << player1InfSetsAndActions.size() <<"\n";
  auto player2InfSetsAndActions =
      algorithms::generateInformationSetsAndAvailableActions(domain, player2);

  auto numOfInfSetsPlayer1 = player1InfSetsAndActions.size();
  auto numOfInfSetsPlayer2 = player2InfSetsAndActions.size();

  // Number of infSets: rootStates: 13, p1 after first round : 13*(13*13)*12

  BOOST_CHECK(numOfInfSetsPlayer1 == 26377 && numOfInfSetsPlayer2 == 26377);
}

BOOST_AUTO_TEST_CASE(depth4numberOfNodes) {  // TODO: check number of Nodes
  domains::GoofSpielDomain gsd(4, nullopt);

  // Long test!!! 10 mins
  // [(13*13)*13*12] * [12*12*11]

  int numberOfNodes = algorithms::countNodes(gsd);
  BOOST_CHECK(numberOfNodes == 42103490);
}

BOOST_AUTO_TEST_SUITE_END()
