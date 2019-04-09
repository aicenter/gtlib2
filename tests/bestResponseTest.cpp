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


#include "algorithms/bestResponse.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/tree.h"
#include "algorithms/utility.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"
#include "LPsolvers/LPSolver.h"
#include "domains/goofSpiel.h"

#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>
#include <algorithms/strategy.h>


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::SimultaneousMatchingPenniesDomain;
using domains::Heads;
using domains::Tails;
using algorithms::DomainStatistics;
using algorithms::playOnlyAction;
using domains::GoofSpielDomain;
using domains::GoofSpielAction;
using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;
using algorithms::getUniformStrategy;
using algorithms::playOnlyAction;
using std::unordered_set;

BOOST_AUTO_TEST_SUITE(BestResponseTests)


BOOST_AUTO_TEST_CASE(testSmallDomain) {
    MatchingPenniesDomain domain;

    auto initNodes = algorithms::createRootEFGNodes(domain.getRootStatesDistribution());
    auto tailAction = make_shared<MatchingPenniesAction>(Tails);
    auto headAction = make_shared<MatchingPenniesAction>(Heads);

    auto rootNode = initNodes[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();

    auto childNode = rootNode->performAction(headAction)[0].first;
    auto childInfoset = childNode->getAOHInfSet();

    BehavioralStrategy stratHeads;
    stratHeads[rootInfoset] = ProbDistribution(2, 0.0);
    playOnlyAction(stratHeads[rootInfoset], headAction->getId());

    auto brsVal = algorithms::bestResponseTo(stratHeads, Player(0), Player(1), domain, 2);
    auto brProbs = brsVal.first[childInfoset];
    auto optAction = *max_element(brProbs.begin(), brProbs.end());

    BOOST_CHECK(optAction == tailAction->getId());
}

BOOST_AUTO_TEST_CASE(bestResponseFullDepthCard4) {
    GoofSpielDomain domain(4, 4, nullopt);

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain.getRootStatesDistribution());
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1)->getId();
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2)->getId();
    auto thirdLowestCardAction = make_shared<GoofSpielAction>(0, 3)->getId();
    auto fourthLowestCardAction = make_shared<GoofSpielAction>(0, 4)->getId();

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getDistanceFromRoot() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->getDistanceFromRoot() == 2) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        } else if (node->getDistanceFromRoot() == 4) {
            playOnlyAction(profile[opponent][infoset], thirdLowestCardAction);
        } else if (node->getDistanceFromRoot() == 6) {
            playOnlyAction(profile[opponent][infoset], fourthLowestCardAction);
        }
    };
    algorithms::treeWalkEFG(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain);

    BOOST_CHECK(std::abs(bestResponse.second - 5.25) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseDepth2Card4) {
    GoofSpielDomain domain(4, 2, nullopt);

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain.getRootStatesDistribution());
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1)->getId();
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2)->getId();

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getDistanceFromRoot() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->getDistanceFromRoot() == 2) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        }
    };
    algorithms::treeWalkEFG(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain);

    BOOST_CHECK(std::abs(bestResponse.second - 5) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseDepth1Card13) {
    GoofSpielDomain domain(1, nullopt);

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain.getRootStatesDistribution());
    StrategyProfile profile = getUniformStrategy(cache, 1);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1)->getId();

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getDistanceFromRoot() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        }
    };
    algorithms::treeWalkEFG(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain);

    BOOST_CHECK(std::abs(bestResponse.second - 7) <= 0.001);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
