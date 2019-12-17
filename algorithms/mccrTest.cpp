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


#include "mccrTest.h"

#include "base/base.h"
#include "base/fogefg.h"

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/strategy.h"
#include "algorithms/continualResolving.h"
#include "algorithms/oos.h"
#include "algorithms/onlineAvgStrategy.h"
#include "algorithms/mccr.h"
#include "algorithms/evaluation.h"

#include "algorithms/utility.h"
#include "algorithms/evaluation.h"
#include "domains/goofSpiel.h"
#include "domains/gambit/gambit_games.h"
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
        /*.utilities=*/vector<double>(),
        /*.playerReachProbs=*/vector<double>{1., 1.},
        /*.chanceReachProb=*/1.
    };

    std::shared_ptr<EFGNode> h = start;
    int firstAction = -1;
    while (h->type_ != TerminalNode) {
        const auto nextActions = h->availableActions();
        const int ai = nextAction();
        if (firstAction == -1) firstAction = ai;

        switch (h->type_) {
            case ChanceNode:
                out.chanceReachProb *= h->chanceProbForAction(nextActions[ai]);
                break;
            case PlayerNode:
                out.playerReachProbs[h->getPlayer()] *= 1.0 / nextActions.size();
                break;
            case TerminalNode:
                unreachable("terminal node!");
            default:
                unreachable("unrecognized option!");
        }

        h = h->performAction(nextActions[ai]);
    }

    out.utilities = h->getUtilities();

    cache_.getChildFor(start, actions[firstAction]); // build the node in cache
    return make_pair(firstAction, out);
}


// quick hack to setup samples. A method would have to encapsulate MCCRData since
// it's passed by reference and the allocation from stack would be removed.
#define GTLIB_TEST_MCCR_SETUP()                                                     \
    auto d = domains::gambit::doublePokerDomain();                                  \
    auto cache = MCCRData(*d);                                                      \
    MCCRSettings settings;                                                          \
    settings.baseline = OOSSettings::WeightedActingPlayer;                  \
    auto playingPlayer = Player(0);                                                 \
    unique_ptr <MCCRResolver> resolver = make_unique<FixedSamplingMCCRResolver>(    \
        *d, playingPlayer, cache, settings, samples);                               \
    auto mccr = MCCRAlgorithm(*d, playingPlayer, cache, move(resolver), settings);


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
    auto *testResolver = dynamic_cast<FixedSamplingMCCRResolver *>(mccr.getResolver());

    auto testsSamples = vector<vector<vector<ActionId>>>{
        {{0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}}, // make sure resolver is reset properly
    };
    auto expectedValues = vector<double>{
        -2 / 3.,
        -2 / 3.,
    };

    for (unsigned int test_case = 0; test_case < testsSamples.size(); ++test_case) {
        testResolver->clear();
        samples = testsSamples[test_case];

        // Run
        for (unsigned int i = 0; i < samples.size() / 2; ++i) mccr.runPlayIteration(nullopt);

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
    auto *testResolver = dynamic_cast<FixedSamplingMCCRResolver *>(mccr.getResolver());

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
        // todo: revise test
//        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//         {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    };
    auto expectedValues = vector<double>{
        -2 / 3.,
        -2 / 3.,
        -2 / 3.,
        -2 / 3.,
        -2 / 3.,
//        -0.50769363071401552,
    };

    for (unsigned int test_case = 0; test_case < testsSamples.size(); ++test_case) {
        testResolver->reset();
        samples = testsSamples[test_case];

        // Run
        for (unsigned int i = 0; i < samples.size() / 2; ++i) mccr.runPlayIteration(nullopt);

        // Test
        const auto rootPs = mccr.getCache().getRootPublicState();
        const auto summary = mccr.getCache().getPublicStateSummary(rootPs);
        const auto actualValue = summary.expectedUtilities[0];
        EXPECT_DOUBLE_EQ(actualValue, expectedValues[test_case]);
    }
}

TEST(MCCR, ResolveMatchingPenniesWithCorrectCFVs) {
    const auto d = domains::MatchingPenniesDomain();
    const auto rootNode = createRootEFGNode(d);
    const auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    const auto targetAoh = childNode->getAOHInfSet();

    // Not derived from any calculations, just by seeing the results.
    // The bound is too loose and only probabilistic anyway.
    const auto expectedUBonDiffs = vector<double>{0.5, 0.2, 0.1, 0.02, 0.005};

    cout << "\n";
    for (int seed = 0; seed < 5; seed++) {
        for (int i = 0; i < 5; i++) {
            int iters = pow(10, (i + 1));

            auto cache = MCCRData(d);
            cache.buildTree();

            MCCRSettings settings;
            settings.seed = seed;
            settings.baseline = OOSSettings::NoAvgValueCalculation;
            settings.approxRegretMatching = 0.1;
            settings.targetBiasing = 0.;

            const auto playingPlayer = Player(1);
            auto resolver = make_unique<MCCRResolver>(d, playingPlayer, cache, settings);
            auto mccr = MCCRAlgorithm(d, playingPlayer, cache, move(resolver), settings);

            // CFVs are implicitly zero (i.e. correct values for MP),
            // because we dot run any root iterations beforehand.
            playForIterations(mccr, targetAoh, iters);

            for (const auto&[aoh, data] : cache.infosetData) {
                if (*targetAoh == *aoh) {
                    auto probs = calcAvgProbs(data.avgStratAccumulator);
                    cout << iters << "," << abs(probs[0] - 0.5) << "\n";
                    cout << "----\n";
                    cout << "seed " << seed << "\n";
                    cout << "iters " << iters << "\n";
                    cout << "reg: " << data.regrets << "\n";
                    cout << "avg: " << data.avgStratAccumulator << "\n";
                    cout << "probs: " << probs << "\n";
                    EXPECT_LT(abs(probs[0] - 0.5), expectedUBonDiffs[i]);
                }
            }
        }
    }
}


}
