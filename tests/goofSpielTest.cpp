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
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielVariant::CompleteObservations;
using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;
using algorithms::getUniformStrategy;
using algorithms::playOnlyAction;
using std::unordered_set;

BOOST_AUTO_TEST_SUITE(DomainsTests)
BOOST_AUTO_TEST_SUITE(GoofSpiel)

GoofSpielDomain testDomainsGoofSpiel[] = { // NOLINT(cert-err58-cpp)
    GoofSpielDomain(1, CompleteObservations),
    GoofSpielDomain(2, CompleteObservations),
    GoofSpielDomain(3, CompleteObservations),
    GoofSpielDomain(1, 0, CompleteObservations),
    GoofSpielDomain(2, 0, CompleteObservations),
    GoofSpielDomain(3, 0, CompleteObservations),
    GoofSpielDomain(1, IncompleteObservations),
    GoofSpielDomain(2, IncompleteObservations),
    GoofSpielDomain(3, IncompleteObservations),
    GoofSpielDomain(1, 0, IncompleteObservations),
    GoofSpielDomain(2, 0, IncompleteObservations),
    GoofSpielDomain(3, 0, IncompleteObservations),
};


BOOST_AUTO_TEST_CASE(buildGameTreeAndCheckSizes) {
    DomainStatistics expectedStats[] = {
        {
            .max_EFGDepth   = 2,
            .max_StateDepth = 1,
            .num_nodes      = 3,
            .num_terminals  = 1,
            .num_states     = 2,
            .num_histories  = {1, 1},
            .num_infosets   = {1, 1},
            .num_sequences  = {2, 2},
        }, {
            .max_EFGDepth   = 4,
            .max_StateDepth = 2,
            .num_nodes      = 30,
            .num_terminals  = 8,
            .num_states     = 18,
            .num_histories  = {10, 12},
            .num_infosets   = {10, 10},
            .num_sequences  = {13, 13},
        }, {
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 822,
            .num_terminals  = 216,
            .num_states     = 489,
            .num_histories  = {273, 333},
            .num_infosets   = {273, 273},
            .num_sequences  = {334, 334},
        },  {
            .max_EFGDepth   = 2,
            .max_StateDepth = 1,
            .num_nodes      = 3,
            .num_terminals  = 1,
            .num_states     = 2,
            .num_histories  = {1, 1},
            .num_infosets   = {1, 1},
            .num_sequences  = {2, 2},
        }, {
            .max_EFGDepth   = 4,
            .max_StateDepth = 2,
            .num_nodes      = 15,
            .num_terminals  = 4,
            .num_states     = 9,
            .num_histories  = {5, 6},
            .num_infosets   = {5, 5},
            .num_sequences  = {7, 7},
        }, {
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 139,
            .num_terminals  = 36,
            .num_states     = 82,
            .num_histories  = {46, 57},
            .num_infosets   = {46, 46},
            .num_sequences  = {58, 58},
        }, {
            .max_EFGDepth   = 2,
            .max_StateDepth = 1,
            .num_nodes      = 3,
            .num_terminals  = 1,
            .num_states     = 2,
            .num_histories  = {1, 1},
            .num_infosets   = {1, 1},
            .num_sequences  = {2, 2},
        }, {
            .max_EFGDepth   = 4,
            .max_StateDepth = 2,
            .num_nodes      = 30,
            .num_terminals  = 8,
            .num_states     = 18,
            .num_histories  = {10, 12},
            .num_infosets   = {10, 10},
            .num_sequences  = {13, 13},
        }, {
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 822,
            .num_terminals  = 216,
            .num_states     = 489,
            .num_histories  = {273, 333},
            .num_infosets   = {213, 213},
            .num_sequences  = {262, 262},
        }, {
            .max_EFGDepth   = 2,
            .max_StateDepth = 1,
            .num_nodes      = 3,
            .num_terminals  = 1,
            .num_states     = 2,
            .num_histories  = {1, 1},
            .num_infosets   = {1, 1},
            .num_sequences  = {2, 2},
        }, {
            .max_EFGDepth   = 4,
            .max_StateDepth = 2,
            .num_nodes      = 15,
            .num_terminals  = 4,
            .num_states     = 9,
            .num_histories  = {5, 6},
            .num_infosets   = {5, 5},
            .num_sequences  = {7, 7},
        }, {
            .max_EFGDepth   = 6,
            .max_StateDepth = 3,
            .num_nodes      = 139,
            .num_terminals  = 36,
            .num_states     = 82,
            .num_histories  = {46, 57},
            .num_infosets   = {36, 36},
            .num_sequences  = {46, 46},
        }
    };

    for (auto tuple : zip(testDomainsGoofSpiel, expectedStats)) {
        unzip(tuple, testDomain, expectedStat);
        DomainStatistics actualStat;
        calculateDomainStatistics(testDomain, &actualStat);

        BOOST_CHECK_EQUAL(actualStat, expectedStat);
    }
}

// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
