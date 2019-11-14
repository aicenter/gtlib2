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

#include <domains/goofSpiel.h>
#include <domains/stratego.h>
#include <algorithms/MCTS/ISMCTS.h>
#include <algorithms/MCTS/CPW_ISMCTS.h>
#include <algorithms/MCTS/DD_ISMCTS.h>
#include "gtest/gtest.h"

namespace GTLib2::algorithms {


TEST(DDISMCTS, CheckInfosetConsistency) {
    // todo:
    auto domain = GoofSpielDomain::IIGS(4);
    auto cache = InfosetCache(*domain);
    cache.buildTree();

    auto mapping = cache.getInfoset2NodeMapping();
    for(auto&[infoset,expectedNodes] : mapping) {
        auto actualNodes = ;/* ... */
        // sort based on vector<ActionId> OR make into sets
        EFGNode n.getHistory();
        EXPECT_EQ(actualNodes, expectedNodes);
    }
}

//Test that GS domain returns correct number of histories for player 1
TEST(DDISMCTS, GSHistoriesGenerationTestpl1) {
    GTLib2::domains::GoofSpielSettings settings
        ({.variant =   GTLib2::domains::IncompleteObservations, .numCards = 6, .fixChanceCards = true});
    domains::GoofSpielDomain domain(settings);
    const auto root = createRootEFGNode(domain);
    auto currnode = root;
    unordered_map<unsigned long, shared_ptr<Constraint>> revealed_;
    domain.initializeEnumerativeConstraints(revealed_);
    currnode = currnode->performAction(currnode->getActionByID(4));//5 (5-6)
    currnode = currnode->performAction(currnode->getActionByID(3));//4 (1-4)
    currnode = currnode->performAction(currnode->getActionByID(0));//1 (1-2)
    currnode = currnode->performAction(currnode->getActionByID(2));//3 (2-5)
    currnode = currnode->performAction(currnode->getActionByID(0));//2
    long startindex = 0;
    domain.updateConstraints(make_shared<AOH>(1, currnode->getAOHInfSet()->getAOids()),
                             startindex,
                             revealed_);
    int count = 0;
    domain.generateNodes(make_shared<AOH>(1, currnode->getAOHInfSet()->getAOids()), revealed_, 1000,
                         [&r = count](const shared_ptr<EFGNode> & node) -> double{ return r++;});
    EXPECT_EQ(count, 2*2*4);
}

//Test that GS domain returns correct number of histories for player 0
TEST(DDISMCTS, GSHistoriesGenerationTestpl0) {
    GTLib2::domains::GoofSpielSettings settings
        ({.variant =   GTLib2::domains::IncompleteObservations, .numCards = 6, .fixChanceCards = true});
    domains::GoofSpielDomain domain(settings);
    const auto root = createRootEFGNode(domain);
    auto currnode = root;
    unordered_map<unsigned long, shared_ptr<Constraint>> revealed_;
    domain.initializeEnumerativeConstraints(revealed_);
    currnode = currnode->performAction(currnode->getActionByID(3));//4 (1-4)
    currnode = currnode->performAction(currnode->getActionByID(4));//5 (5-6)
    currnode = currnode->performAction(currnode->getActionByID(2));//3 (2-5)
    currnode = currnode->performAction(currnode->getActionByID(0));//1 (1-2)
    long startindex = 0;
    domain.updateConstraints(make_shared<AOH>(0, currnode->getAOHInfSet()->getAOids()),
                             startindex,
                             revealed_);
    int count = 0;
    domain.generateNodes(make_shared<AOH>(0, currnode->getAOHInfSet()->getAOids()), revealed_, 1000,
        [&r = count](const shared_ptr<EFGNode> & node) -> double{ return r++;});
    EXPECT_EQ(count, 2*2);
}

TEST(DDISMCTS, GS19Test) {
    // todo: runs awfully slow (2mins), and fails! using gcc compiler
    auto rewards = vector<double>(20);
    for (int seed = 0; seed < 20; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  19, .fixChanceCards = true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact0 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed, .generateIters = 100};
        ISMCTSSettings settings1 = {.fact_ = fact1, .useBelief=true, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<DD_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 250;
    double summ = 0;
    cout << rewards << endl;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(DDISMCTS, GS10Test) {
    // todo: runs awfully slow (24sec), and fails! using gcc compiler
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  10, .fixChanceCards = true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact0 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed, .generateIters = 100};
        ISMCTSSettings settings1 = {.fact_ = fact1, .useBelief=true, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<DD_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 213;
    double summ = 0;
    cout << rewards << endl;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(DDISMCTS, Strat6Test) {
    // todo: runs awfully slow (33sec), and fails! using gcc compiler
    auto rewards = vector<double>(20);

    for (int seed = 0; seed < 20; ++seed) {
        GTLib2::domains::StrategoSettings settings
            ({6,6,{{2,2,2,2}}, {'B','4','3','3','2','2', '2','1','1','1','1','F'}});
        domains::StrategoDomain domain(settings);
        auto fact0 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed, .generateIters = 100};
        ISMCTSSettings settings1 = {.fact_ = fact1, .useBelief=true, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<DD_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 3;
    double summ = 0;
    cout << rewards << endl;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}
}
