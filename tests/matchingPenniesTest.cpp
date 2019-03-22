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
#include "algorithms/treeWalk.h"
#include "algorithms/utility.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"

#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::SimultaneousMatchingPenniesDomain;
using domains::Heads;
using domains::Tails;
using algorithms::DomainStatistics;


BOOST_AUTO_TEST_SUITE(Matching_Pennies)

BOOST_AUTO_TEST_CASE(best_response_to_equilibrium) {
    MatchingPenniesDomain d;
    auto v = algorithms::findEquilibriumTwoPlayersZeroSum(d);
    auto strat = std::get<1>(v);
    auto brsVal = algorithms::bestResponseTo(strat, 0, 1, d, 5);
    double val = std::get<1>(brsVal);
    BOOST_CHECK(val == 0.0);
}

BOOST_AUTO_TEST_CASE(equilibrium_normal_form_lp_test) {
    MatchingPenniesDomain d;
    auto v = algorithms::findEquilibriumTwoPlayersZeroSum(d);
    auto strat = std::get<1>(v);
    auto actionHeads = make_shared<MatchingPenniesAction>(Heads);
    auto actionTails = make_shared<MatchingPenniesAction>(Tails);
    double headsProb = (*strat.begin()).second[actionHeads];
    double tailsProb = (*strat.begin()).second[actionTails];

    BOOST_CHECK(std::get<0>(v) == 0);
    BOOST_CHECK(headsProb == 0.5 && tailsProb == 0.5);
}

BOOST_AUTO_TEST_CASE(best_response_test) {
    MatchingPenniesDomain d;

    auto initNodes =
        algorithms::createRootEFGNodes(
            d.getRootStatesDistribution());
    auto firstNode = (*initNodes.begin()).first;

    auto player0Is = firstNode->getAOHInfSet();
    auto secondNode = (*firstNode->performAction(
        make_shared<MatchingPenniesAction>(Heads)).begin()).first;
    auto player1Is = secondNode->getAOHInfSet();

    BehavioralStrategy stratHeads;
    auto action = make_shared<MatchingPenniesAction>(Heads);
    stratHeads[player0Is] = {{action, 1.0}};
    auto brsVal = algorithms::bestResponseTo(stratHeads, 0, 1, d, 5);
    auto optAction = (*brsVal.first[player1Is].begin()).first;
    auto tailAction = make_shared<MatchingPenniesAction>(Tails);

    BOOST_CHECK(*optAction == *tailAction);
}

BOOST_AUTO_TEST_CASE(buildGameTreeAndCheckSizes) {
    DomainStatistics expectedStat = {
        .max_EFGDepth   = 2,
        .max_StateDepth = 2,
        .num_nodes      = 7,
        .num_terminals  = 4,
        .num_states     = 7,
        .num_histories  = {1, 2},
        .num_infosets   = {1, 1},
        .num_sequences  = {3, 3},
    };

    MatchingPenniesDomain testDomain;
    DomainStatistics actualStat;
    calculateDomainStatistics(testDomain, &actualStat);

    BOOST_CHECK_EQUAL(actualStat, expectedStat);
}

BOOST_AUTO_TEST_CASE(buildGameTreeAndCheckSizesSimultaneous) {
    DomainStatistics expectedStat = {
        .max_EFGDepth   = 2,
        .max_StateDepth = 1,
        .num_nodes      = 7,
        .num_terminals  = 4,
        .num_states     = 5,
        .num_histories  = {1, 2},
        .num_infosets   = {1, 1},
        .num_sequences  = {3, 3},
    };

    SimultaneousMatchingPenniesDomain testDomain;
    DomainStatistics actualStat;
    calculateDomainStatistics(testDomain, &actualStat);

    BOOST_CHECK_EQUAL(actualStat, expectedStat);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
