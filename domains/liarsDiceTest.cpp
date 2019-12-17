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

#include "algorithms/stats.h"
#include "domains/liarsDice.h"

#include "gtest/gtest.h"

namespace GTLib2::domains {

using algorithms::DomainStatistics;


// @formatter:off
LiarsDiceDomain testDomainsLiarsDice[] = { // NOLINT(cert-err58-cpp)
    LiarsDiceDomain({2, 0}, 2),
    LiarsDiceDomain({0, 2}, 2),
    LiarsDiceDomain({2, 2}, 2),
    LiarsDiceDomain({3, 0}, 3),
    LiarsDiceDomain({1, 0}, 2),
    LiarsDiceDomain({1, 0}, 3),
    LiarsDiceDomain({2, 1}, 4),
};
// @formatter:on


TEST(LiarsDice, BuildGameTreeAndCheckSizes) {
    vector<DomainStatistics> expectedStats = {
        {
            .max_EFGDepth   = 6,
            .max_StateDepth = 6,
            .num_nodes      = 94,
            .num_terminals  = 45,
            .num_states     = 94,
            .num_histories  = {24, 24},
            .num_infosets   = {24, 8},
            .num_sequences  = {46, 16},
        },
        {
            .max_EFGDepth   = 6,
            .max_StateDepth = 6,
            .num_nodes      = 94,
            .num_terminals  = 45,
            .num_states     = 94,
            .num_histories  = {24, 24},
            .num_infosets   = {8, 24},
            .num_sequences  = {16, 46},
        },
        {
            .max_EFGDepth   = 10,
            .max_StateDepth = 10,
            .num_nodes      = 4600,
            .num_terminals  = 2295,
            .num_states     = 4600,
            .num_histories  = {1152, 1152},
            .num_infosets   = {384, 384},
            .num_sequences  = {766, 766},
        },
        {
            .max_EFGDepth   = 11,
            .max_StateDepth = 11,
            .num_nodes      = 10231,
            .num_terminals  = 5110,
            .num_states     = 10231,
            .num_histories  = {2560, 2560},
            .num_infosets   = {2560, 256},
            .num_sequences  = {5111, 512},
        },
        {
            .max_EFGDepth   = 4,
            .max_StateDepth = 4,
            .num_nodes      = 15,
            .num_terminals  = 6,
            .num_states     = 15,
            .num_histories  = {4, 4},
            .num_infosets   = {4, 2},
            .num_sequences  = {7, 4},
        },
        {
            .max_EFGDepth   = 5,
            .max_StateDepth = 5,
            .num_nodes      = 46,
            .num_terminals  = 21,
            .num_states     = 46,
            .num_histories  = {12, 12},
            .num_infosets   = {12, 4},
            .num_sequences  = {22, 8},
        },
        {
            .max_EFGDepth   = 14,
            .max_StateDepth = 14,
            .num_nodes      = 327641,
            .num_terminals  = 163800,
            .num_states     = 327641,
            .num_histories  = {81920, 81920},
            .num_infosets   = {20480, 8192},
            .num_sequences  = {40951, 16381},
        }
    };

    for (unsigned int i = 0; i < expectedStats.size(); ++i) {
        cout << ">> checking domain [" << i << "] " << testDomainsLiarsDice[i].getInfo() << endl;
        DomainStatistics actualStats;
        calculateDomainStatistics(testDomainsLiarsDice[i], &actualStats);
        EXPECT_EQ(actualStats, expectedStats[i]);
    }
}

}  // namespace GTLib2
