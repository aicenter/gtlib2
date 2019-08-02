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


#include "domains/stratego.h"

#include "tests/domainsTest.h"
#include "gtest/gtest.h"

namespace GTLib2::domains {

TEST(Stratego, CannotMovePastBounds) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell('1', 0), LAKE};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    EXPECT_TRUE((*s).getAvailableActionsFor(0).empty());
    b = {LAKE, createCell('1', 0)};
    s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    EXPECT_TRUE((*s).getAvailableActionsFor(0).empty());
}

TEST(Stratego, CannotMoveIntoLake) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{3, 3,
                                                                     {{0, 1, 1, 1}, {1, 0, 1, 1},
                                                                      {2, 1, 1, 1}, {1, 2, 1, 1}},
                                                                     {'1'}});
    vector<CellState> b = {' ', LAKE, ' ', LAKE, createCell('1', 0), LAKE, ' ', LAKE, ' '};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    EXPECT_TRUE((*s).getAvailableActionsFor(0).empty());
}

TEST(Stratego, CannotMoveOnSamePlayerFigure) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{3, 3,
                                                                     {{0, 0, 1, 1}, {2, 2, 1, 1},
                                                                      {2, 0, 1, 1}, {0, 2, 1, 1}},
                                                                     {'1'}});
    vector<CellState> b =
        {LAKE, createCell('1', 0), LAKE, createCell('1', 0), createCell('1', 0),
         createCell('1', 0), LAKE, createCell('1', 0), LAKE};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    EXPECT_TRUE((*s).getAvailableActionsFor(0).empty());
}

TEST(Stratego, BombCannotMove) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell(BOMB, 0), createCell('1', 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    EXPECT_TRUE((*s).getAvailableActionsFor(0).empty());
}

TEST(Stratego, FlagCannotMove) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell(FLAG, 0), createCell('1', 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    EXPECT_TRUE((*s).getAvailableActionsFor(0).empty());
}

TEST(Stratego, SameRankAttack) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell('1', 0), createCell('1', 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    vector<shared_ptr<Action>> actions;
    actions = (*s).getAvailableActionsFor(0);
    EXPECT_EQ(actions.size(), 1);
    auto newstates = (*s).performActions(actions);
    EXPECT_EQ(newstates.size(), 1);
    const StrategoState state = dynamic_cast<const StrategoState &>(*newstates[0].outcome.state);
    const vector<CellState> newboard = {' ', ' '};
    EXPECT_EQ(state.boardState_, newboard);
    EXPECT_EQ(state.currentPlayer_, 1);
    EXPECT_TRUE(state.isFinished_);
}

TEST(Stratego, DiffRankAttack1) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell('1', 0), createCell('2', 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    auto actions = (*s).getAvailableActionsFor(0);
    EXPECT_EQ(actions.size(), 1);
    auto newstates = (*s).performActions(actions);
    EXPECT_EQ(newstates.size(), 1);
    const StrategoState state = dynamic_cast<const StrategoState &>(*newstates[0].outcome.state);
    const vector<CellState> newboard = {' ', createCell('2', 1)};
    EXPECT_EQ(state.boardState_, newboard);
    EXPECT_EQ(state.currentPlayer_, 1);
    EXPECT_TRUE(state.isFinished_);
}

TEST(Stratego, DiffRankAttack2) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell('2', 0), createCell('1', 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    auto actions = (*s).getAvailableActionsFor(0);
    EXPECT_EQ(actions.size(), 1);
    auto newstates = (*s).performActions(actions);
    EXPECT_EQ(newstates.size(), 1);
    const StrategoState state = dynamic_cast<const StrategoState &>(*newstates[0].outcome.state);
    const vector<CellState> newboard = {' ', createCell('2', 0)};
    EXPECT_EQ(state.boardState_, newboard);
    EXPECT_EQ(state.currentPlayer_, 1);
    EXPECT_TRUE(state.isFinished_);
}

TEST(Stratego, AttackOnBomb) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell('1', 0), createCell(BOMB, 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    auto actions = (*s).getAvailableActionsFor(0);
    EXPECT_EQ(actions.size(), 1);
    auto newstates = (*s).performActions(actions);
    EXPECT_EQ(newstates.size(), 1);
    const StrategoState state = dynamic_cast<const StrategoState &>(*newstates[0].outcome.state);
    const vector<CellState> newboard = {' ', createCell(BOMB, 1)};
    EXPECT_EQ(state.boardState_, newboard);
    EXPECT_EQ(state.currentPlayer_, 1);
    EXPECT_TRUE(state.isFinished_);
}

TEST(Stratego, AttackOnFlag) {
    const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {}, {'1'}});
    vector<CellState> b = {createCell('1', 0), createCell(FLAG, 1)};
    auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
    auto actions = (*s).getAvailableActionsFor(0);
    EXPECT_EQ(actions.size(), 1);
    auto newstates = (*s).performActions(actions);
    EXPECT_EQ(newstates.size(), 1);
    const StrategoState state = dynamic_cast<const StrategoState &>(*newstates[0].outcome.state);
    const vector<CellState> newboard = {' ', createCell('1', 0)};
    EXPECT_EQ(state.boardState_, newboard);
    EXPECT_EQ(state.currentPlayer_, 1);
    EXPECT_TRUE(state.isFinished_);
}

}
