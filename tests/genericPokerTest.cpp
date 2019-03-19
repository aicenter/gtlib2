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


#include <unordered_set>

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/utility.h"
#include "domains/genericPoker.h"
#include "utils/functools.h"
#include "tests/domainsTest.h"

#include <boost/test/unit_test.hpp>

using std::unordered_set;

namespace GTLib2 {

using domains::GenericPokerDomain;
using domains::GenericPokerAction;
using algorithms::DomainStatistics;
using std::unordered_set;


BOOST_AUTO_TEST_SUITE(GenericPoker)

GenericPokerDomain testDomainsPoker[] = { // NOLINT(cert-err58-cpp)
    GenericPokerDomain(2, 2, 2, 2, 2),
    GenericPokerDomain(3, 3, 1, 2, 3),
};


BOOST_AUTO_TEST_CASE(buildGameTreeAndCheckSizes) {
    DomainStatistics expectedStats[] = {
        {
            .max_EFGDepth   = 10,
            .max_StateDepth = 10,
            .num_nodes      = 15370,
            .num_terminals  = 10030,
            .num_states     = 15370,
            .num_histories  = {2670, 2670},
            .num_infosets   = {1770, 1770},
            .num_sequences  = {5075, 5075},
        }, {
            .max_EFGDepth   = 8,
            .max_StateDepth = 8,
            .num_nodes      = 23715,
            .num_terminals  = 15291,
            .num_states     = 23715,
            .num_histories  = {4212, 4212},
            .num_infosets   = {1404, 1404},
            .num_sequences  = {3901, 3901},
        }
    };

    for (auto tuple : zip(testDomainsPoker, expectedStats)) {
        unzip(tuple, testDomain, expectedStat);
        DomainStatistics actualStat;
        calculateDomainStatistics(testDomain, &actualStat);

        BOOST_CHECK_EQUAL(actualStat, expectedStat);
    }
}

// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!

// todo: make a fast CFR test!
//BOOST_AUTO_TEST_CASE(Poker63222CFR100iter) {
//    domains::GenericPokerDomain domain(6, 3, 2, 2, 2);
//    auto regrets = algorithms::CFRiterations(domain, 100);
//    auto strat1 =
//        algorithms::getStrategyFor(domain, domain.getPlayers()[0], regrets);
//    auto strat2 =
//        algorithms::getStrategyFor(domain, domain.getPlayers()[1], regrets);
//    auto bestResp1 = algorithms::bestResponseTo(
//        strat2, domain.getPlayers()[1], domain.getPlayers()[0], domain).second;
//    auto bestResp2 = algorithms::bestResponseTo(
//        strat1, domain.getPlayers()[0], domain.getPlayers()[1], domain).second;
//    double utility = algorithms::computeUtilityTwoPlayersGame(
//        domain, strat1, strat2, domain.getPlayers()[0], domain.getPlayers()[1]
//    ).first;
//
//    BOOST_CHECK(std::abs(utility + 0.220125) <= 0.0001);
//    BOOST_CHECK(std::abs(bestResp1 - 0.113038) <= 0.0001);
//    BOOST_CHECK(std::abs(bestResp2 - 0.570683) <= 0.0001);
//}

BOOST_AUTO_TEST_SUITE_END()
}
