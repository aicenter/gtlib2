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


BOOST_AUTO_TEST_SUITE(MatchingPenniesTests)

#if LP_SOLVER != NO_LP_SOLVER
BOOST_AUTO_TEST_CASE(best_response_to_equilibrium) {
    MatchingPenniesDomain d;
    auto v = algorithms::findEquilibriumTwoPlayersZeroSum(d);
    auto strat = std::get<1>(v);
    auto brsVal = algorithms::bestResponseTo(strat, 0, 1, d, 5);
    double val = std::get<1>(brsVal);
    BOOST_CHECK(val == 0.0);
}
#endif

#if LP_SOLVER != NO_LP_SOLVER
BOOST_AUTO_TEST_CASE(equilibrium_normal_form_lp_test) {
    MatchingPenniesDomain d;
    auto v = algorithms::findEquilibriumTwoPlayersZeroSum(d);
    auto strat = std::get<1>(v);
    auto actionHeads = make_shared<MatchingPenniesAction>(Heads);
    auto actionTails = make_shared<MatchingPenniesAction>(Tails);
    double headsProb = (*strat.begin()).second[actionHeads->getId()];
    double tailsProb = (*strat.begin()).second[actionTails->getId()];

    BOOST_CHECK(std::get<0>(v) == 0);
    BOOST_CHECK(headsProb == 0.5 && tailsProb == 0.5);
}
#endif

BOOST_AUTO_TEST_CASE(bestResponseSmallDomain) {
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
