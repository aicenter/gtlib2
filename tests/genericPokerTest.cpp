//
// Created by Jakub Rozlivek on 30/08/18.
//


#include "../algorithms/treeWalk.h"
#include "../algorithms/bestResponse.h"
#include "../algorithms/common.h"
#include "../algorithms/equilibrium.h"
#include "../algorithms/cfr.h"
#include "../domains/genericPoker.h"
#include "../algorithms/utility.h"

#define BOOST_TEST_DYN_LINK  // For linking with dynamic libraries.

#include <boost/test/unit_test.hpp>

using namespace GTLib2;

BOOST_AUTO_TEST_SUITE(GenericPoker)

BOOST_AUTO_TEST_CASE(Poker63222numberOfInfSets) {
  domains::GenericPokerDomain domain(6, 3, 2, 2, 2);

  int player1 = domain.getPlayers()[0];
  int player2 = domain.getPlayers()[1];
  auto player1InfSetsAndActions =
      algorithms::generateInformationSetsAndAvailableActions(domain, player1);
  auto player2InfSetsAndActions =
      algorithms::generateInformationSetsAndAvailableActions(domain, player2);
  auto numOfInfSetsPlayer1 = player1InfSetsAndActions.size();
  auto numOfInfSetsPlayer2 = player2InfSetsAndActions.size();

  BOOST_CHECK(numOfInfSetsPlayer1 == 15750 && numOfInfSetsPlayer2 == 15750);
}

BOOST_AUTO_TEST_CASE(Poker83322numberOfNodes) {
  auto domain = domains::GenericPokerDomain(8, 3, 3, 2, 2);
  int numberOfNodes = algorithms::countNodes(domain);
  BOOST_CHECK(numberOfNodes == 1940352);
}

BOOST_AUTO_TEST_CASE(Poker63222CFR100iter) {
  domains::GenericPokerDomain domain(6, 3, 2, 2, 2);
  auto regrets = algorithms::CFRiterations(domain, 100);
  auto strat1 = algorithms::getStrategyFor(domain, domain.getPlayers()[0], regrets);
  auto strat2 = algorithms::getStrategyFor(domain, domain.getPlayers()[1], regrets);
  auto bestResp1 = algorithms::bestResponseTo(strat2, domain.getPlayers()[1],
      domain.getPlayers()[0], domain).second;
  auto bestResp2 =  algorithms::bestResponseTo(strat1, domain.getPlayers()[0],
      domain.getPlayers()[1], domain).second;
  double utility =  algorithms::computeUtilityTwoPlayersGame(domain, strat1, strat2,
      domain.getPlayers()[0], domain.getPlayers()[1]).first;
  BOOST_CHECK(std::abs(utility + 0.220125) <= 0.0001);
  BOOST_CHECK(std::abs(bestResp1 - 0.113038) <= 0.0001);
  BOOST_CHECK(std::abs(bestResp2 - 0.570683) <= 0.0001);
}

BOOST_AUTO_TEST_CASE(Poker63222ActionSequences) {
  domains::GenericPokerDomain domain(6, 3, 2, 2, 2);
  auto sequences = unordered_map<int, unordered_set<ActionSequence>>();
  sequences[domain.getPlayers()[0]] = unordered_set<ActionSequence>();
  sequences[domain.getPlayers()[1]] = unordered_set<ActionSequence>();
  auto countingFunction = [&sequences, &domain](shared_ptr<EFGNode> node) {
    if (node->getCurrentPlayer()) {
      for (auto &player : domain.getPlayers()) {
        auto seq = node->getActionsSeqOfPlayer(player);
        sequences[player].emplace(seq);
      }
    }
  };
  algorithms::treeWalkEFG(domain, countingFunction, domain.getMaxDepth());
  BOOST_CHECK(sequences[domain.getPlayers()[0]].size() == 15835);
  BOOST_CHECK(sequences[domain.getPlayers()[1]].size() == 14791);
}


BOOST_AUTO_TEST_SUITE_END()
