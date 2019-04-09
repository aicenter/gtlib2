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
#include "algorithms/tree.h"
#include "algorithms/stats.h"
#include "algorithms/utility.h"
#include "algorithms/strategy.h"
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
using algorithms::getUniformStrategy;
using algorithms::playOnlyAction;
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
        }, {
            .max_EFGDepth   = 4,
            .max_StateDepth = 2,
            .num_nodes      = 390,
            .num_terminals  = 0,
            .num_states     = 273,
            .num_histories  = {273, 117},
            .num_infosets   = {273, 57},
            .num_sequences  = {118, 118},
        }, {
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 822,
            .num_terminals  = 216,
            .num_states     = 489,
            .num_histories  = {273, 333},
            .num_infosets   = {273, 273},
            .num_sequences  = {334, 334},
        }, {
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 822,
            .num_terminals  = 216,
            .num_states     = 489,
            .num_histories  = {273, 333},
            .num_infosets   = {273, 273},
            .num_sequences  = {334, 334},
        }, {
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
    GoofSpielDomain domain(4, 4, nullopt);

    Player player = Player(1);
    Player opponent = Player(0);

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
        } else if (node->getDistanceFromRoot() == 1) {
            playOnlyAction(profile[player][infoset], lowestCardAction);
        } else if (node->getDistanceFromRoot() == 3) {
            playOnlyAction(profile[player][infoset], secondLowestCardAction);
        } else if (node->getDistanceFromRoot() == 5) {
            playOnlyAction(profile[player][infoset], thirdLowestCardAction);
        } else if (node->getDistanceFromRoot() == 7) {
            playOnlyAction(profile[player][infoset], fourthLowestCardAction);
        }

    };
    algorithms::treeWalkEFG(domain, setAction);

    auto utility = algorithms::computeUtilityTwoPlayersGame(domain, profile[0], profile[1],
                                                            Player(0), Player(1));

    BOOST_CHECK(std::abs(utility.second - 0) <= 0.001);
}

BOOST_AUTO_TEST_CASE(bestResponseFullDepthCard4) {
    GoofSpielDomain domain(4, 4, nullopt);

    Player player = Player(1);
    Player opponent = Player(0);

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

    Player player = Player(1);
    Player opponent = Player(0);

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

    Player player = Player(1);
    Player opponent = Player(0);

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


// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!



BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
