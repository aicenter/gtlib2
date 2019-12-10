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


#ifndef GTLIB2_TREETEST_CPP
#define GTLIB2_TREETEST_CPP

#include "base/tree.h"
#include "base/fogefg.h"
#include "base/gadget.h"
#include "domains/simplePoker.h"
#include "domains/simple_games.h"
#include "domains/normal_form_game.h"
#include "utils/export.h"
#include "algorithms/oos.h"

#include "gtest/gtest.h"

namespace GTLib2 {

TEST(Gadget, GadgetForRPS) {
    // Player 0 plays with probs 0.2, 0.3, 0.5
    const auto rps = domains::RPSDomain();
    const auto rootNode = createRootEFGNode(rps);
    const auto rootInfoset = rootNode->getAOHInfSet();

    const auto rockNode = rootNode->performAction(rootNode->availableActions()[0]);
    const auto paperNode = rootNode->performAction(rootNode->availableActions()[1]);
    const auto scissorsNode = rootNode->performAction(rootNode->availableActions()[2]);

    const auto pubState = rockNode->getPublicState();
    const auto childInfoset = rockNode->getAOHInfSet();

    const auto summary = PublicStateSummary(pubState,
                                            {rockNode, paperNode, scissorsNode},
                                            {{0.2, 1.0, 1.0},
                                             {0.3, 1.0, 1.0},
                                             {0.5, 1.0, 1.0}},
                                            {1, 2, 3});
    GadgetGame pl0(summary, Player(0), childInfoset, SAFE_RESOLVING);
    GadgetGame pl1(summary, Player(1), childInfoset, SAFE_RESOLVING);
    std::stringstream ss0, ss1;

    utils::exportGambit(pl0.getRootNode(), ss0);
    utils::exportGambit(pl1.getRootNode(), ss1);

    // note: these trees are not multiplied by pubStateReach!
    // expected1 should have all leaf utilities multiplied by 3 = \pi_{-1}(S)
    const auto expected0 =
        R"(EFG 2 R "" { "Pl0" "Pl1" })" "\n"
        R"(c "" 0 "" { "0" 0.2 "1" 0.3 "2" 0.5 } 0)" "\n"
        R"( p "" 2 1 "-1" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 1 "1" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 0 "" { 0, 0})" "\n"
        R"(  t "" 1 "" { -1, 1})" "\n"
        R"(  t "" 2 "" { 1, -1})" "\n"
        R"(  t "" 3 "" { 2.3, -2.3})" "\n"
        R"( p "" 2 1 "-1" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 1 "1" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 4 "" { 1, -1})" "\n"
        R"(  t "" 5 "" { 0, 0})" "\n"
        R"(  t "" 6 "" { -1, 1})" "\n"
        R"(  t "" 7 "" { 2.3, -2.3})" "\n"
        R"( p "" 2 1 "-1" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 1 "1" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 8 "" { -1, 1})" "\n"
        R"(  t "" 9 "" { 1, -1})" "\n"
        R"(  t "" 10 "" { 0, 0})" "\n"
        R"(  t "" 11 "" { 2.3, -2.3})" "\n";

    const auto expected1 =
        R"(EFG 2 R "" { "Pl0" "Pl1" })" "\n"
        R"(c "" 0 "" { "0" 0.333333 "1" 0.333333 "2" 0.333333 } 0)" "\n"
        R"( p "" 1 1 "-1" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 2 "1" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 0 "" { 0, 0})" "\n"
        R"(  t "" 1 "" { -1, 1})" "\n"
        R"(  t "" 2 "" { 1, -1})" "\n"
        R"(  t "" 3 "" { 1, -1})" "\n"
        R"( p "" 1 3 "-1" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 2 "1" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 4 "" { 1, -1})" "\n"
        R"(  t "" 5 "" { 0, 0})" "\n"
        R"(  t "" 6 "" { -1, 1})" "\n"
        R"(  t "" 7 "" { 2, -2})" "\n"
        R"( p "" 1 4 "-1" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 2 "1" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 8 "" { -1, 1})" "\n"
        R"(  t "" 9 "" { 1, -1})" "\n"
        R"(  t "" 10 "" { 0, 0})" "\n"
        R"(  t "" 11 "" { 3, -3})" "\n";

    EXPECT_EQ(ss0.str(), expected0);
    EXPECT_EQ(ss1.str(), expected1);
}

TEST(Gadget, ComputeTerminateValues) {
    domains::SimplePokerDomain d;
    auto cache = algorithms::OOSData(d);
    cache.buildTree();
    const auto &rootPs = cache.getRootPublicState();
    const auto psPlayer1 = expandPs(cache, rootPs, 0);
    const auto psPlayer2 = expandPs(cache, psPlayer1, 1); // betting
    const auto &nodesInPs1 = cache.getNodesForPubState(psPlayer1);
    const auto &nodesInPs2 = cache.getNodesForPubState(psPlayer2);
    // make sure nodes are sorted, so we have actions JJ,JQ,QJ,QQ (set messes this up)
    auto nodes1 = vector<shared_ptr<EFGNode>>(nodesInPs1.begin(), nodesInPs1.end());
    std::sort(nodes1.begin(), nodes1.end(), nodeCompare<EFGNode>);
    auto nodes2 = vector<shared_ptr<EFGNode>>(nodesInPs2.begin(), nodesInPs2.end());
    std::sort(nodes2.begin(), nodes2.end(), nodeCompare<EFGNode>);

    auto values1 = vector<algorithms::OOSData::Fraction*>();
    auto values2 = vector<algorithms::OOSData::Fraction*>();
    for (auto &n : nodes1) { values1.emplace_back(&cache.nodeValues.at(n)); }
    for (auto &n : nodes2) { values2.emplace_back(&cache.nodeValues.at(n)); }
    auto strat1 = vector<vector<double>*>();
    auto strat2 = vector<vector<double>*>();
    for (auto &n : nodes1) { strat1.emplace_back(&cache.infosetData.at(n->getAOHInfSet()).avgStratAccumulator); }
    for (auto &n : nodes2) { strat2.emplace_back(&cache.infosetData.at(n->getAOHInfSet()).avgStratAccumulator); }


    { // no update to strategy
        const auto summary1 = cache.getPublicStateSummary(psPlayer1);
        const auto summary2 = cache.getPublicStateSummary(psPlayer2);
        auto expectedValues = vector<double>{0, 0, 0, 0};
        auto actualValues = computeTerminateCFVValues(summary1, Player(0));

        EXPECT_EQ(expectedValues.size(), actualValues.size());
        for (int i = 0; i < expectedValues.size(); ++i) {
            EXPECT_DOUBLE_EQ(expectedValues[i], actualValues[i]);
        }
    }

    { // setup utils differently
        for (int i = 0; i < 4; ++i) values1[i]->nominator = i;
        for (int i = 0; i < 4; ++i) values2[i]->nominator = 3-i;

        // --- Public state 1 ---
        const auto summary1 = cache.getPublicStateSummary(psPlayer1);
        for (int i = 0; i < 4; ++i) EXPECT_DOUBLE_EQ(summary1.expectedUtilities[i], i);
        for (int i = 0; i < 4; ++i) {
            EXPECT_DOUBLE_EQ(summary1.topmostHistoriesReachProbs[i][0], 1.);
            EXPECT_DOUBLE_EQ(summary1.topmostHistoriesReachProbs[i][1], 1.);
            EXPECT_DOUBLE_EQ(summary1.topmostHistoriesReachProbs[i][2], i == 0 || i == 3 ? 1/6. : 1/3.);
        }
        // first player
        auto expectedValues11 = vector<double>{4/3., 5/3., 4/3., 5/3.};
        auto actualValues11 = computeTerminateCFVValues(summary1, Player(0));
        EXPECT_EQ(expectedValues11.size(), actualValues11.size());
        for (int i = 0; i < expectedValues11.size(); ++i) {
            EXPECT_DOUBLE_EQ(expectedValues11[i], actualValues11[i]);
        }
        // second player
        auto expectedValues12 = vector<double>{2/3., 2/3., 7./3., 7./3.};
        auto actualValues12 = computeTerminateCFVValues(summary1, Player(1));
        EXPECT_EQ(expectedValues12.size(), actualValues12.size());
        for (int i = 0; i < expectedValues12.size(); ++i) {
            EXPECT_DOUBLE_EQ(expectedValues12[i], actualValues12[i]);
        }

        // --- Public state 2 ---
        const auto summary2 = cache.getPublicStateSummary(psPlayer2);
        for (int i = 0; i < 4; ++i) EXPECT_DOUBLE_EQ(summary2.expectedUtilities[i], 3-i);
        for (int i = 0; i < 4; ++i) {
            EXPECT_DOUBLE_EQ(summary2.topmostHistoriesReachProbs[i][0], 0.5);
            EXPECT_DOUBLE_EQ(summary2.topmostHistoriesReachProbs[i][1], 1.);
            EXPECT_DOUBLE_EQ(summary2.topmostHistoriesReachProbs[i][2], i == 0 || i == 3 ? 1/6. : 1/3.);
        }
        // first player
        auto expectedValues21 = vector<double>{5./3., 4./3., 5./3., 4/3.};
        auto actualValues21 = computeTerminateCFVValues(summary2, Player(0));
        EXPECT_EQ(expectedValues21.size(), actualValues21.size());
        for (int i = 0; i < expectedValues21.size(); ++i) {
            EXPECT_DOUBLE_EQ(expectedValues21[i], actualValues21[i]);
        }        
        // second player
        auto expectedValues22 = vector<double>{7./3., 7./3., 2/3., 2/3.};
        auto actualValues22 = computeTerminateCFVValues(summary2, Player(1));
        EXPECT_EQ(expectedValues22.size(), actualValues22.size());
        for (int i = 0; i < expectedValues22.size(); ++i) {
            EXPECT_DOUBLE_EQ(expectedValues22[i], actualValues22[i]);
        }
    }
}

}


#endif //GTLIB2_TREETEST_CPP
