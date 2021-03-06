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


#include "LPsolvers/LPSolver.h"
#if LP_SOLVER != NO_LP_SOLVER

#include "base/base.h"
#include "algorithms/bestResponse.h"
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"

#include "algorithms/utility.h"
#include "algorithms/stats.h"
#include "algorithms/strategy.h"
#include "domains/goofSpiel.h"

#include "domains/matching_pennies.h"
#include "tests/domainsTest.h"
#include "gtest/gtest.h"


namespace GTLib2::algorithms {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::ActionHeads;
using domains::ActionTails;


TEST(LPSolver, BestResponseToEquilibrium) {
    MatchingPenniesDomain domain(AlternatingMoves);
    auto stratValue = algorithms::findEquilibriumTwoPlayersZeroSum(domain);
    auto brsVal = algorithms::bestResponseTo(stratValue.strategy, Player(1), domain);
    EXPECT_EQ(stratValue.value, 0.0);
}

TEST(LPSolver, EquilibriumNormalFormLP) {
    MatchingPenniesDomain domain(AlternatingMoves);
    auto stratValue = algorithms::findEquilibriumTwoPlayersZeroSum(domain);
    auto actionHeads = make_shared<MatchingPenniesAction>(ActionHeads);
    auto actionTails = make_shared<MatchingPenniesAction>(ActionTails);
    double headsProb = (*stratValue.strategy.begin()).second[actionHeads];
    double tailsProb = (*stratValue.strategy.begin()).second[actionTails];

    EXPECT_EQ(stratValue.value, 0);
    EXPECT_EQ(headsProb, 0.5);
    EXPECT_EQ(tailsProb, 0.5);
}

}  // namespace GTLib2

#endif // LP_SOLVER != NO_LP_SOLVER
