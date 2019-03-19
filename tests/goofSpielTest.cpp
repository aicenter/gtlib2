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

    You should have received a copy of the GNU Lesser General Public
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#include <unordered_set>

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/treeWalk.h"
#include "algorithms/stats.h"
#include "algorithms/utility.h"
#include "domains/goofSpiel.h"
#include "utils/functools.h"
#include "tests/domainsTest.h"

// This must be defined in exacly one test file.
#define BOOST_TEST_MODULE GTLib2 test
// todo: move this out somehow
#include <boost/range/combine.hpp>
#include <boost/test/unit_test.hpp>

namespace GTLib2 {

using domains::GoofSpielDomain;
using domains::GoofSpielAction;
using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;
using std::unordered_set;

BOOST_AUTO_TEST_SUITE(GoofSpiel)

GoofSpielDomain testDomainsGoofSpiel[] = { // NOLINT(cert-err58-cpp)
    GoofSpielDomain(3, 1, nullopt),
    GoofSpielDomain(3, 2, nullopt),
    GoofSpielDomain(3, 3, nullopt),
    GoofSpielDomain(3, 4, nullopt), // we should get the same results as prev case

    GoofSpielDomain(4, 4, nullopt),
};


BOOST_AUTO_TEST_CASE(buildGameTreeAndCheckSizes) {
    DomainStatistics expectedStats[] = {
        {
            .max_EFGDepth   = 2,
            .max_StateDepth = 1,
            .num_nodes      = 66,
            .num_terminals  = 0,
            .num_states     = 57,
            .num_histories  = {57, 9},
            .num_infosets   = {57, 3},
            .num_sequences  = {10, 10},
        },{
            .max_EFGDepth   = 4,
            .max_StateDepth = 2,
            .num_nodes      = 390,
            .num_terminals  = 0,
            .num_states     = 273,
            .num_histories  = {273, 117},
            .num_infosets   = {273, 57},
            .num_sequences  = {118, 118},
        },{
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 822,
            .num_terminals  = 216,
            .num_states     = 489,
            .num_histories  = {273, 333},
            .num_infosets   = {273, 273},
            .num_sequences  = {334, 334},
        },{
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 822,
            .num_terminals  = 216,
            .num_states     = 489,
            .num_histories  = {273, 333},
            .num_infosets   = {273, 273},
            .num_sequences  = {334, 334},
        },{
            .max_EFGDepth   = 8,
            .max_StateDepth = 4,
            .num_nodes      = 52628,
            .num_terminals  = 13824,
            .num_states     = 31300,
            .num_histories  = {17476, 21328},
            .num_infosets   = {17476, 17476},
            .num_sequences  = {21329, 21329},
        },
    };

    for (auto tuple : zip(testDomainsGoofSpiel, expectedStats)) {
        unzip(tuple, testDomain, expectedStat);
        DomainStatistics actualStat;
        calculateDomainStatistics(testDomain, &actualStat);

        BOOST_CHECK_EQUAL(actualStat, expectedStat);
    }
}

BOOST_AUTO_TEST_CASE(computeUtilityFullDepthCard4) {
    GoofSpielDomain gsd(4, 4, nullopt);

    int player1 = gsd.getPlayers()[1];
    int player2 = gsd.getPlayers()[0];


    // Create strategy that plays the lowest card
    BehavioralStrategy player2Strat;
    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);
    auto thirdLowestCardAction = make_shared<GoofSpielAction>(0, 3);
    auto fourthLowestCardAction = make_shared<GoofSpielAction>(0, 4);

    auto setAction =
        [&player2Strat, &lowestCardAction, &secondLowestCardAction,
            &thirdLowestCardAction, &fourthLowestCardAction](
            shared_ptr<EFGNode> node) {
            if (node->getDistanceFromRoot() == 0) {
                player2Strat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
            } else if (node->getDistanceFromRoot() == 2) {
                player2Strat[node->getAOHInfSet()] =
                    {{secondLowestCardAction, 1.0}};
            } else if (node->getDistanceFromRoot() == 4) {
                player2Strat[node->getAOHInfSet()] =
                    {{thirdLowestCardAction, 1.0}};
            } else if (node->getDistanceFromRoot() == 6) {
                player2Strat[node->getAOHInfSet()] =
                    {{fourthLowestCardAction, 1.0}};
            }
        };
    algorithms::treeWalkEFG(gsd, setAction);

    // Create strategy that plays the lowest card
    BehavioralStrategy player1Strat;
    auto lowestCardAction1 = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction1 = make_shared<GoofSpielAction>(0, 2);
    auto thirdLowestCardAction1 = make_shared<GoofSpielAction>(0, 3);
    auto fourthLowestCardAction1 = make_shared<GoofSpielAction>(0, 4);

    auto setAction1 =
        [&player1Strat, &lowestCardAction1, &secondLowestCardAction1,
            &thirdLowestCardAction1, &fourthLowestCardAction1]
            (shared_ptr<EFGNode> node) {
            if (node->getDistanceFromRoot() == 1) {
                player1Strat[node->getAOHInfSet()] = {{lowestCardAction1, 1.0}};
            } else if (node->getDistanceFromRoot() == 3) {
                player1Strat[node->getAOHInfSet()] =
                    {{secondLowestCardAction1, 1.0}};
            } else if (node->getDistanceFromRoot() == 5) {
                player1Strat[node->getAOHInfSet()] =
                    {{thirdLowestCardAction1, 1.0}};
            } else if (node->getDistanceFromRoot() == 7) {
                player1Strat[node->getAOHInfSet()] =
                    {{fourthLowestCardAction1, 1.0}};
            }
        };
    algorithms::treeWalkEFG(gsd, setAction1);

    auto utility =
        algorithms::computeUtilityTwoPlayersGame(gsd,
                                                 player2Strat,
                                                 player1Strat,
                                                 player2,
                                                 player1);

    BOOST_CHECK(std::abs(utility.second - 0) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseFullDepthCard4) {
    GoofSpielDomain gsd(4, 4, nullopt);

    int player1 = gsd.getPlayers()[1];
    int opponent = gsd.getPlayers()[0];


    // Create strategy that plays the lowest card
    BehavioralStrategy opponentStrat;
    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);
    auto thirdLowestCardAction = make_shared<GoofSpielAction>(0, 3);
    auto fourthLowestCardAction = make_shared<GoofSpielAction>(0, 4);

    auto
        setAction = [&opponentStrat, &lowestCardAction, &secondLowestCardAction,
        &thirdLowestCardAction, &fourthLowestCardAction](
        shared_ptr<EFGNode> node) {
        if (node->getDistanceFromRoot() == 0) {
            opponentStrat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 2) {
            opponentStrat[node->getAOHInfSet()] =
                {{secondLowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 4) {
            opponentStrat[node->getAOHInfSet()] =
                {{thirdLowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 6) {
            opponentStrat[node->getAOHInfSet()] =
                {{fourthLowestCardAction, 1.0}};
        }
    };
    algorithms::treeWalkEFG(gsd, setAction);

    auto player1BestResponse =
        algorithms::bestResponseTo(opponentStrat, opponent, player1, gsd);

    BOOST_CHECK(std::abs(player1BestResponse.second - 5.25) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseDepth2Card4) {
    GoofSpielDomain gsd(4, 2, nullopt);

    int player1 = gsd.getPlayers()[1];
    int oponent = gsd.getPlayers()[0];


    // Create strategy that plays the lowest card
    BehavioralStrategy oponentStrat;
    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);

    auto setAction =
        [&oponentStrat, &lowestCardAction, &secondLowestCardAction](shared_ptr<
            EFGNode> node) {
            if (node->getDistanceFromRoot() == 0) {
                oponentStrat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
            } else if (node->getDistanceFromRoot() == 2) {
                oponentStrat[node->getAOHInfSet()] =
                    {{secondLowestCardAction, 1.0}};
            }
        };
    algorithms::treeWalkEFG(gsd, setAction);

    auto player1BestResponse =
        algorithms::bestResponseTo(oponentStrat, oponent, player1, gsd);

    BOOST_CHECK(std::abs(player1BestResponse.second - 5) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseDepth1Card13) {
    GoofSpielDomain gsd(1, nullopt);

    int player1 = gsd.getPlayers()[1];
    int opponent = gsd.getPlayers()[0];


    // Create strategy that plays the lowest card
    BehavioralStrategy player2Strat;
    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto setAction =
        [&player2Strat, &lowestCardAction](shared_ptr<EFGNode> node) {
            if (node->getDistanceFromRoot() == 0) {
                player2Strat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
            }
        };
    algorithms::treeWalkEFG(gsd, setAction);

    auto player1BestResponse =
        algorithms::bestResponseTo(player2Strat, opponent, player1, gsd);

    BOOST_CHECK(std::abs(player1BestResponse.second - 7) <= 0.001);
}


// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!

// todo: make a fast CFR test!
//BOOST_AUTO_TEST_CASE(FulldepthCard4CFR2iter) {
//    GoofSpielDomain gsd(4, 4, nullopt);
//    auto regrets = algorithms::CFRiterations(gsd, 2);
//    auto strat1 = algorithms::getStrategyFor(gsd, gsd.getPlayers()[0], regrets);
//    auto strat2 = algorithms::getStrategyFor(gsd, gsd.getPlayers()[1], regrets);
//    auto bestResp1 =
//        algorithms::bestResponseTo(strat2,
//                                   gsd.getPlayers()[1],
//                                   gsd.getPlayers()[0],
//                                   gsd).second;
//    auto bestResp2 =
//        algorithms::bestResponseTo(strat1,
//                                   gsd.getPlayers()[0],
//                                   gsd.getPlayers()[1],
//                                   gsd).second;
//    double
//        utility = algorithms::computeUtilityTwoPlayersGame(
//        gsd, strat1, strat2, gsd.getPlayers()[0], gsd.getPlayers()[1]).first;
//    BOOST_CHECK(std::abs(utility  - -0.434018) <= 0.0001);
//    BOOST_CHECK(std::abs(bestResp1 - 1.566028) <= 0.0001);
//    BOOST_CHECK(std::abs(bestResp2 - 1.784722) <= 0.0001);
//}


BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
