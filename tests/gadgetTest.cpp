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
#include "domains/simple_games.h"
#include "domains/normal_form_game.h"
#include "utils/export.h"
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

    const auto childInfoset = rockNode->getAOHInfSet();

    const auto summary = PublicStateSummary({rockNode, paperNode, scissorsNode},
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
        R"("")" "\n"
        R"(c "" 0 "" { "0" 0.2 "1" 0.3 "2" 0.5 } 0)" "\n"
        R"( p "" 2 1 "" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 1 "" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 0 "" { 0, 0})" "\n"
        R"(  t "" 1 "" { -1, 1})" "\n"
        R"(  t "" 2 "" { 1, -1})" "\n"
        R"(  t "" 3 "" { 2.3, -2.3})" "\n"
        R"( p "" 2 1 "" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 1 "" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 4 "" { 1, -1})" "\n"
        R"(  t "" 5 "" { 0, 0})" "\n"
        R"(  t "" 6 "" { -1, 1})" "\n"
        R"(  t "" 7 "" { 2.3, -2.3})" "\n"
        R"( p "" 2 1 "" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 1 "" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 8 "" { -1, 1})" "\n"
        R"(  t "" 9 "" { 1, -1})" "\n"
        R"(  t "" 10 "" { 0, 0})" "\n"
        R"(  t "" 11 "" { 2.3, -2.3})" "\n";

    const auto expected1 =
        R"(EFG 2 R "" { "Pl0" "Pl1" })" "\n"
        R"("")" "\n"
        R"(c "" 0 "" { "0" 0.333333 "1" 0.333333 "2" 0.333333 } 0)" "\n"
        R"( p "" 1 1 "" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 2 "" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 0 "" { 0, 0})" "\n"
        R"(  t "" 1 "" { -1, 1})" "\n"
        R"(  t "" 2 "" { 1, -1})" "\n"
        R"(  t "" 3 "" { 1, -1})" "\n"
        R"( p "" 1 3 "" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 2 "" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 4 "" { 1, -1})" "\n"
        R"(  t "" 5 "" { 0, 0})" "\n"
        R"(  t "" 6 "" { -1, 1})" "\n"
        R"(  t "" 7 "" { 2, -2})" "\n"
        R"( p "" 1 4 "" { "Follow" "Terminate" } 0)" "\n"
        R"( p "" 2 2 "" { "Rock" "Paper" "Scissors" } 0)" "\n"
        R"(  t "" 8 "" { -1, 1})" "\n"
        R"(  t "" 9 "" { 1, -1})" "\n"
        R"(  t "" 10 "" { 0, 0})" "\n"
        R"(  t "" 11 "" { 3, -3})" "\n";

    EXPECT_EQ(ss0.str(), expected0);
    EXPECT_EQ(ss1.str(), expected1);
}

}


#endif //GTLIB2_TREETEST_CPP
