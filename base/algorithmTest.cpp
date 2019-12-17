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
#include "base/efg.h"
#include "base/cache.h"
#include "base/algorithm.h"

#include "gtest/gtest.h"
#include "domains/goofSpiel.h"

namespace GTLib2 {

using domains::GoofSpielDomain;
using domains::GoofSpielSettings;
using domains::GoofSpielVariant::IncompleteObservations;

TEST(PlayMatch, PlaySmallGame) {
    int a = 0;
    int b = -1;
    PreparedAlgorithm firstAction = createInitializer<FixedActionPlayer>(a);
    PreparedAlgorithm lastAction = createInitializer<FixedActionPlayer>(b);

    GoofSpielSettings settings({/*.variant=*/IncompleteObservations, /*.numCards=*/3,
                                   /*.fixChanceCards=*/true, /*.chanceCards=*/{}});
    settings.shuffleChanceCards(0);
    GoofSpielDomain domain(settings);
    vector<double> expectedUtilities = vector<double>{1., -1.};
    vector<double> actualUtilities = playMatch(
        domain, vector<PreparedAlgorithm>{firstAction, lastAction},
        vector<unsigned int>{1000, 1000}, vector<unsigned int>{10, 10}, BudgetIterations, 0);
    EXPECT_EQ(actualUtilities, expectedUtilities);
}

}  // namespace GTLib2
