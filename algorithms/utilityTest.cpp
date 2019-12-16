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
#include "algorithms/stats.h"
#include "algorithms/strategy.h"

#include "algorithms/utility.h"
#include "domains/goofSpiel.h"

#include "gtest/gtest.h"

namespace GTLib2::algorithms {

using domains::GoofSpielDomain;
using domains::GoofSpielAction;
using domains::GoofSpielVariant::CompleteObservations;


TEST(Utility, ComputeUtilityFullDepthCard4) {
    GoofSpielDomain domain
        ({variant: CompleteObservations, numCards: 4, fixChanceCards: false, chanceCards: {}});

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain);
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1);
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2);
    auto thirdLowestCardAction = make_shared<GoofSpielAction>(0, 3);
    auto fourthLowestCardAction = make_shared<GoofSpielAction>(0, 4);

    auto setAction = [&](shared_ptr<EFGNode> node) {
        if(node->type_ != PlayerNode) return;

        auto infoset = node->getAOHInfSet();
        if (node->efgDepth() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->efgDepth() == 2) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        } else if (node->efgDepth() == 4) {
            playOnlyAction(profile[opponent][infoset], thirdLowestCardAction);
        } else if (node->efgDepth() == 6) {
            playOnlyAction(profile[opponent][infoset], fourthLowestCardAction);
        } else if (node->efgDepth() == 1) {
            playOnlyAction(profile[player][infoset], lowestCardAction);
        } else if (node->efgDepth() == 3) {
            playOnlyAction(profile[player][infoset], secondLowestCardAction);
        } else if (node->efgDepth() == 5) {
            playOnlyAction(profile[player][infoset], thirdLowestCardAction);
        } else if (node->efgDepth() == 7) {
            playOnlyAction(profile[player][infoset], fourthLowestCardAction);
        }

    };
    treeWalk(domain, setAction);

    auto utility = algorithms::computeUtilitiesTwoPlayerGame(domain, profile)[1];
    EXPECT_LE(std::abs(utility - 0), 0.001);
}

}  // namespace GTLib2
