//
// Created by filip on 31.5.19.
//

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
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"
#include "algorithms/tree.h"
#include "algorithms/stats.h"
#include "algorithms/utility.h"
#include "algorithms/strategy.h"
#include "domains/normal_form_game.h"

#include "LPsolvers/LPSolver.h"
#include "tests/domainsTest.h"

#include "gtest/gtest.h"

namespace GTLib2::domains {

using algorithms::DomainStatistics;
using algorithms::playOnlyAction;

TEST(NormalFormGame, buildGameTreeAndCheckSizesMatrix) {
    DomainStatistics expectedStat = {
        .max_EFGDepth   = 2,
        .max_StateDepth = 2,
        .num_nodes      = 7,
        .num_terminals  = 4,
        .num_states     = 5,
        .num_histories  = {1, 2},
        .num_infosets   = {1, 1},
        .num_sequences  = {3, 3},
    };

    auto nfgSettings1 = NFGSettings();
    nfgSettings1.utilityMatrix = {{-1, -3}, {0, -2}};
    nfgSettings1.dimensions = {2, 2};
    auto prisonersDilemma =
        NFGDomain(nfgSettings1);
    DomainStatistics actualStat;

    calculateDomainStatistics(prisonersDilemma, &actualStat);

    EXPECT_EQ(actualStat, expectedStat);
}

TEST(NormalFormGame, buildGameTreeAndCheckSizesVector) {
    DomainStatistics expectedStat = {
        .max_EFGDepth   = 2,
        .max_StateDepth = 2,
        .num_nodes      = 13,
        .num_terminals  = 9,
        .num_states     = 10,
        .num_histories  = {1, 3},
        .num_infosets   = {1, 1},
        .num_sequences  = {4, 4},
    };

    auto rpsSettings = NFGSettings();
    rpsSettings.numPlayers = 2;
    rpsSettings.inputVariant = VectorOFUtilities;
    rpsSettings.dimensions = {3, 3};
    rpsSettings.utilities = {{0, 0}, {-1, 1}, {1, -1}, {1, -1}, {0, 0}, {-1, 1}, {-1, 1}, {1, -1}, {0, 0}};

    auto rps = NFGDomain(rpsSettings);

    DomainStatistics actualStat;
    calculateDomainStatistics(rps, &actualStat);

    EXPECT_EQ(actualStat, expectedStat);
}

}  // namespace GTLib2
