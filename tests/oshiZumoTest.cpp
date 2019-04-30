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
#include "algorithms/stats.h"
#include "algorithms/tree.h"
#include "algorithms/utility.h"
#include "domains/oshiZumo.h"
#include "utils/functools.h"

#include "gtest/gtest.h"

namespace GTLib2::domains {

using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;


OshiZumoDomain testDomainsOshiZumo[]{ // NOLINT(cert-err58-cpp)
    OshiZumoDomain({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 0, .minBid = 1, .optimalEndGame = false}),
    OshiZumoDomain({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 3, .minBid = 1, .optimalEndGame = false}),
    OshiZumoDomain({.variant =  CompleteObservation, .startingCoins = 1, .startingLocation = 3, .minBid = 0, .optimalEndGame = false}),
    OshiZumoDomain({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 0, .minBid = 1, .optimalEndGame = true}),
    OshiZumoDomain({.variant =  CompleteObservation, .startingCoins = 1, .startingLocation = 3, .minBid = 0, .optimalEndGame = true}),
    OshiZumoDomain({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 3, .minBid = 1, .optimalEndGame = true}),
    OshiZumoDomain({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 0, .minBid = 1, .optimalEndGame = false}),
    OshiZumoDomain({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 3, .minBid = 1, .optimalEndGame = false}),
    OshiZumoDomain({.variant =  IncompleteObservation, .startingCoins = 1, .startingLocation = 3, .minBid = 0, .optimalEndGame = false}),
    OshiZumoDomain({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 0, .minBid = 1, .optimalEndGame = true}),
    OshiZumoDomain({.variant =  IncompleteObservation, .startingCoins = 1, .startingLocation = 3, .minBid = 0, .optimalEndGame = true}),
    OshiZumoDomain({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 3, .minBid = 1, .optimalEndGame = true}),

};

TEST(Oshizumo, BuildAndCheckGameTree) {
    DomainStatistics expectedResults[]{
        {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 23,
            .num_terminals = 12,
            .num_states = 16,
            .num_histories = {4, 7},
            .num_infosets = {4, 4},
            .num_sequences = {8, 8},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 53,
            .num_terminals = 16,
            .num_states = 32,
            .num_histories = {16, 21},
            .num_infosets = {16, 16},
            .num_sequences = {22, 22},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 2,
            .num_nodes = 20,
            .num_terminals = 4,
            .num_states = 13,
            .num_histories = {9, 7},
            .num_infosets = {9, 4},
            .num_sequences = {8, 8},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 23,
            .num_terminals = 12,
            .num_states = 16,
            .num_histories = {4, 7},
            .num_infosets = {4, 4},
            .num_sequences = {8, 8},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 2,
            .num_nodes = 13,
            .num_terminals = 6,
            .num_states = 9,
            .num_histories = {3, 4},
            .num_infosets = {3, 2},
            .num_sequences = {5, 5},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 30,
            .num_terminals = 14,
            .num_states = 20,
            .num_histories = {6, 10},
            .num_infosets = {6, 6},
            .num_sequences = {11, 11},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 23,
            .num_terminals = 12,
            .num_states = 16,
            .num_histories = {4, 7},
            .num_infosets = {4, 4},
            .num_sequences = {8, 8},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 53,
            .num_terminals = 16,
            .num_states = 32,
            .num_histories = {16, 21},
            .num_infosets = {14, 14},
            .num_sequences = {19, 19},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 2,
            .num_nodes = 20,
            .num_terminals = 4,
            .num_states = 13,
            .num_histories = {9, 7},
            .num_infosets = {9, 4},
            .num_sequences = {8, 8},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 23,
            .num_terminals = 12,
            .num_states = 16,
            .num_histories = {4, 7},
            .num_infosets = {4, 4},
            .num_sequences = {8, 8},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 2,
            .num_nodes = 13,
            .num_terminals = 6,
            .num_states = 9,
            .num_histories = {3, 4},
            .num_infosets = {3, 2},
            .num_sequences = {5, 5},
        }, {
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 30,
            .num_terminals = 14,
            .num_states = 20,
            .num_histories = {6, 10},
            .num_infosets = {6, 6},
            .num_sequences = {11, 11},
        }
    };
    for (auto tuple : zip(testDomainsOshiZumo, expectedResults)) {
        unzip(tuple, testDomain, expectedRes);
        DomainStatistics stats;
        calculateDomainStatistics(testDomain, &stats);
        EXPECT_EQ(stats, expectedRes);
    }
}

TEST(Oshizumo, CorrectRewardsDistribution) {
    OshiZumoDomain ozd
        ({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 0, .minBid = 1, .optimalEndGame = true});
    vector<int> rew{0, 0, 0};

    auto getRewards = [&rew](shared_ptr<EFGNode> node) {
        if (node->isTerminal()) {
            if (node->rewards_[0] == 0) {
                rew[0]++;
            } else if (node->rewards_[0] == 1) {
                rew[1]++;
            } else {
                rew[2]++;
            }
        }
    };
    treeWalkEFG(ozd, getRewards);

    EXPECT_EQ(rew[0], 4);
    EXPECT_EQ(rew[1], 4);
    EXPECT_EQ(rew[2], 4);
}

}
