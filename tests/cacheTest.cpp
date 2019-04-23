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

#include "domains/matching_pennies.h"
#include "domains/goofSpiel.h"
#include <boost/test/unit_test.hpp>


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielObservation;
using domains::GoofspielRoundOutcome;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::MatchingPenniesAction;
using domains::Heads;
using domains::Tails;
using algorithms::createRootEFGNodes;

BOOST_AUTO_TEST_SUITE(BaseTests)
BOOST_AUTO_TEST_SUITE(CacheTests)

BOOST_AUTO_TEST_CASE(CacheHit) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNodes = createRootEFGNodes(
        mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));

    rootNode->performAction(actions[0]);
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));

    cache.getChildrenFor(rootNode, actions[0]);
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    cache.getChildrenFor(rootNode, actions[1]);
    BOOST_CHECK(cache.hasAllChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));

    // check that getting children doesn't create new uses of shared pointer
    long old_use_cnt = rootNode.use_count();
    cache.getChildrenFor(rootNode, actions[0]);
    BOOST_CHECK(old_use_cnt - rootNode.use_count() == 0);
}


BOOST_AUTO_TEST_CASE(BuildCacheMaxDepth) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    rootNode->performAction(actions[0]);
    rootNode->performAction(actions[1]);
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    cache.buildForest();
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(cache.hasAllChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    // todo: make test(s) based on comparing domain statistics
}


BOOST_AUTO_TEST_CASE(BuildCacheLimitedDepth) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    auto node0 = rootNode->performAction(actions[0])[0].first;
    auto node1 = rootNode->performAction(actions[1])[0].first;
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));

    cache.buildForest(0);
    BOOST_CHECK(!cache.hasNode(rootNode));
    BOOST_CHECK(!cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(!cache.hasAllChildren(rootNode));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(!cache.hasChildren(rootNode, actions[1]));
    BOOST_CHECK(!cache.hasAllChildren(node0));
    BOOST_CHECK(!cache.hasAllChildren(node1));

    cache.buildForest(1);
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(cache.hasAllChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    BOOST_CHECK(!cache.hasAllChildren(node0));
    BOOST_CHECK(!cache.hasAllChildren(node1));

    cache.buildForest(2);
    BOOST_CHECK(cache.hasNode(rootNode));
    BOOST_CHECK(cache.hasInfoset(rootNode->getAOHInfSet()));
    BOOST_CHECK(cache.hasAllChildren(rootNode));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[0]));
    BOOST_CHECK(cache.hasChildren(rootNode, actions[1]));
    BOOST_CHECK(cache.hasAllChildren(node0));
    BOOST_CHECK(cache.hasAllChildren(node1));
}

BOOST_AUTO_TEST_CASE(BuildPublicStateCache) {
    MatchingPenniesDomain domains[] = {MatchingPenniesDomain(AlternatingMoves),
                                       MatchingPenniesDomain(SimultaneousMoves)};
    for (const auto &mp : domains) {
        auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
        PublicStateCache cache(mp);

        auto rootNode = rootNodes[0].first;
        auto actions = rootNode->availableActions();
        BOOST_CHECK(!cache.hasNode(rootNode));
        BOOST_CHECK(!cache.hasPublicState(rootNode->getPublicState()));

        rootNode->performAction(actions[0]);
        rootNode->performAction(actions[1]);
        BOOST_CHECK(!cache.hasNode(rootNode));
        BOOST_CHECK(!cache.hasPublicState(rootNode->getPublicState()));

        cache.buildForest();
        BOOST_CHECK(cache.hasNode(rootNode));
        BOOST_CHECK(cache.hasPublicState(rootNode->getPublicState()));
        BOOST_CHECK(cache.countPublicStates() == 4);
    }
}

BOOST_AUTO_TEST_CASE(BuildLargePublicStateCache) {
    GoofSpielDomain domains[] = {
        GoofSpielDomain({
                            variant:  IncompleteObservations,
                            numCards: 2,
                            fixChanceCards: true,
                            chanceCards: {}
                        }),
        GoofSpielDomain({
                            variant:  IncompleteObservations,
                            numCards: 3,
                            fixChanceCards: true,
                            chanceCards: {}
                        }),
        GoofSpielDomain({
                            variant:  IncompleteObservations,
                            numCards: 4,
                            fixChanceCards: true,
                            chanceCards: {}
                        }),
    };

    for (const auto &domain : domains) {
        PublicStateCache cache(domain);
        cache.buildForest();
        switch (domain.numberOfCards_) {
            case 2:
                BOOST_CHECK(cache.countPublicStates() == 11);
                break;
            case 3:
                BOOST_CHECK(cache.countPublicStates() == 39);
                break;
            case 4:
                BOOST_CHECK(cache.countPublicStates() == 131);
                break;
        }
    }
}

BOOST_AUTO_TEST_CASE(PublicStateCacheGetInfosets) {
    GoofSpielDomain domain({
                               variant:  IncompleteObservations,
                               numCards: 2,
                               fixChanceCards: true,
                               chanceCards: {}
                           });
    PublicStateCache cache(domain);
    cache.buildForest();

    auto rootNode = cache.getRootNodes()[0].first;
    auto childNodes = cache.getChildrenFor(rootNode);
    shared_ptr<EFGNode> aNode = (*childNodes[0])[0].first;
    shared_ptr<EFGNode> bNode = (*childNodes[1])[0].first;
    auto children = unordered_set<shared_ptr<EFGNode>>{aNode, bNode};
    auto pubState = cache.getPublicStateFor(aNode);
    BOOST_CHECK(cache.getNodesFor(pubState).size() == 2);
    BOOST_CHECK(cache.getNodesFor(pubState) == children);

    shared_ptr<AOH> actualInfoset = aNode->getAOHInfSet();
    shared_ptr<AOH> expectedInfoset = *cache.getInfosetsFor(pubState, Player(1)).begin();
    BOOST_CHECK(expectedInfoset != actualInfoset);
    BOOST_CHECK(*expectedInfoset == *actualInfoset);

    actualInfoset = aNode->getAOHAugInfSet(Player(0));
    expectedInfoset = *cache.getInfosetsFor(pubState, Player(0)).begin();
    BOOST_CHECK(expectedInfoset != actualInfoset);
    BOOST_CHECK(*expectedInfoset == *actualInfoset);
}

BOOST_AUTO_TEST_CASE(PublicStateCacheGetInfosetsLarge) {
    GoofSpielDomain domain({
                               variant:  IncompleteObservations,
                               numCards: 4,
                               fixChanceCards: true,
                               chanceCards: {}
                           });
    PublicStateCache cache(domain);
    cache.buildForest();

    auto rootNode = cache.getRootNodes()[0].first;
    auto aNode = (*cache.getChildrenFor(rootNode)[0])[0].first;
    auto bNode = (*cache.getChildrenFor(aNode)[0])[0].first;
    auto cNode = (*cache.getChildrenFor(bNode)[0])[0].first;
    auto dNode = (*cache.getChildrenFor(cNode)[0])[0].first;
    auto eNode = (*cache.getChildrenFor(dNode)[0])[0].first;
    // eNode == draw outcome 3 times in a row

    auto pubState = cache.getPublicStateFor(eNode);
    cout << cache.getNodesFor(pubState).size() << " ";
    BOOST_CHECK(cache.getNodesFor(pubState).size() == 24); // 3! * 4

    auto expectedInfosets = cache.getInfosetsFor(pubState, Player(0));
    cout << expectedInfosets.size() << " ";
    BOOST_CHECK(expectedInfosets.size() == 12);

    expectedInfosets = cache.getInfosetsFor(pubState, Player(1));
    cout << expectedInfosets.size() << " ";
    BOOST_CHECK(expectedInfosets.size() == 12);

    const vector<shared_ptr<Observation>> obsHistory = pubState->getPublicHistory();
    const vector<shared_ptr<GoofSpielObservation>> goofObsHistory = Cast<Observation, GoofSpielObservation>(obsHistory);
    for(const auto& obs : goofObsHistory) {
        BOOST_CHECK(obs->roundResult_ == GoofspielRoundOutcome::PL0_DRAW);
    }
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
