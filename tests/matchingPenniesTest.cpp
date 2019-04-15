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

BOOST_AUTO_TEST_SUITE(DomainsTests)
BOOST_AUTO_TEST_SUITE(MatchingPennies)

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
BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
