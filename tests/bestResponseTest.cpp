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

#include "base/base.h"
#include "algorithms/bestResponse.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/tree.h"
#include "algorithms/utility.h"
#include "algorithms/strategy.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"

#include "LPsolvers/LPSolver.h"
#include "tests/domainsTest.h"
#include "gtest/gtest.h"


namespace GTLib2::algorithms {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::MatchingPenniesAction;
using domains::ActionHeads;
using domains::ActionTails;
using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielVariant::CompleteObservations;
using domains::GoofSpielAction;


TEST(BestResponse, TestSmallDomain) {
    MatchingPenniesDomain domain(AlternatingMoves);

    auto initNodes = algorithms::createRootEFGNodes(domain.getRootStatesDistribution());
    auto tailAction = make_shared<MatchingPenniesAction>(ActionTails);
    auto headAction = make_shared<MatchingPenniesAction>(ActionHeads);

    auto rootNode = initNodes[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();

    auto childNode = rootNode->performAction(headAction)[0].first;
    auto childInfoset = childNode->getAOHInfSet();

    BehavioralStrategy stratHeads;
    auto action = make_shared<MatchingPenniesAction>(ActionHeads);
    stratHeads[rootInfoset] = {{action, 1.0}};

    auto brsVal = algorithms::bestResponseTo(stratHeads, Player(0), Player(1), domain, 2);
    auto optAction = (*brsVal.first[childInfoset].begin()).first;

    EXPECT_TRUE(*optAction == *tailAction);
}

TEST(BestResponse, GoofspielFullDepthCard4) {
    GoofSpielDomain domain
        ({variant:  CompleteObservations, numCards: 4, fixChanceCards: false, chanceCards: {}});

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain);
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);
    auto thirdLowestCardAction = make_shared<GoofSpielAction>(0, 3);
    auto fourthLowestCardAction = make_shared<GoofSpielAction>(0, 4);

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getEFGDepth() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->getEFGDepth() == 2) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        } else if (node->getEFGDepth() == 4) {
            playOnlyAction(profile[opponent][infoset], thirdLowestCardAction);
        } else if (node->getEFGDepth() == 6) {
            playOnlyAction(profile[opponent][infoset], fourthLowestCardAction);
        }
    };
    algorithms::treeWalkEFG(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain);

    EXPECT_TRUE(std::abs(bestResponse.second - 5.25) <= 0.001);
}

TEST(BestResponse, GoofspielDepth2Card4) {
    GoofSpielDomain domain
        ({variant:  CompleteObservations, numCards: 4, fixChanceCards: false, chanceCards: {}});

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain);
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getEFGDepth() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->getEFGDepth() == 2) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        }
    };
    algorithms::treeWalkEFG(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain);

    EXPECT_TRUE(std::abs(bestResponse.second - 5) <= 0.001);
}

TEST(BestResponse, GoofspielDepth1Card13) {
    GoofSpielDomain domain
        ({variant:  CompleteObservations, numCards: 13, fixChanceCards: false, chanceCards: {}});

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain);
    StrategyProfile profile = getUniformStrategy(cache, 1);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getEFGDepth() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        }
    };
    algorithms::treeWalkEFG(domain, setAction, 1);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain, 1);

    EXPECT_TRUE(std::abs(bestResponse.second - 7) <= 0.001);
}

}  // namespace GTLib2
