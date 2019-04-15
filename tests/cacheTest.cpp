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

#include "algorithms/common.h"
#include "domains/matching_pennies.h"
#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::Heads;
using domains::Tails;
using algorithms::createRootEFGNodes;

BOOST_AUTO_TEST_SUITE(BaseTests)
BOOST_AUTO_TEST_SUITE(CacheTests)

BOOST_AUTO_TEST_CASE(CacheHit) {
    MatchingPenniesDomain mp;
    auto rootNodes = createRootEFGNodes(
        mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));

    rootNode->performAction(actions[0]);
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));

    cache.getChildrenFor(rootNode, actions[0]);
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    cache.getChildrenFor(rootNode, actions[1]);
    BOOST_CHECK(cache.hasChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));

    // check that getting children doesn't create new uses of shared pointer
    long old_use_cnt = rootNode.use_count();
    cache.getChildrenFor(rootNode, actions[0]);
    BOOST_CHECK(old_use_cnt - rootNode.use_count() == 0);
}


BOOST_AUTO_TEST_CASE(BuildCacheMaxDepth) {
    MatchingPenniesDomain mp;
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    rootNode->performAction(actions[0]);
    rootNode->performAction(actions[1]);
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    cache.buildForest();
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(cache.hasChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    // todo: make test(s) based on comparing domain statistics
}


BOOST_AUTO_TEST_CASE(BuildCacheLimitedDepth) {
    MatchingPenniesDomain mp;
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    auto node0 = rootNode->performAction(actions[0])[0].first;
    auto node1 = rootNode->performAction(actions[1])[0].first;
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    cache.buildForest(0);
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));
    BOOST_CHECK(!cache.hasChildren(node0));
    BOOST_CHECK(!cache.hasChildren(node1));

    cache.buildForest(1);
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(cache.hasChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    BOOST_CHECK(!cache.hasChildren(node0));
    BOOST_CHECK(!cache.hasChildren(node1));

    cache.buildForest(2);
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(cache.hasChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    BOOST_CHECK(cache.hasChildren(node0));
    BOOST_CHECK(cache.hasChildren(node1));
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
