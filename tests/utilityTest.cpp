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

#include <boost/test/unit_test.hpp>

namespace GTLib2 {

using domains::GoofSpielDomain;
using domains::GoofSpielAction;
using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;
using algorithms::getUniformStrategy;
using algorithms::playOnlyAction;
using std::unordered_set;

BOOST_AUTO_TEST_SUITE(Utility)

BOOST_AUTO_TEST_CASE(computeUtilityFullDepthCard4) {
    GoofSpielDomain domain(4, 4, nullopt);

    auto player = Player(1);
    auto opponent = Player(0);

    InfosetCache cache(domain.getRootStatesDistribution());
    StrategyProfile profile = getUniformStrategy(cache);

    auto lowestCardAction = make_shared<GoofSpielAction>(0, 1)->getId();
    auto secondLowestCardAction = make_shared<GoofSpielAction>(0, 2)->getId();
    auto thirdLowestCardAction = make_shared<GoofSpielAction>(0, 3)->getId();
    auto fourthLowestCardAction = make_shared<GoofSpielAction>(0, 4)->getId();

    auto setAction = [&](shared_ptr<EFGNode> node) {
        auto infoset = node->getAOHInfSet();
        if (node->getDistanceFromRoot() == 0) {
            playOnlyAction(profile[opponent][infoset], lowestCardAction);
        } else if (node->getDistanceFromRoot() == 2) {
            playOnlyAction(profile[opponent][infoset], secondLowestCardAction);
        } else if (node->getDistanceFromRoot() == 4) {
            playOnlyAction(profile[opponent][infoset], thirdLowestCardAction);
        } else if (node->getDistanceFromRoot() == 6) {
            playOnlyAction(profile[opponent][infoset], fourthLowestCardAction);
        } else if (node->getDistanceFromRoot() == 1) {
            playOnlyAction(profile[player][infoset], lowestCardAction);
        } else if (node->getDistanceFromRoot() == 3) {
            playOnlyAction(profile[player][infoset], secondLowestCardAction);
        } else if (node->getDistanceFromRoot() == 5) {
            playOnlyAction(profile[player][infoset], thirdLowestCardAction);
        } else if (node->getDistanceFromRoot() == 7) {
            playOnlyAction(profile[player][infoset], fourthLowestCardAction);
        }

    };
    algorithms::treeWalkEFG(domain, setAction);

    auto utility = algorithms::computeUtilityTwoPlayersGame(domain, profile[0], profile[1],
                                                            Player(0), Player(1));

    BOOST_CHECK(std::abs(utility.second - 0) <= 0.001);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
