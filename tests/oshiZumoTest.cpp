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

#include <boost/test/unit_test.hpp>
#include "algorithms/stats.h"
#include "algorithms/tree.h"
#include "algorithms/utility.h"
#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "utils/functools.h"
#include "domains/oshiZumo.h"


using namespace GTLib2;

using domains::OshiZumoDomain;
using domains::OshiZumoState;
using domains::OshiZumoAction;
using algorithms::DomainStatistics;
using algorithms::treeWalkEFG;



BOOST_AUTO_TEST_SUITE(OshiZumo)

OshiZumoDomain testDomainsOshiZumo[]{ // NOLINT(cert-err58-cpp)
    OshiZumoDomain(3, 0, 1, false),
    OshiZumoDomain(3, 3, 1, false),
    OshiZumoDomain(1, 3, 0, false),
    OshiZumoDomain(3, 0, 1),
    OshiZumoDomain(1, 3, 0),
    OshiZumoDomain(3, 3, 1),
};

BOOST_AUTO_TEST_CASE(builtAndCheckGameTree){
    DomainStatistics expectedResults[] {
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
        },{
            .max_EFGDepth = 4,
            .max_StateDepth = 2,
            .num_nodes = 20,
            .num_terminals = 4,
            .num_states = 13,
            .num_histories = {9, 7},
            .num_infosets = {9, 4},
            .num_sequences = {8, 8},
        },{
            .max_EFGDepth = 6,
            .max_StateDepth = 3,
            .num_nodes = 23,
            .num_terminals = 12,
            .num_states = 16,
            .num_histories = {4, 7},
            .num_infosets = {4, 4},
            .num_sequences = {8, 8},
        },{
            .max_EFGDepth = 4,
            .max_StateDepth = 2,
            .num_nodes = 13,
            .num_terminals = 6,
            .num_states = 9,
            .num_histories = {3, 4},
            .num_infosets = {3, 2},
            .num_sequences = {5, 5},
        },{
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
    for (auto tuple : zip(testDomainsOshiZumo, expectedResults)){
        unzip(tuple, testDomain, expectedRes);
        DomainStatistics stats;
        calculateDomainStatistics(testDomain, &stats);
        BOOST_CHECK_EQUAL(stats, expectedRes);
    }
}


BOOST_AUTO_TEST_CASE(correctRewardsDistribution){
    OshiZumoDomain ozd(3, 0, 1);
    vector<int> rew{0, 0, 0};

    auto getRewards = [&rew](shared_ptr<EFGNode> node){
        if (node->isTerminal()){
            if (node->rewards_[0] == 0) {
                rew[0]++;
            } else if (node->rewards_[0] == 1){
                rew[1]++;
            } else {
                rew[2]++;
            }
        }
    };
    treeWalkEFG(ozd, getRewards);

    BOOST_CHECK_EQUAL(rew[0], 4);
    BOOST_CHECK_EQUAL(rew[1], 4);
    BOOST_CHECK_EQUAL(rew[2], 4);
}


BOOST_AUTO_TEST_SUITE_END()