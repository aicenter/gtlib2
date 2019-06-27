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
#include "domains/randomGame.h"

#include "gtest/gtest.h"

namespace GTLib2::domains {

using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;

bool gameConsistency(const RandomGameDomain &domain1, const RandomGameDomain &domain2) {
    unsigned int violations = 0;
    auto traverse = [&](const shared_ptr<EFGNode> &node1,
                        const shared_ptr<EFGNode> &node2,
                        const auto &traverse) {
        if (!(*node1 == *node2)) {
            violations++;
            return;
        }
        if (node1->type_ == TerminalNode) return;
        const auto actions1 = node1->availableActions();
        const auto actions2 = node2->availableActions();
        if (actions1.size() != actions2.size()) {
            violations++;
            return;
        }
        for (int i = 0; i < actions1.size(); ++i) {
            traverse(node1->performAction(actions1[i]),
                     node2->performAction(actions2[i]),
                     traverse);
        }
    };
    traverse(createRootEFGNode(domain1), createRootEFGNode(domain2), traverse);
    return 0 == violations;
}

//@formatter:off
RandomGameSettings p_game_settings[] = {
    {.seed = 13, .maxDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 4, .maxRewardModification = 2,  .maxUtility = 100, .binaryUtility = true,  .utilityCorrelation = true,  .fixedBranchingFactor = true},
    {.seed = 7,  .maxDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 2,  .maxUtility = 100, .binaryUtility = true,  .utilityCorrelation = true,  .fixedBranchingFactor = true},
    {.seed = 5,  .maxDepth = 2, .maxBranchingFactor = 6, .maxDifferentObservations = 2, .maxRewardModification = 2,  .maxUtility = 100, .binaryUtility = true,  .utilityCorrelation = true,  .fixedBranchingFactor = false},
    {.seed = 9,  .maxDepth = 3, .maxBranchingFactor = 6, .maxDifferentObservations = 3, .maxRewardModification = 20, .maxUtility = 100, .binaryUtility = false, .utilityCorrelation = true,  .fixedBranchingFactor = false},
    {.seed = 17, .maxDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 20, .maxUtility = 100, .binaryUtility = false, .utilityCorrelation = true,  .fixedBranchingFactor = false},
    {.seed = 1,  .maxDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 2,  .maxUtility = 100, .binaryUtility = true,  .utilityCorrelation = false, .fixedBranchingFactor = false},
    {.seed = 3,  .maxDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 20, .maxUtility = 100, .binaryUtility = false, .utilityCorrelation = true,  .fixedBranchingFactor = false},
    {.seed = 13, .maxDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 2,  .maxUtility = 100, .binaryUtility = false, .utilityCorrelation = false, .fixedBranchingFactor = false},

};
//@formatter:on

TEST(RandomGame, checkDomainStats) {

    vector<DomainStatistics> expectedStats = {
        {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 341,
            .num_terminals = 256,
            .num_states = 273,
            .num_histories = {17, 68},
            .num_infosets = {17, 17},
            .num_sequences = {69, 69},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 341,
            .num_terminals = 256,
            .num_states = 273,
            .num_histories = {17, 68},
            .num_infosets = {9, 9},
            .num_sequences = {37, 37},
        }, {
            .max_EFGDepth = 2,
            .max_StateDepth = 2,
            .num_nodes = 13,
            .num_terminals = 9,
            .num_states = 10,
            .num_histories = {1, 3},
            .num_infosets = {1, 1},
            .num_sequences = {4, 4},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 629,
            .num_terminals = 489,
            .num_states = 515,
            .num_histories = {26, 114},
            .num_infosets = {16, 16},
            .num_sequences = {72, 72},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 175,
            .num_terminals = 112,
            .num_states = 129,
            .num_histories = {17, 46},
            .num_infosets = {9, 9},
            .num_sequences = {26, 26},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 51,
            .num_terminals = 33,
            .num_states = 38,
            .num_histories = {5, 13},
            .num_infosets = {5, 5},
            .num_sequences = {14, 14},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 37,
            .num_terminals = 21,
            .num_states = 26,
            .num_histories = {5, 11},
            .num_infosets = {5, 5},
            .num_sequences = {12, 12},
        }, {
            .max_EFGDepth = 4,
            .max_StateDepth = 3,
            .num_nodes = 159,
            .num_terminals = 98,
            .num_states = 115,
            .num_histories = {17, 44},
            .num_infosets = {9, 9},
            .num_sequences = {25, 25},
        },
    };

    for (int i = 0; i < expectedStats.size(); ++i) {
        RandomGameDomain domain(p_game_settings[i]);
        cout << ">> checking domain [" << i << "] " << domain.getInfo() << endl;
        DomainStatistics actualStats;
        calculateDomainStatistics(domain, &actualStats);
        EXPECT_EQ(actualStats, expectedStats[i]);
    }
}

TEST(RandomGame, gameConsistency) {
    for (auto &settings : p_game_settings) {
        RandomGameDomain rg1(settings);
        RandomGameDomain rg2(settings);
        cout << ">> checking domain " << rg1.getInfo() << endl;
        EXPECT_TRUE(gameConsistency(rg1, rg2));
    }
}
}
