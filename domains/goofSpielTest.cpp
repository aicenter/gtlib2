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
#include "algorithms/common.h"
#include "algorithms/equilibrium.h"

#include "algorithms/stats.h"
#include "algorithms/utility.h"
#include "algorithms/strategy.h"
#include "domains/goofSpiel.h"

#include "gtest/gtest.h"

namespace GTLib2::domains {

using algorithms::DomainStatistics;
using algorithms::getUniformStrategy;
using algorithms::playOnlyAction;


// @formatter:off
GoofSpielDomain testDomainsGoofSpiel[] = { // NOLINT(cert-err58-cpp)
    GoofSpielDomain({ /*.variant =*/CompleteObservations,   /*.numCards=*/1, /*.fixChanceCards=*/false, /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/CompleteObservations,   /*.numCards=*/2, /*.fixChanceCards=*/false, /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/CompleteObservations,   /*.numCards=*/3, /*.fixChanceCards=*/false, /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/CompleteObservations,   /*.numCards=*/1, /*.fixChanceCards=*/true,  /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/CompleteObservations,   /*.numCards=*/2, /*.fixChanceCards=*/true,  /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/CompleteObservations,   /*.numCards=*/3, /*.fixChanceCards=*/true,  /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/IncompleteObservations, /*.numCards=*/1, /*.fixChanceCards=*/false, /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/IncompleteObservations, /*.numCards=*/2, /*.fixChanceCards=*/false, /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/IncompleteObservations, /*.numCards=*/3, /*.fixChanceCards=*/false, /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/IncompleteObservations, /*.numCards=*/1, /*.fixChanceCards=*/true,  /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/IncompleteObservations, /*.numCards=*/2, /*.fixChanceCards=*/true,  /*.chanceCards=*/{}}),
    GoofSpielDomain({ /*.variant =*/IncompleteObservations, /*.numCards=*/3, /*.fixChanceCards=*/true,  /*.chanceCards=*/{}}),
};
// @formatter:on


TEST(Goofspiel, BuildGameTreeAndCheckSizes) {
    vector<DomainStatistics> expectedStats = {
        {
            /*.max_EFGDepth   = */2,
            /*.max_StateDepth =*/2,
            /*.num_nodes      =*/3,
            /*.num_terminals  =*/1,
            /*.num_states     =*/2,
            /*.num_histories  =*/{1, 1},
            /*.num_infosets   =*/{1, 1},
            /*.num_sequences  =*/{2, 2},
        }, {
            /*.max_EFGDepth   = */5,
            /*.max_StateDepth =*/3,
            /*.num_nodes      =*/31,
            /*.num_terminals  =*/8,
            /*.num_states     =*/19,
            /*.num_histories  =*/{10, 12},
            /*.num_infosets   =*/{10, 10},
            /*.num_sequences  =*/{13, 13},
        }, {
            /*.max_EFGDepth   = */8,
            /*.max_StateDepth =*/4,
            /*.num_nodes      =*/850,
            /*.num_terminals  =*/216,
            /*.num_states     =*/490,
            /*.num_histories  =*/{273, 333},
            /*.num_infosets   =*/{273, 273},
            /*.num_sequences  =*/{334, 334},
        }, {
            /*.max_EFGDepth   = */2,
            /*.max_StateDepth =*/2,
            /*.num_nodes      =*/3,
            /*.num_terminals  =*/1,
            /*.num_states     =*/2,
            /*.num_histories  =*/{1, 1},
            /*.num_infosets   =*/{1, 1},
            /*.num_sequences  =*/{2, 2},
        },
        {
            /*.max_EFGDepth   = */4,
            /*.max_StateDepth =*/3,
            /*.num_nodes      =*/15,
            /*.num_terminals  =*/4,
            /*.num_states     =*/9,
            /*.num_histories  =*/{5, 6},
            /*.num_infosets   =*/{5, 5},
            /*.num_sequences  =*/{7, 7},
        }, {
            /*.max_EFGDepth   = */6,
            /*.max_StateDepth =*/4,
            /*.num_nodes      =*/139,
            /*.num_terminals  =*/36,
            /*.num_states     =*/82,
            /*.num_histories  =*/{46, 57},
            /*.num_infosets   =*/{46, 46},
            /*.num_sequences  =*/{58, 58},
        }, {
            /*.max_EFGDepth   = */2,
            /*.max_StateDepth =*/2,
            /*.num_nodes      =*/3,
            /*.num_terminals  =*/1,
            /*.num_states     =*/2,
            /*.num_histories  =*/{1, 1},
            /*.num_infosets   =*/{1, 1},
            /*.num_sequences  =*/{2, 2},
        }, {
            /*.max_EFGDepth   = */5,
            /*.max_StateDepth =*/3,
            /*.num_nodes      =*/31,
            /*.num_terminals  =*/8,
            /*.num_states     =*/19,
            /*.num_histories  =*/{10, 12},
            /*.num_infosets   =*/{10, 10},
            /*.num_sequences  =*/{13, 13},
        }, {
            /*.max_EFGDepth   = */8,
            /*.max_StateDepth =*/4,
            /*.num_nodes      =*/850,
            /*.num_terminals  =*/216,
            /*.num_states     =*/490,
            /*.num_histories  =*/{273, 333},
            /*.num_infosets   =*/{213, 213},
            /*.num_sequences  =*/{262, 262},
        }, {
            /*.max_EFGDepth   = */2,
            /*.max_StateDepth =*/2,
            /*.num_nodes      =*/3,
            /*.num_terminals  =*/1,
            /*.num_states     =*/2,
            /*.num_histories  =*/{1, 1},
            /*.num_infosets   =*/{1, 1},
            /*.num_sequences  =*/{2, 2},
        }, {
            /*.max_EFGDepth   = */4,
            /*.max_StateDepth =*/3,
            /*.num_nodes      =*/15,
            /*.num_terminals  =*/4,
            /*.num_states     =*/9,
            /*.num_histories  =*/{5, 6},
            /*.num_infosets   =*/{5, 5},
            /*.num_sequences  =*/{7, 7},
        }, {
            /*.max_EFGDepth   = */6,
            /*.max_StateDepth =*/4,
            /*.num_nodes      =*/139,
            /*.num_terminals  =*/36,
            /*.num_states     =*/82,
            /*.num_histories  =*/{46, 57},
            /*.num_infosets   =*/{36, 36},
            /*.num_sequences  =*/{46, 46},
        }
    };

    for (unsigned int i = 0; i < expectedStats.size(); ++i) {
        cout << ">> checking domain [" << i << "] " << testDomainsGoofSpiel[i].getInfo() << endl;
        DomainStatistics actualStats;
        calculateDomainStatistics(testDomainsGoofSpiel[i], &actualStats);
        EXPECT_EQ(actualStats, expectedStats[i]);
    }
}

TEST(Goofspiel, checkBinaryUtilities) {
    auto binary = GoofSpielDomain({
                                      /*.variant= */CompleteObservations,
                                      /*.numCards=*/3,
                                      /*.fixChanceCards=*/false,
                                      /*.chanceCards=*/{},
                                      /*.binaryTerminalRewards=*/true
                                  });
    auto nonBinary = GoofSpielDomain({
                                         /*.variant=*/IncompleteObservations,
                                         /*.numCards=*/3,
                                         /*.fixChanceCards=*/false,
                                         /*.chanceCards=*/{},
                                         /*.binaryTerminalRewards=*/false
                                     });

    int numViolations;
    auto binaryChecker = [&numViolations](shared_ptr<EFGNode> node) {
        if (node->type_ != TerminalNode) return;
        if (abs(node->getUtilities()[0]) - 1.0 > 1e-5
            && node->getUtilities()[0] > 1e-5)
            numViolations++;
    };

    numViolations = 0;
    treeWalk(binary, binaryChecker);
    EXPECT_EQ(numViolations, 0);

    numViolations = 0;
    treeWalk(nonBinary, binaryChecker);
    EXPECT_EQ(numViolations, 42);
}

// todo: create an actual domain test!
//  i.e. given some game position, these are the action available etc.
//  for inspiration look at kriegspieltest!





}  // namespace GTLib2
