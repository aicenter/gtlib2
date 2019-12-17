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

#include "algorithms/utility.h"
#include "algorithms/strategy.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"

#include "LPsolvers/LPSolver.h"
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

    auto rootNode = createRootEFGNode(domain.getRootStatesDistribution());
    auto tailAction = make_shared<MatchingPenniesAction>(ActionTails);
    auto headAction = make_shared<MatchingPenniesAction>(ActionHeads);

    auto rootInfoset = rootNode->getAOHInfSet();

    auto childNode = rootNode->performAction(headAction);
    auto childInfoset = childNode->getAOHInfSet();

    BehavioralStrategy stratHeads;
    stratHeads[rootInfoset] = {{headAction, 1.0}};

    auto response = algorithms::bestResponseTo(stratHeads, Player(1), domain);
    auto infosetStrategy = response.strategy[childInfoset];
    auto optimalAction = (*infosetStrategy.begin()).first;

    EXPECT_EQ(*optimalAction, *tailAction);
}

TEST(BestResponse, GoofspielFullDepthCard4) {
    GoofSpielDomain domain({/*.variant=*/CompleteObservations, /*.numCards=*/4,
                               /*.fixChanceCards=*/false, /*.chanceCards=*/{}});

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain);
    StrategyProfile profile = getUniformStrategy(cache);

    // specify which card should the opponent play at each depth of the tree
    // the sequence of players playing goes like this:
    // C01C01C0101
    unordered_map<int, shared_ptr<GoofSpielAction>> actions;
    actions.emplace(1, make_shared<GoofSpielAction>(0, 1));
    actions.emplace(4, make_shared<GoofSpielAction>(0, 2));
    actions.emplace(7, make_shared<GoofSpielAction>(0, 3));
    actions.emplace(9, make_shared<GoofSpielAction>(0, 4));

    auto setAction = [&](shared_ptr<EFGNode> node) {
        if (node->type_ != PlayerNode) return;

        if (node->getPlayer() == opponent) {
            auto infoset = node->getAOHInfSet();
            playOnlyAction(profile[opponent][infoset], actions[node->efgDepth()]);
        }
    };
    treeWalk(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], player, domain);

    EXPECT_LE(std::abs(bestResponse.value - 5.25), 0.001);
}

TEST(BestResponse, GoofspielDepth2Card4) {
    GoofSpielDomain domain({/*.variant=*/CompleteObservations, /*.numCards=*/4,
                               /*.fixChanceCards=*/false, /*.chanceCards=*/{}});

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain);
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);

    auto setAction = [&](shared_ptr<EFGNode> node) {
        if (node->type_ != PlayerNode) return;

        auto infoset = node->getAOHInfSet();
        if (node->efgDepth() == 1) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->efgDepth() == 4) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        }
    };
    treeWalk(domain, setAction);

    auto bestResponse = algorithms::bestResponseTo(profile[opponent], player, domain);
    EXPECT_LE(std::abs(bestResponse.value - 5), 0.001);
}

// todo: im not sure what to do with max depth and best response at the moment
//   seems like a weird concept. I think only the domain should specify optional max depth
//TEST(BestResponse, GoofspielDepth1Card13) {
//    GoofSpielDomain domain
//        ({variant:  CompleteObservations, numCards: 13, fixChanceCards: false, chanceCards: {}});
//
//    auto player = Player(1);
//    auto opponent = Player(0);
//
//    InfosetCache cache(domain);
//    StrategyProfile profile = getUniformStrategy(cache, 1);
//
//    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
//
//    auto setAction = [&](shared_ptr<EFGNode> node) {
//        if(node->type_ != PlayerNode) return;
//
//        if (node->efgDepth() == 1) {
//            auto infoset = node->getAOHInfSet();
//            playOnlyAction(profile[opponent][infoset], lowestCardAction);
//        }
//    };
//    treeWalk(domain, setAction, 1);
//
//    auto bestResponse = algorithms::bestResponseTo(profile[opponent], opponent, player, domain);
//    EXPECT_LE(std::abs(bestResponse.value - 7), 0.001);
//}

}  // namespace GTLib2
