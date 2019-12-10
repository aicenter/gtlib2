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


#include "tests/mccrTest.h"

#include "base/base.h"
#include "base/fogefg.h"

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/strategy.h"
#include "algorithms/continualResolving.h"
#include "algorithms/oos.h"
#include "algorithms/mccr.h"

#include "algorithms/utility.h"
#include "algorithms/evaluation.h"
#include "domains/goofSpiel.h"
#include "domains/gambit.h"
#include "domains/simple_games.h"

#include "gtest/gtest.h"

#include <iostream>
#include <fstream>


namespace GTLib2::algorithms {

using domains::GoofSpielDomain;
using domains::RPSDomain;


pair<ActionId, RandomLeafOutcome>
FixedSamplingMCCRResolver::selectLeaf(const shared_ptr<EFGNode> &start,
                             const vector<shared_ptr<Action>> &actions) {
    RandomLeafOutcome out = {
        .utilities = vector<double>(),
        .playerReachProbs = vector<double>{1., 1.},
        .chanceReachProb = 1.
    };

    std::shared_ptr<EFGNode> h = start;
    int firstAction = -1;
    while (h->type_ != TerminalNode) {
        const auto actions = h->availableActions();
        const int ai = nextAction();
        if (firstAction == -1) firstAction = ai;

        switch (h->type_) {
            case ChanceNode:
                out.chanceReachProb *= h->chanceProbForAction(actions[ai]);
                break;
            case PlayerNode:
                out.playerReachProbs[h->getPlayer()] *= 1.0 / actions.size();
                break;
            case TerminalNode:
                unreachable("terminal node!");
            default:
                unreachable("unrecognized option!");
        }

        h = h->performAction(actions[ai]);
    }

    out.utilities = h->getUtilities();

    cache_.getChildFor(start, actions[firstAction]); // build the node in cache
    return make_pair(firstAction, out);
}


void prepareGambitFile(string loc) {
    string gambit_description = "EFG 2 R \"\" { \"Pl0\" \"Pl1\" }\n"
                                "p \"\" 1 1000 \"100\" { \"Pl1 begins\" \"Pl2 begins\" } 0\n"
                                " c \"\" 1 \"\" { \"CardsJJ\" 0.167 \"CardsJQ\" 0.333 \"CardsQJ\" 0.333 \"CardsQQ\" 0.167 } 0\n"
                                "  p \"\" 1 1110 \"111\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 1 \"\" { -1, 1 }\n"
                                "   p \"\" 2 1120 \"112\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 2 \"\" { 1, -1 }\n"
                                "    t \"\" 3 \"\" { 0, 0 }\n"
                                "  p \"\" 1 1110 \"111\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 4 \"\" { -1, 1 }\n"
                                "   p \"\" 2 1121 \"112\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 5 \"\" { 1, -1 }\n"
                                "    t \"\" 6 \"\" { -3, 3 }\n"
                                "  p \"\" 1 1111 \"111\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 7 \"\" { -1, 1 }\n"
                                "   p \"\" 2 1120 \"112\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 8 \"\" { 1, -1 }\n"
                                "    t \"\" 9 \"\" { 3, -3 }\n"
                                "  p \"\" 1 1111 \"111\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 10 \"\" { -1, 1 }\n"
                                "   p \"\" 2 1121 \"112\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 11 \"\" { 1, -1 }\n"
                                "    t \"\" 12 \"\" { 0, 0 }\n"
                                " c \"\" 2 \"\" { \"CardsJJ\" 0.167 \"CardsJQ\" 0.333 \"CardsQJ\" 0.333 \"CardsQQ\" 0.167 } 0\n"
                                "  p \"\" 2 1220 \"122\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 1 \"\" { -1, 1 }\n"
                                "   p \"\" 1 1210 \"121\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 2 \"\" { 1, -1 }\n"
                                "    t \"\" 3 \"\" { 0, 0 }\n"
                                "  p \"\" 2 1220 \"122\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 4 \"\" { -1, 1 }\n"
                                "   p \"\" 1 1211 \"121\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 5 \"\" { 1, -1 }\n"
                                "    t \"\" 6 \"\" { -3, 3 }\n"
                                "  p \"\" 2 1221 \"122\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 7 \"\" { -1, 1 }\n"
                                "   p \"\" 1 1210 \"121\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 8 \"\" { 1, -1 }\n"
                                "    t \"\" 9 \"\" { 3, -3 }\n"
                                "  p \"\" 2 1221 \"122\" { \"Fold\" \"Bet\" } 0\n"
                                "   t \"\" 10 \"\" { -1, 1 }\n"
                                "   p \"\" 1 1211 \"121\" { \"Fold\" \"Call\" } 0\n"
                                "    t \"\" 11 \"\" { 1, -1 }\n"
                                "    t \"\" 12 \"\" { 0, 0 }\n";

    std::ofstream myfile;
    myfile.open(loc);
    myfile << gambit_description;
    myfile.close();
}

// quick hack to setup samples. A method would have to encapsulate MCCRData since
// it's passed by reference and the allocation from stack would be removed.
#define GTLIB_TEST_MCCR_SETUP()                                                     \
    prepareGambitFile("/tmp/double_poker.gbt");                                     \
    auto d = domains::GambitDomain("/tmp/double_poker.gbt");                        \
    auto cache = MCCRData(d);                                                       \
    MCCRSettings settings;                                                          \
    settings.baseline = OOSSettings::WeightedActingPlayerBaseline;                  \
    auto playingPlayer = Player(0);                                                 \
    unique_ptr <MCCRResolver> resolver = make_unique<FixedSamplingMCCRResolver>(    \
        d, playingPlayer, cache, settings, samples);                                \
    auto mccr = MCCRAlgorithm(d, playingPlayer, cache, move(resolver), settings);


TEST(MCCR, NoSamplesValuesShouldBeZero) {
    auto samples = vector<vector<ActionId>>{};
    GTLIB_TEST_MCCR_SETUP()

    // Test
    auto rootPs = mccr.getCache().getRootPublicState();
    auto summary = mccr.getCache().getPublicStateSummary(rootPs);
    EXPECT_EQ(summary.expectedUtilities[0], 0);
}

TEST(MCCR, IncrementallyBuildTree) {
    auto samples = vector<vector<ActionId>>();
    GTLIB_TEST_MCCR_SETUP()
    auto* testResolver = dynamic_cast<FixedSamplingMCCRResolver*>(mccr.getResolver());

    auto testsSamples = vector<vector<vector<ActionId>>>{
        {{0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}}, // make sure resolver is reset properly
    };
    auto expectedValues = vector<double>{
        -2/3.,
        -2/3.,
    };

    for (int test_case = 0; test_case < testsSamples.size(); ++test_case) {
        testResolver->clear();
        samples = testsSamples[test_case];

        // Run
        for (int i = 0; i < samples.size() / 2; ++i) mccr.runPlayIteration(nullopt);

        // Test
        const auto rootPs = mccr.getCache().getRootPublicState();
        const auto summary = mccr.getCache().getPublicStateSummary(rootPs);
        const auto actualValue = summary.expectedUtilities[0];
        EXPECT_DOUBLE_EQ(actualValue, expectedValues[test_case]);
    }
}

TEST(MCCR, PreBuildTree) {
    auto samples = vector<vector<ActionId>>();
    GTLIB_TEST_MCCR_SETUP()
    cache.buildTree();
    auto* testResolver = dynamic_cast<FixedSamplingMCCRResolver*>(mccr.getResolver());

    auto testsSamples = vector<vector<vector<ActionId>>>{
        // make sure the whichever fold we sample doesnt matter
        {{0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}}, // make sure resolver is reset properly
        {{0, 0, 0}, {1, 0, 0}},
        {{1, 0, 0}, {0, 0, 0}},
        {{1, 3, 0}, {0, 2, 0}},

        // Make multiple visits of fold (utility -1). It converges to -0.5 if visited infinitely,
        // because after first iteration player 0 does not want to go there (he has negative regret).
        // However, we force him, and he receives almost 0 value (not exactly 0 due to approx RM.)
        // The opponent goes there no problem. So we have a fraction (-x-1) / (2x+1), with x->inf
        // it becomes -0.5
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    };
    auto expectedValues = vector<double>{
        -2/3.,
        -2/3.,
        -2/3.,
        -2/3.,
        -2/3.,
        -0.50769363071401552,
    };

    for (int test_case = 0; test_case < testsSamples.size(); ++test_case) {
        testResolver->reset();
        samples = testsSamples[test_case];

        // Run
        for (int i = 0; i < samples.size() / 2; ++i) mccr.runPlayIteration(nullopt);

        // Test
        const auto rootPs = mccr.getCache().getRootPublicState();
        const auto summary = mccr.getCache().getPublicStateSummary(rootPs);
        const auto actualValue = summary.expectedUtilities[0];
        EXPECT_DOUBLE_EQ(actualValue, expectedValues[test_case]);
    }
}


}
