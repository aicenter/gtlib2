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


#include "base/efg.h"

#include "algorithms/common.h"
#include "algorithms/tree.h"
#include "domains/matching_pennies.h"
#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::Heads;
using domains::Tails;
using algorithms::createRootEFGNodes;
using algorithms::buildTree;

BOOST_AUTO_TEST_SUITE(EFGTests)

BOOST_AUTO_TEST_CASE(BuildTree) {
    MatchingPenniesDomain mp;
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    EFGCache cache(rootNodes);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    rootNode->performAction(actions[0]);
    rootNode->performAction(actions[0]);
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    buildTree(&cache, mp.getMaxDepth());

    BOOST_CHECK(cache.hasChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    // todo: make test(s) based on comparing domain statistics
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
