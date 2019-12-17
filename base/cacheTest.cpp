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
#include "domains/randomGame.h"

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


TEST(Cache, CacheHit) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNode = createRootEFGNode(mp);
    InfosetCache cache(mp);

    auto actions = rootNode->availableActions();
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));

    rootNode->performAction(actions[0]);
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));

    cache.getChildFor(rootNode, actions[0]);
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
    cache.getChildFor(rootNode, actions[1]);
    EXPECT_TRUE(cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));

    // check that getting children doesn't create new uses of shared pointer
    long old_use_cnt = rootNode.use_count();
    cache.getChildFor(rootNode, actions[0]);
    EXPECT_EQ(old_use_cnt - rootNode.use_count(), 0);
}


TEST(Cache, BuildCacheMaxDepth) {
    MatchingPenniesDomain mp(AlternatingMoves);
    auto rootNode = createRootEFGNode(mp);
    InfosetCache cache(mp);

    auto actions = rootNode->availableActions();
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

    rootNode->performAction(actions[0]);
    rootNode->performAction(actions[1]);
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(!cache.hasAllChildren(rootNode));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

    cache.buildTree();
    EXPECT_TRUE(cache.hasNode(rootNode));
    EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
    EXPECT_TRUE(cache.hasAllChildren(rootNode));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
    EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
    // todo: make test(s) based on comparing domain statistics
}


TEST(Cache, BuildCacheLimitedDepth) {
    MatchingPenniesDomain domains[] = {
        MatchingPenniesDomain(AlternatingMoves),
        MatchingPenniesDomain(SimultaneousMoves)
    };
    for (const auto &mp : domains) {
        auto rootNode = createRootEFGNode(mp);
        InfosetCache cache(mp);

        auto actions = rootNode->availableActions();
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(!cache.hasAllChildren(rootNode));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

        auto node0 = rootNode->performAction(actions[0]);
        auto node1 = rootNode->performAction(actions[1]);
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(!cache.hasAllChildren(rootNode));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));

        cache.getRootNode();
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(!cache.hasAllChildren(rootNode));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));
        EXPECT_TRUE(!cache.hasAllChildren(node0));
        EXPECT_TRUE(!cache.hasAllChildren(node1));

        cache.buildTree(0);
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(!cache.hasAllChildren(rootNode));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(!cache.hasChildren(rootNode, actions[1]));
        EXPECT_TRUE(!cache.hasNode(node0));
        EXPECT_TRUE(!cache.hasNode(node1));
        EXPECT_TRUE(!cache.hasAllChildren(node0));
        EXPECT_TRUE(!cache.hasAllChildren(node1));

        cache.buildTree(1);
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(cache.hasAllChildren(rootNode));
        EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
        EXPECT_TRUE(cache.hasNode(node0));
        EXPECT_TRUE(cache.hasNode(node1));
        EXPECT_TRUE(!cache.hasAllChildren(node0));
        EXPECT_TRUE(!cache.hasAllChildren(node1));

        cache.buildTree(2);
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(cache.hasAllChildren(rootNode));
        EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
        EXPECT_TRUE(cache.hasNode(node0));
        EXPECT_TRUE(cache.hasNode(node1));
        EXPECT_TRUE(cache.hasAllChildren(node0));
        EXPECT_TRUE(cache.hasAllChildren(node1));

        cache.buildTree(3); // over the depth. MP has EFG depth of 2
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasInfoset(rootNode->getAOHInfSet()));
        EXPECT_TRUE(cache.hasAllChildren(rootNode));
        EXPECT_TRUE(cache.hasChildren(rootNode, actions[0]));
        EXPECT_TRUE(cache.hasChildren(rootNode, actions[1]));
        EXPECT_TRUE(cache.hasNode(node0));
        EXPECT_TRUE(cache.hasNode(node1));
        EXPECT_TRUE(cache.hasAllChildren(node0));
        EXPECT_TRUE(cache.hasAllChildren(node1));
    }
}

TEST(Cache, BuildPublicStateCache) {
    MatchingPenniesDomain domains[] = {MatchingPenniesDomain(AlternatingMoves),
                                       MatchingPenniesDomain(SimultaneousMoves)};
    for (const auto &mp : domains) {
        auto rootNode = createRootEFGNode(mp);
        PublicStateCache cache(mp);

        auto actions = rootNode->availableActions();
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasPublicState(rootNode->getPublicState()));

        rootNode->performAction(actions[0]);
        rootNode->performAction(actions[1]);
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasPublicState(rootNode->getPublicState()));

        cache.buildTree();
        EXPECT_TRUE(cache.hasNode(rootNode));
        EXPECT_TRUE(cache.hasPublicState(rootNode->getPublicState()));
        EXPECT_EQ(cache.countPublicStates(), 4);
    }
}

TEST(Cache, BuildLargePublicStateCache) {
    GoofSpielDomain domains[] = {
        GoofSpielDomain({
                            /*.variant=*/ IncompleteObservations,
                            /*.numCards=*/2,
                            /*.fixChanceCards=*/true,
                            /*.chanceCards=*/{}
                        }),
        GoofSpielDomain({
                            /*.variant=*/ IncompleteObservations,
                            /*.numCards=*/3,
                            /*.fixChanceCards=*/true,
                            /*.chanceCards=*/{}
                        }),
        GoofSpielDomain({
                            /*.variant=*/ IncompleteObservations,
                            /*.numCards=*/4,
                            /*.fixChanceCards=*/true,
                            /*.chanceCards=*/{}
                        }),
    };

    for (const auto &domain : domains) {
        PublicStateCache cache(domain);
        cache.buildTree();
        switch (domain.numberOfCards_) {
            case 2:
                EXPECT_EQ(cache.countPublicStates(), 11);
                break;
            case 3:
                EXPECT_EQ(cache.countPublicStates(), 39);
                break;
            case 4:
                EXPECT_EQ(cache.countPublicStates(), 131);
                break;
        }
    }
}

TEST(Cache, PublicStateCacheGetInfosets) {
    GoofSpielDomain domain({
                               /*.variant=*/ IncompleteObservations,
                               /*.numCards=*/2,
                               /*.fixChanceCards=*/true,
                               /*.chanceCards=*/{}
                           });
    PublicStateCache cache(domain);
    cache.buildTree();

    auto rootNode = cache.getRootNode();
    auto childNodes = cache.getChildrenFor(rootNode);
    shared_ptr<EFGNode> aNode = childNodes[0];
    shared_ptr<EFGNode> bNode = childNodes[1];
    auto children = unordered_set<shared_ptr<EFGNode>>{aNode, bNode};
    auto pubState = cache.getPublicStateFor(aNode);
    EXPECT_EQ(cache.getNodesForPubState(pubState).size(), 2);
    EXPECT_EQ(cache.getNodesForPubState(pubState), children);

    // infoset for player 1 contains the two nodes
    shared_ptr<AOH> actualInfoset = aNode->getAOHInfSet();
    shared_ptr<AOH>
        expectedInfoset = *cache.getInfosetsForPubStatePlayer(pubState, Player(1)).begin();
    EXPECT_NE(expectedInfoset, actualInfoset);
    EXPECT_EQ(*expectedInfoset, *actualInfoset);

    // infoset for player 0 contains two augmented infosets
    auto actualInfosets = unordered_set<AOH>{
        *aNode->getAOHAugInfSet(Player(0)),
        *bNode->getAOHAugInfSet(Player(0)),
    };
    auto cachedInfosets = cache.getInfosetsForPubStatePlayer(pubState, Player(0));
    auto it = cachedInfosets.begin();
    const AOH &x = *(*std::next(it, 0));
    const AOH &y = *(*std::next(it, 1));
    auto expectedInfosets = unordered_set<AOH>{x, y};
    EXPECT_EQ(expectedInfosets, actualInfosets);
}

TEST(Cache, PublicStateTree) {
    const auto numCards = 4;
    unique_ptr <GoofSpielDomain> domain = GoofSpielDomain::IIGS(numCards);
    PublicStateCache cache(*domain);
    cache.buildTree();

    std::function<void(shared_ptr<PublicState>)> walkCheck = [&](shared_ptr<PublicState> node) {
        if (psIsTerminal(cache, node)) return;
        if(node->getDepth() < numCards*2-1) {
            if (node->getDepth() % 2 == 0) EXPECT_EQ(cntPsChildren(cache, node), 1);
            else EXPECT_EQ(cntPsChildren(cache, node), 3);
        } else { // children must be leaves
            for (int i = 0; i < cntPsChildren(cache, node); ++i) {
                auto child = expandPs(cache, node, i);
                EXPECT_TRUE(psIsTerminal(cache, child));
            }
        }

        for (int i = 0; i < cntPsChildren(cache, node); ++i) {
            auto child = expandPs(cache, node, i);
            EXPECT_TRUE(isCompatible(node->getHistory(), child->getHistory()));
            walkCheck(child);
        }
    };
    walkCheck(cache.getRootPublicState());
}

TEST(Cache, PublicStateCacheGetInfosetsLarge) {
    GoofSpielDomain domain({
                               /*.variant=*/ IncompleteObservations,
                               /*.numCards=*/4,
                               /*.fixChanceCards=*/true,
                               /*.chanceCards=*/{}
                           });
    PublicStateCache cache(domain);
    cache.buildTree();

    auto rootNode = cache.getRootNode();
    auto aNode = cache.getChildrenFor(rootNode)[0];
    auto bNode = cache.getChildrenFor(aNode)[0];
    auto cNode = cache.getChildrenFor(bNode)[0];
    auto dNode = cache.getChildrenFor(cNode)[0];
    auto eNode = cache.getChildrenFor(dNode)[0];
    // eNode == draw outcome 3 times in a row

    auto pubState = cache.getPublicStateFor(eNode);
    cout << cache.getNodesForPubState(pubState).size() << " ";
    EXPECT_EQ(cache.getNodesForPubState(pubState).size(), 24); // 3! * 4

    auto expectedInfosets = cache.getInfosetsForPubStatePlayer(pubState, Player(0));
    cout << expectedInfosets.size() << " ";
    EXPECT_EQ(expectedInfosets.size(), 24);

    expectedInfosets = cache.getInfosetsForPubStatePlayer(pubState, Player(1));
    cout << expectedInfosets.size() << " ";
    EXPECT_EQ(expectedInfosets.size(), 12);

    // todo: fix
//    const vector<shared_ptr<Observation>> obsHistory = pubState->getPublicHistory();
//    const vector<shared_ptr<GoofSpielObservation>>
//        goofObsHistory = Cast<Observation, GoofSpielObservation>(obsHistory);
//    for (const auto &obs : goofObsHistory) {
//        EXPECT_EQ(obs->roundResult_, GoofspielRoundOutcome::PL0_DRAW);
//    }
}

TEST(Cache, CollectInformationSetsForBothPlayers) {
    auto rg = GoofSpielDomain::IIGS(2);
    auto rootNode = createRootEFGNode(*rg);
    InfosetCache cache(*rg);
    cache.buildTree();
    auto leader = Player(1);
    auto follower = Player(0);
    unordered_map<shared_ptr<InformationSet>, int> leader_is2idx;
    unordered_map<shared_ptr<InformationSet>, int> follower_is2idx;
    int leader_idx = 0;
    int follower_idx = 0;
    for (const auto &[infoset, nodes]: cache.getInfoset2NodeMapping()) {
        cout << *infoset << ": \n";
        for(auto &node:nodes) {
            cout << "\t" << *node << "\n";
        }
        cout << endl;
    }

    for (const auto &[infoset, nodes]: cache.getInfoset2NodeMapping()) {
        if (nodes.at(0)->type_ != PlayerNode) continue;
        if (infoset->getPlayer() == leader && nodes.at(0)->getPlayer() == leader) {
            leader_is2idx[infoset] = leader_idx;
            leader_idx += nodes.at(0)->countAvailableActions();
        }
        if (infoset->getPlayer() == follower && nodes.at(0)->getPlayer() == follower) {
            follower_is2idx[infoset] = follower_idx ;
            follower_idx  += nodes.at(0)->countAvailableActions();
        }
    }

    EXPECT_EQ(leader_is2idx.size(), 5);
    EXPECT_EQ(follower_is2idx.size(), 5);
}


}  // namespace GTLib2
