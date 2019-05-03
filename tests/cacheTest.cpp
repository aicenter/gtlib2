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

#include "gtest/gtest.h"


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielObservation;
using domains::GoofspielRoundOutcome;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::MatchingPenniesAction;
using domains::ActionHeads;
using domains::ActionTails;
using algorithms::createRootEFGNodes;


TEST(Cache, CacheHit) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNodes = createRootEFGNodes(
        mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    EXPECT_TRUE(!cache.hasNode(rootNode));
    EXPECT_TRUE(!cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));

    rootNode->performAction(actions[0]);
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));

    cache.getChildrenFor(rootNode, actions[0]);
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
    cache.getChildrenFor(rootNode, actions[1]);
    EXPECT_TRUE(cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));

    // check that getting children doesn't create new uses of shared pointer
    long old_use_cnt = rootNode.use_count();
    cache.getChildrenFor(rootNode, actions[0]);
    EXPECT_TRUE(old_use_cnt - rootNode.use_count() == 0);
}


TEST(Cache, BuildCacheMaxDepth) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    EXPECT_TRUE(!cache.hasNode(rootNode));
    EXPECT_TRUE(!cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

    rootNode->performAction(actions[0]);
    rootNode->performAction(actions[1]);
    EXPECT_TRUE(!cache.hasNode(rootNode));
    EXPECT_TRUE(!cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

    cache.buildForest();
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
    // todo: make test(s) based on comparing domain statistics
}


TEST(Cache, BuildCacheLimitedDepth) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
    InfosetCache cache(mp);

    auto rootNode = rootNodes[0].first;
    auto actions = rootNode->availableActions();
    EXPECT_TRUE(!cache.hasNode(rootNode));
    EXPECT_TRUE(!cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

    auto node0 = rootNode->performAction(actions[0])[0].first;
    auto node1 = rootNode->performAction(actions[1])[0].first;
    EXPECT_TRUE(!cache.hasNode(rootNode));
    EXPECT_TRUE(!cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

    cache.buildForest(0);
    EXPECT_TRUE(!cache.hasNode(rootNode));
    EXPECT_TRUE(!cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));
    EXPECT_TRUE(!cache.hasAllChildren(node0));
    EXPECT_TRUE(!cache.hasAllChildren(node1));

    cache.buildForest(1);
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
    EXPECT_TRUE(!cache.hasAllChildren(node0));
    EXPECT_TRUE(!cache.hasAllChildren(node1));

    cache.buildForest(2);
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
    EXPECT_TRUE(cache.hasAllChildren(node0));
    EXPECT_TRUE(cache.hasAllChildren(node1));
}

TEST(Cache, BuildPublicStateCache) {
    MatchingPenniesDomain domains[] = {MatchingPenniesDomain(AlternatingMoves),
                                       MatchingPenniesDomain(SimultaneousMoves)};
    for (const auto &mp : domains) {
        auto rootNodes = createRootEFGNodes(mp.getRootStatesDistribution());
        PublicStateCache cache(mp);

        auto rootNode = rootNodes[0].first;
        auto actions = rootNode->availableActions();
        EXPECT_TRUE(!cache.hasNode(rootNode));
        EXPECT_TRUE(!cache.hasPublicState(rootNode->getPublicState()));

        rootNode->performAction(actions[0]);
        rootNode->performAction(actions[1]);
        EXPECT_TRUE(!cache.hasNode(rootNode));
        EXPECT_TRUE(!cache.hasPublicState(rootNode->getPublicState()));

        cache.buildForest();
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasPublicState(rootNode->getPublicState()));
        EXPECT_TRUE(cache.countPublicStates() == 4);
    }
}

TEST(Cache, BuildLargePublicStateCache) {
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
                EXPECT_TRUE(cache.countPublicStates() == 11);
                break;
            case 3:
                EXPECT_TRUE(cache.countPublicStates() == 39);
                break;
            case 4:
                EXPECT_TRUE(cache.countPublicStates() == 131);
                break;
        }
    }
}

TEST(Cache, PublicStateCacheGetInfosets) {
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
    EXPECT_TRUE(cache.getNodesFor(pubState).size() == 2);
    EXPECT_TRUE(cache.getNodesFor(pubState) == children);

    shared_ptr<AOH> actualInfoset = aNode->getAOHInfSet();
    shared_ptr<AOH> expectedInfoset = *cache.getInfosetsFor(pubState, Player(1)).begin();
    EXPECT_TRUE(expectedInfoset != actualInfoset);
    EXPECT_TRUE(*expectedInfoset == *actualInfoset);

    actualInfoset = aNode->getAOHAugInfSet(Player(0));
    expectedInfoset = *cache.getInfosetsFor(pubState, Player(0)).begin();
    EXPECT_TRUE(expectedInfoset != actualInfoset);
    EXPECT_TRUE(*expectedInfoset == *actualInfoset);
}

TEST(Cache, PublicStateCacheGetInfosetsLarge) {
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
    EXPECT_TRUE(cache.getNodesFor(pubState).size() == 24); // 3! * 4

    auto expectedInfosets = cache.getInfosetsFor(pubState, Player(0));
    cout << expectedInfosets.size() << " ";
    EXPECT_TRUE(expectedInfosets.size() == 12);

    expectedInfosets = cache.getInfosetsFor(pubState, Player(1));
    cout << expectedInfosets.size() << " ";
    EXPECT_TRUE(expectedInfosets.size() == 12);

    const vector<shared_ptr<Observation>> obsHistory = pubState->getPublicHistory();
    const vector<shared_ptr<GoofSpielObservation>>
        goofObsHistory = Cast<Observation, GoofSpielObservation>(obsHistory);
    for (const auto &obs : goofObsHistory) {
        EXPECT_TRUE(obs->roundResult_ == GoofspielRoundOutcome::PL0_DRAW);
    }
}

}  // namespace GTLib2
