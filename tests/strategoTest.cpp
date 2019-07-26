//
// Created by Nikita Milyukov on 2019-07-22.
//

#include "base/base.h"
#include "domains/stratego.h"

#include "tests/domainsTest.h"
#include "gtest/gtest.h"

namespace GTLib2::domains {

    TEST(Stratego, BoundsTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{1,1,{},{'1'}});
        vector<unsigned char> b = {'1'};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        assert ((*s).getAvailableActionsFor(0).empty());
    }

    TEST(Stratego, LakeTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{3, 3, {{0,1,1,1}, {1,0,1,1}, {2,1,1,1}, {1,2,1,1}},{'1'}});
        vector<unsigned char> b = {' ', 'L',' ', 'L', '1', 'L', ' ', 'L', ' '};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        assert ((*s).getAvailableActionsFor(0).empty());
    }

    TEST(Stratego, SamePlayerTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{3, 3, {{0,0,1,1}, {2,2,1,1}, {2,0,1,1}, {0,2,1,1}},{'1'}});
        vector<unsigned char> b = {'L', '1', 'L', '1', '1', '1', 'L', '1', 'L'};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        assert ((*s).getAvailableActionsFor(0).empty());
    }

    TEST(Stratego, BombMoveTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'B', makePlayer1('1')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        assert ((*s).getAvailableActionsFor(0).empty());
    }

    TEST(Stratego, FlagMoveTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'F', makePlayer1('1')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        assert ((*s).getAvailableActionsFor(0).empty());
    }

    TEST(Stratego, SameRankTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'1', makePlayer1('1')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        vector <shared_ptr<Action>> actions;
        actions = (*s).getAvailableActionsFor(0);
        assert (actions.size() == 1);
        auto newstates = (*s).performActions(actions);
        assert(newstates.size() == 1);
        const StrategoState state = dynamic_cast<const StrategoState&>(*newstates[0].outcome.state);
        const vector<unsigned char> newboard = {' ', ' '};
        assert(state.boardState_ == newboard);
        assert(state.currentPlayer_ == 1);
        assert(state.isFinished_);
    }

    TEST(Stratego, DiffRankTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'1', makePlayer1('2')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        auto actions = (*s).getAvailableActionsFor(0);
        assert (actions.size() == 1);
        auto newstates = (*s).performActions(actions);
        assert(newstates.size() == 1);
        const StrategoState state = dynamic_cast<const StrategoState&>(*newstates[0].outcome.state);
        const vector<unsigned char> newboard = {' ', makePlayer1('2')};
        assert(state.boardState_ == newboard);
        assert(state.currentPlayer_ == 1);
        assert(state.isFinished_);
    }

    TEST(Stratego, DiffRankTest2)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'2', makePlayer1('1')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        auto actions = (*s).getAvailableActionsFor(0);
        assert (actions.size() == 1);
        auto newstates = (*s).performActions(actions);
        assert(newstates.size() == 1);
        const StrategoState state = dynamic_cast<const StrategoState&>(*newstates[0].outcome.state);
        const vector<unsigned char> newboard = {' ', '2'};
        assert(state.boardState_ == newboard);
        assert(state.currentPlayer_ == 1);
        assert(state.isFinished_);
    }

    TEST(Stratego, BombTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'1', makePlayer1('B')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        auto actions = (*s).getAvailableActionsFor(0);
        assert (actions.size() == 1);
        auto newstates = (*s).performActions(actions);
        assert(newstates.size() == 1);
        const StrategoState state = dynamic_cast<const StrategoState&>(*newstates[0].outcome.state);
        const vector<unsigned char> newboard = {' ', makePlayer1('B')};
        assert(state.boardState_ == newboard);
        assert(state.currentPlayer_ == 1);
        assert(state.isFinished_);
    }

    TEST(Stratego, FlagTest)
    {
        const auto domain = make_shared<StrategoDomain>(StrategoSettings{2, 1, {},{'1'}});
        vector<unsigned char> b = {'1', makePlayer1('F')};
        auto s = make_shared<StrategoState>(&(*domain), b, false, false, 0, 0);
        auto actions = (*s).getAvailableActionsFor(0);
        assert (actions.size() == 1);
        auto newstates = (*s).performActions(actions);
        assert(newstates.size() == 1);
        const StrategoState state = dynamic_cast<const StrategoState&>(*newstates[0].outcome.state);
        const vector<unsigned char> newboard = {' ', '1'};
        assert(state.boardState_ == newboard);
        assert(state.currentPlayer_ == 1);
        assert(state.isFinished_);
    }

}