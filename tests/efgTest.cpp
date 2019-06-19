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
#include "algorithms/strategy.h"

#include "algorithms/utility.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"

#include "tests/domainsTest.h"

#include "gtest/gtest.h"


namespace GTLib2 {

using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;

TEST(EFG, CheckActionObservationIds) {
    auto iigs2_seed = GoofSpielDomain({
                                          variant:  IncompleteObservations,
                                          numCards: 2,
                                          fixChanceCards: true,
                                          chanceCards: {}
                                      });
    auto root = createRootEFGNode(iigs2_seed);
    auto child0 = root->performAction(root->availableActions()[0]);
    auto child1 = root->performAction(root->availableActions()[1]);
    auto ao00 = child0->getAOids(Player(0));
    auto ao01 = child0->getAOids(Player(1));
    auto ao10 = child1->getAOids(Player(0));
    auto ao11 = child1->getAOids(Player(1));
    EXPECT_EQ(ao01, ao11);
    EXPECT_NE(ao00, ao10);

    auto infoset00 = child0->getAOHAugInfSet(Player(0));
    auto infoset01 = child0->getAOHAugInfSet(Player(1));
    auto infoset10 = child1->getAOHAugInfSet(Player(0));
    auto infoset11 = child1->getAOHAugInfSet(Player(1));
    EXPECT_EQ(*infoset01, *infoset11);
    EXPECT_NE(*infoset00, *infoset10);
}

}
