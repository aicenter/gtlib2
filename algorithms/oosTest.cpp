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
#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/oos.h"

#include "algorithms/utility.h"
#include "algorithms/evaluation.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"
#include "utils/range.h"
#include "utils/benchmark.h"

#include "oosTest.h"
#include <iostream>
#include <fstream>

#include "gtest/gtest.h"


namespace GTLib2::algorithms {

pair<ActionId, RandomLeafOutcome>
FixedSamplingOOS::selectLeaf(const shared_ptr<EFGNode> &start,
                             const vector<shared_ptr<Action>> &) {
    RandomLeafOutcome out = {
        /*.utilities=*/vector<double>(),
        /*.playerReachProbs=*/vector<double>{1., 1.},
        /*.chanceReachProb=*/1.
    };

    std::shared_ptr<EFGNode> h = start;
    int firstAction = -1;
    while (h->type_ != TerminalNode) {
        const auto &actions = h->availableActions();
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
    return pair(firstAction, out);
}


using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielVariant::CompleteObservations;


TEST(OOS, CheckExploitabilityInSmallDomain) {
    const auto domain = GoofSpielDomain::IIGS(4);
    const auto settings = OOSSettings();

    auto data = OOSData(*domain);
    data.buildTree();
    OOSAlgorithm oos(*domain, Player(0), data, settings);


    const auto expectedExploitabilities = vector<double>{
        0.3125, 0.28461549783710799, 0.21121123287370133, 0.058389394221310148, 0.012451791378926925
    };
    int j = 0;
    cout << endl;
    double prevIters = 1.0;
    for (const auto iters: utils::log10Range(1, 1e4, 5)) {
        const auto timems = utils::benchmarkRuntime([&]() {
            for (int i = 0; i < (iters - prevIters); ++i) oos.runPlayIteration(nullopt);
        });
        const auto actualExpl = calcExploitability(*domain, getAverageStrategy(data)).expl;

        cout << floor(iters) << " "
             << floor(iters - prevIters) << " "
             << "[" << timems << "ms]: "
             << actualExpl << endl;
        prevIters = iters;

        EXPECT_LE(fabs(floor(pow(10, j)) - floor(iters)), 1e-5);
        EXPECT_LE(fabs(expectedExploitabilities[j++] - actualExpl), 1e-5);
    }
}

TEST(OOS, PlayMatchInSmallDomain) {
    auto generator = std::mt19937(0);
    EXPECT_DOUBLE_EQ(uniformDist(generator), 0.59284461651668263);
    EXPECT_DOUBLE_EQ(uniformDist(generator), 0.84426574425659828);

    const auto domain = GoofSpielDomain::IIGS(2);
    const auto settings = OOSSettings();
    const auto expectedRewards =
        vector<double>{0, 0, 0, 0}; // todo: make better test where we don't get just zeros :/
    const auto samples0 = vector<vector<vector<ActionId>>>{
        {{0, 0, 0, 0}, {0, 0, 0, 0}}, // only root
        {{0, 0, 0, 0}, {0, 0, 0, 0}, // root
         {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}}, // move
        {{0, 0, 0, 0}, {0, 0, 0, 0}, // root
         {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0},
         {1, 0, 0, 0}, {1, 0, 0, 0}}, // move
    };
    const auto samples1 = vector<vector<vector<ActionId>>>{
        {{0, 0, 0, 0}, {0, 0, 0, 0}}, // only root
        {{0, 0, 0, 0}, {0, 0, 0, 0},  // root
         {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0},}, // move
        {{0, 0, 0, 0}, {0, 0, 0, 0}, // root
         {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0},
         {0, 1, 0, 0}, {0, 1, 0, 0}}, // move
    };

    for (unsigned int i = 0; i < 3; ++i) {
        auto data0 = OOSData(*domain);
        data0.buildTree();
        auto data1 = OOSData(*domain);
        data1.buildTree();
        vector<PreparedAlgorithm> algs = {
            createInitializer<FixedSamplingOOS>(data0, settings, samples0[i]),
            createInitializer<FixedSamplingOOS>(data1, settings, samples1[i])
        };
        auto r = expectedRewards[i];
        EXPECT_EQ(playMatch(*domain, algs, {1, 1}, {i, i}, BudgetIterations, 0),
                  (vector<double>{r, -r}));
    }
}

TEST(OOS, PlayManyMatches) {
    const auto domain = GoofSpielDomain::IIGS(3);
    const auto settings = OOSSettings();
    const auto expectedRewards = vector<double>{
        0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 2, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 4, 0, 1, 2, 2, 0, 0, 0,
        0, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 4, 0, 1,
        0, 0, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 2, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 0, 0,
        0, 0, 1, 0,
    };

    for (int seed = 0; seed < 100; ++seed) {
        auto data0 = OOSData(*domain);
        auto data1 = OOSData(*domain);
        vector<PreparedAlgorithm> algs = {
            createInitializer<OOSAlgorithm>(data0, settings),
            createInitializer<OOSAlgorithm>(data1, settings)
        };
        auto r = expectedRewards[seed];
        auto actualOutcome = playMatch(*domain, algs, {10, 10}, {5, 5}, BudgetIterations, seed);
        auto expectedOutcome = vector<double>{r, -r};
        EXPECT_EQ(expectedOutcome, actualOutcome);
    }
}

// todo: add test which checks regrets / strategy after some samples
// todo: add test which checks biasing in the game
// todo: add test which checks statistics of targetting

// todo: add test that OOS makes the same samples as FixedOOS
//vector<vector<ActionId>> loadSamplesIIGS_3(string f) {
//    vector<vector<ActionId>> vec = {};
//    std::ifstream inFile;
//    ActionId v1, v2, v3, v4, v5, v6;
//
//    inFile.open(f);
//    while (!inFile.eof()) {
//        inFile >> v1 >> v2 >> v3 >> v4 >> v5 >> v6;
//        vec.push_back(vector<ActionId>{v1, v2, v3, v4, v5, v6});
//    }
//    inFile.close();
//    return vec;
//}
//
//TEST(OOS, CheckSpecificSamples) {
//    auto domain = GoofSpielDomain::IIGS(3);
//
//    auto settings = OOSSettings();
//    auto dataFixed = OOSData(*domain);
//    auto dataNotFixed = OOSData(*domain);
//
//    auto samples = loadSamplesIIGS_3("resources/oos_test_samples.txt");
//    FixedSamplingOOS fixed(*domain, Player(0), dataFixed, settings, samples);
//    OOSAlgorithm notFixed(*domain, Player(0), dataNotFixed, settings);
//
//    for (int i = 0; i < samples.size(); ++i) notFixed.runPlayIteration(nullopt);
//
//    EXPECT_EQ(dataFixed, dataNotFixed);
//}

}
