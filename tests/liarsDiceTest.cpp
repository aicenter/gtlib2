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

#include "base/base.h"

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/tree.h"
#include "algorithms/stats.h"
#include "algorithms/utility.h"
#include "algorithms/strategy.h"
#include "domains/liarsDice.h"
#include "utils/functools.h"
#include "tests/domainsTest.h"

#include <boost/range/combine.hpp>
#include <boost/test/unit_test.hpp>

namespace GTLib2::domains {

using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;
using algorithms::getUniformStrategy;
using algorithms::playOnlyAction;

BOOST_AUTO_TEST_SUITE(DomainsTests)
BOOST_AUTO_TEST_SUITE(LiarsDice)

// @formatter:off
LiarsDiceDomain testDomainsLiarsDice[] = { // NOLINT(cert-err58-cpp)
    LiarsDiceDomain({2, 2}, 2),
    LiarsDiceDomain({3, 0}, 3),
    LiarsDiceDomain({1, 0}, 2),
    LiarsDiceDomain({1, 0}, 3),
    LiarsDiceDomain({2, 1}, 4),
    LiarsDiceDomain({1, 1}, 6),
};
// @formatter:on


BOOST_AUTO_TEST_CASE(buildGameTreeAndCheckSizesLD) {
    DomainStatistics expected[] = {
        {
            .max_EFGDepth   = 9,
            .max_StateDepth = 9,
            .num_nodes      = 4599,
            .num_terminals  = 2295,
            .num_states     = 4599,
            .num_histories  = {1152, 1152},
            .num_infosets   = {384, 384},
            .num_sequences  = {766, 766},
        },
        {
            .max_EFGDepth   = 10,
            .max_StateDepth = 10,
            .num_nodes      = 10230,
            .num_terminals  = 5110,
            .num_states     = 10230,
            .num_histories  = {2560, 2560},
            .num_infosets   = {2560, 256},
            .num_sequences  = {5111, 512},
        },
        {
            .max_EFGDepth   = 3,
            .max_StateDepth = 3,
            .num_nodes      = 14,
            .num_terminals  = 6,
            .num_states     = 14,
            .num_histories  = {4, 4},
            .num_infosets   = {4, 2},
            .num_sequences  = {7, 4},
        },
        {
            .max_EFGDepth   = 4,
            .max_StateDepth = 4,
            .num_nodes      = 45,
            .num_terminals  = 21,
            .num_states     = 45,
            .num_histories  = {12, 12},
            .num_infosets   = {12, 4},
            .num_sequences  = {22, 8},
        },
        {
            .max_EFGDepth   = 5,
            .max_StateDepth = 5,
            .num_nodes      = 93,
            .num_terminals  = 45,
            .num_states     = 93,
            .num_histories  = {24, 24},
            .num_infosets   = {24, 8},
            .num_sequences  = {46, 16},
        },
        {
            .max_EFGDepth   = 13,
            .max_StateDepth = 13,
            .num_nodes      = 294876,
            .num_terminals  = 147420,
            .num_states     = 294876,
            .num_histories  = {73728, 73728},
            .num_infosets   = {12288, 12288},
            .num_sequences  = {24571, 24571},
        }
    };

    for (auto tuple : zip(testDomainsLiarsDice, expected)) {
        unzip(tuple, testDomain, expected);
        DomainStatistics actualStat;
        calculateDomainStatistics(testDomain, &actualStat);

        BOOST_CHECK_EQUAL(actualStat, expected);
    }
}

// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
