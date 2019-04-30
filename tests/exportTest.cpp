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

#include "domains/genericPoker.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"

#include "utils/functools.h"
#include "utils/export.h"

#include "gtest/gtest.h"

namespace GTLib2::utils {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesVariant::AlternatingMoves;


TEST(Export, GambitSmallDomain) {
    auto mp = MatchingPenniesDomain(AlternatingMoves);
    std::ostringstream gambit_os;
    exportGambit(mp, gambit_os);

    string expectedGambit =
        R"(EFG 2 R "N6GTLib27domains21MatchingPenniesDomainE" { "Pl0" "Pl1" })" "\n"
        R"("")" "\n"
        R"(p "" 1 1 "" { "Heads" "Tails" } 0)" "\n"
        R"( p "" 2 2 "" { "Heads" "Tails" } 0)" "\n"
        R"(  t "" 0 "" { 1, -1})" "\n"
        R"(  t "" 1 "" { -1, 1})" "\n"
        R"( p "" 2 2 "" { "Heads" "Tails" } 0)" "\n"
        R"(  t "" 2 "" { -1, 1})" "\n"
        R"(  t "" 3 "" { 1, -1})" "\n";
    EXPECT_TRUE(expectedGambit == gambit_os.str());
}

TEST(Export, GraphvizSmallDomain) {
    auto mp = MatchingPenniesDomain(AlternatingMoves);
    std::ostringstream graphviz_os;
    exportGraphViz(mp, graphviz_os);

    string expectedGraphviz =
        R"(digraph {)" "\n"
        R"(	rankdir=LR)" "\n"
        R"(	graph [fontname=courier])" "\n"
        R"(	node  [fontname=courier, shape=box, style="filled", fillcolor=white])" "\n"
        R"(	edge  [fontname=courier])" "\n"
        R"(	"[4294967295, 4294967295, 0]" -> "[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1]" [label="Heads, p=1"])" "\n"
        R"(	"[4294967295, 4294967295, 0]" [fillcolor="#FF0000", label="", shape=triangle])" "\n"
        R"(	"[4294967295, 4294967295, 0]" -> "[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1]" [label="Tails, p=1"])" "\n"
        R"(	"[4294967295, 4294967295, 0]" [fillcolor="#FF0000", label="", shape=triangle])" "\n"
        R"(	"[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1]" -> "[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1, 0, 0, 0]" [label="Heads, p=1"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1]" [fillcolor="#00FF00", label="", shape=invtriangle])" "\n"
        R"(	"[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1]" -> "[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1, 1, 1, 0]" [label="Tails, p=1"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1]" [fillcolor="#00FF00", label="", shape=invtriangle])" "\n"
        R"(	"[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1, 0, 0, 0]" [fillcolor="#FFFFFF", label="[1, -1]"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 0, 4294967295, 4294967295, 1, 1, 1, 0]" [fillcolor="#FFFFFF", label="[-1, 1]"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1]" -> "[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1, 0, 0, 1]" [label="Heads, p=1"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1]" [fillcolor="#00FF00", label="", shape=invtriangle])" "\n"
        R"(	"[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1]" -> "[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1, 1, 1, 1]" [label="Tails, p=1"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1]" [fillcolor="#00FF00", label="", shape=invtriangle])" "\n"
        R"(	"[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1, 0, 0, 1]" [fillcolor="#FFFFFF", label="[-1, 1]"])" "\n"
        R"(	"[4294967295, 4294967295, 0, 1, 4294967295, 4294967295, 1, 1, 1, 1]" [fillcolor="#FFFFFF", label="[1, -1]"])" "\n"
        R"(})" "\n";

    cout << graphviz_os.str();

    EXPECT_TRUE(expectedGraphviz == graphviz_os.str());
}

}
