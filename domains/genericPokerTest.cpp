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
#include "algorithms/cfr.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/utility.h"
#include "algorithms/stats.h"
#include "domains/genericPoker.h"

#include "gtest/gtest.h"


namespace GTLib2::domains {

using algorithms::DomainStatistics;

GenericPokerDomain testDomainsPoker[] = { // NOLINT(cert-err58-cpp)
    GenericPokerDomain(3, 1, 1, 1, 1),
    GenericPokerDomain(1, 3, 1, 1, 1),
    GenericPokerDomain(2, 2, 2, 2, 2),
    GenericPokerDomain(3, 3, 1, 2, 3),
};

TEST(Poker, BuildGameTreeAndCheckSizes) {
    vector<DomainStatistics> expectedStats = {
        {
            .max_EFGDepth   = 9,
            .max_StateDepth = 9,
            .num_nodes      = 511,
            .num_terminals  = 294,
            .num_states     = 511,
            .num_histories  = {108, 108},
            .num_infosets   = {99, 99},
            .num_sequences  = {232, 232},
        },{
            .max_EFGDepth   = 8,
            .max_StateDepth = 9,
            .num_nodes      = 85,
            .num_terminals  = 49,
            .num_states     = 85,
            .num_histories  = {18, 18},
            .num_infosets   = {18, 18},
            .num_sequences  = {43, 43},
        },{
            .max_EFGDepth   = 12,
            .max_StateDepth = 11,
            .num_nodes      = 15429,
            .num_terminals  = 10030,
            .num_states     = 15371,
            .num_histories  = {2670, 2670},
            .num_infosets   = {1770, 1770},
            .num_sequences  = {5075, 5075},
        }, {
            .max_EFGDepth   = 10,
            .max_StateDepth = 9,
            .num_nodes      = 23869,
            .num_terminals  = 15291,
            .num_states     = 23716,
            .num_histories  = {4212, 4212},
            .num_infosets   = {1404, 1404},
            .num_sequences  = {3901, 3901},
        }
    };

    for (int i = 0; i < expectedStats.size(); ++i) {
        cout << ">> checking domain [" << i << "] " << testDomainsPoker[i].getInfo() << endl;
        DomainStatistics actualStats;
        calculateDomainStatistics(testDomainsPoker[i], &actualStats);
        EXPECT_EQ(actualStats, expectedStats[i]);
    }
}

// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!



}
