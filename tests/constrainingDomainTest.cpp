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

#include <domains/goofSpiel.h>
#include <domains/stratego.h>
#include "gtest/gtest.h"

namespace GTLib2::algorithms {

struct TestCase {
    const ConstrainingDomain &domain;
    const shared_ptr<AOH> &targetInfoset;
    const int expectedNodesCount;
};

vector<EFGNodeGenerator> testGenerators = {
    domainSpecificNodeGenerator,
//        cspNodeGenerator // to be implemented
//        emptyNodeGenerator // does nothing, so it will not pass test
};

using namespace GTLib2::domains;

TEST(ConstrainingDomain, CheckExactInfosets) {
    vector<TestCase> testCases;

    // IIGS-5
    auto domain1 = GoofSpielDomain::IIGS(5);
    vector<ActionObservationIds> targetAOids1 = {
        {NO_ACTION, 21}, {0, observationPlayerMove(0)},
        {NO_ACTION, 4113}, {3, observationPlayerMove(0)},
        {NO_ACTION, 20494}, {1, observationPlayerMove(0)},
        {NO_ACTION, 12296}, {1, observationPlayerMove(0)},
        {NO_ACTION, 16390}, {NO_ACTION, observationPlayerMove(0)}
    };
    auto targetInfoset1 = make_shared<AOH>(Player(0), true, targetAOids1);
    testCases.push_back({*domain1, targetInfoset1, 6});


    // STRAT4x4.
    auto domain2 = StrategoDomain::STRAT4x4();
    // (no figures revealed)
    vector<ActionObservationIds> targetAOids20 = {
        {NO_ACTION, NO_OBSERVATION}, {0, 1074544640},
        {NO_ACTION, 1342177280}, {0, 1074561025},
        {NO_ACTION, 1342177281}, {0, 1074561026},
        {NO_ACTION, 1342177282}, {0, 1074577411},
        {NO_ACTION, 1342177283}, {0, 270368},
        {NO_ACTION, 101195808}, {0, 8396832},
        {NO_ACTION, 109846560}, {NO_ACTION, observationPlayerMove(0)},
    };
    auto targetInfoset20 = make_shared<AOH>(Player(0), true, targetAOids20);
    testCases.push_back({*domain2, targetInfoset20, 12});
    // (1 figure revealed, revealed a rank '3')
    vector<ActionObservationIds> targetAOids21 = {
        {NO_ACTION, NO_OBSERVATION}, {0, 1074544640},
        {NO_ACTION, 1342177280}, {0, 1074561025},
        {NO_ACTION, 1342177281}, {0, 1074561026},
        {NO_ACTION, 1342177282}, {0, 1074577411},
        {NO_ACTION, 1342177283}, {0, 270368},
        {NO_ACTION, 101195808}, {0, 8396832},
        {NO_ACTION, 109846560}, {0, 73760},
        {NO_ACTION, 101523488}, {0, 8396832},
        {NO_ACTION, 67903520}, {1, 16850976},
        {NO_ACTION, 126558240}, {3, 34086944},
        {NO_ACTION, 118431776}, {2, 25305120},
        {NO_ACTION, 109977632}, {1, 16982048},
        {NO_ACTION, 101523488}, {1, 8527904},
        {NO_ACTION, 92741664}, {4, 67379232},
        {NO_ACTION, 126558240}, {1, 16850976},
        {NO_ACTION, 118431776}, {2, 25637811},
        {NO_ACTION, 92741664}, {NO_ACTION, observationPlayerMove(0)},
    };
    auto targetInfoset21 = make_shared<AOH>(Player(0), true, targetAOids21);
    testCases.push_back({*domain2, targetInfoset21, 3});

    // (2 figures revealed, revealed a rank '2')
    vector<ActionObservationIds> targetAOids22 = {
        {NO_ACTION, NO_OBSERVATION}, {0, 1074544640},
        {NO_ACTION, 1342177280}, {0, 1074561025},
        {NO_ACTION, 1342177281}, {0, 1074561026},
        {NO_ACTION, 1342177282}, {0, 1074577411},
        {NO_ACTION, 1342177283}, {0, 270368},
        {NO_ACTION, 101195808}, {0, 8396832},
        {NO_ACTION, 109846560}, {0, 73760},
        {NO_ACTION, 101523488}, {0, 8396832},
        {NO_ACTION, 67903520}, {1, 16850976},
        {NO_ACTION, 126558240}, {3, 34086944},
        {NO_ACTION, 118431776}, {2, 25305120},
        {NO_ACTION, 109977632}, {1, 16982048},
        {NO_ACTION, 101523488}, {1, 8527904},
        {NO_ACTION, 92741664}, {4, 67379232},
        {NO_ACTION, 126558240}, {1, 16850976},
        {NO_ACTION, 118431776}, {2, 25637811},
        {NO_ACTION, 92741664}, {0, 8527904},
        {NO_ACTION, 59449376}, {3, 34086944},
        {NO_ACTION, 109977632}, {2, 16982048},
        {NO_ACTION, 92741664}, {4, 67379232},
        {NO_ACTION, 118300704}, {0, 73760},
        {NO_ACTION, 59449376}, {3, 25632800},
        {NO_ACTION, 109846560}, {3, 34086944},
        {NO_ACTION, 92779058}, {2, 67379232},
        {NO_ACTION, 126558240}, {3, 34086944},
        {NO_ACTION, 101523488}, {NO_ACTION, observationPlayerMove(0)},
    };
    auto targetInfoset22 = make_shared<AOH>(Player(0), true, targetAOids22);
    testCases.push_back({*domain2, targetInfoset22, 2});

    // (3 figures revealed, revealed a rank '1')
    vector<ActionObservationIds> targetAOids23 = {
        {NO_ACTION, NO_OBSERVATION}, {0, 1074544640},
        {NO_ACTION, 1342177280}, {0, 1074561025},
        {NO_ACTION, 1342177281}, {0, 1074561026},
        {NO_ACTION, 1342177282}, {0, 1074577411},
        {NO_ACTION, 1342177283}, {0, 270368},
        {NO_ACTION, 101195808}, {0, 8396832},
        {NO_ACTION, 109846560}, {0, 73760},
        {NO_ACTION, 101523488}, {0, 8396832},
        {NO_ACTION, 67903520}, {1, 16850976},
        {NO_ACTION, 126558240}, {3, 34086944},
        {NO_ACTION, 118431776}, {2, 25305120},
        {NO_ACTION, 109977632}, {1, 16982048},
        {NO_ACTION, 101523488}, {1, 8527904},
        {NO_ACTION, 92741664}, {4, 67379232},
        {NO_ACTION, 126558240}, {1, 16850976},
        {NO_ACTION, 118431776}, {2, 25637811},
        {NO_ACTION, 92741664}, {0, 8527904},
        {NO_ACTION, 59449376}, {3, 34086944},
        {NO_ACTION, 109977632}, {2, 16982048},
        {NO_ACTION, 92741664}, {4, 67379232},
        {NO_ACTION, 118300704}, {0, 73760},
        {NO_ACTION, 59449376}, {3, 25632800},
        {NO_ACTION, 109846560}, {3, 34086944},
        {NO_ACTION, 92779058}, {2, 67379232},
        {NO_ACTION, 126558240}, {3, 34086944},
        {NO_ACTION, 101523488}, {1, 8396832},
        {NO_ACTION, 92741664}, {3, 67903520},
        {NO_ACTION, 58925088}, {3, 101527986},
        {NO_ACTION, 109977632}, {0, 73760},
        {NO_ACTION, 25632800}, {1, 8396832},
        {NO_ACTION, 59449376}, {1, 270368},
        {NO_ACTION, 93265952}, {1, 34086944},
        {NO_ACTION, 118300704}, {1, 67903520},
        {NO_ACTION, 126754848}, {NO_ACTION, observationPlayerMove(0)},
    };
    auto targetInfoset23 = make_shared<AOH>(Player(0), true, targetAOids23);
    testCases.push_back({*domain2, targetInfoset23, 1});

    for (const auto &generator : testGenerators) {
        for (int i = 0; i < testCases.size(); ++i) {
            const auto testCase = testCases[i];
            cout << "Test case " << i << "\n";

            vector<shared_ptr<EFGNode>> actualNodes;
            generator(testCase.domain, testCase.targetInfoset,
                      BudgetIterations, testCase.expectedNodesCount + 1000,
                      [&](const shared_ptr<EFGNode> &node) {
                          actualNodes.push_back(node);
                          EXPECT_EQ(*testCase.targetInfoset, *node->getAOHInfSet());
                      });
            EXPECT_EQ(actualNodes.size(), testCase.expectedNodesCount);
        }
    }
}


TEST(ConstrainingDomain, CheckInfosetConsistency) {
    // pair<domain, skipDepth>
    vector<pair<shared_ptr<ConstrainingDomain>, int>> domains = {
        make_pair(StrategoDomain::STRAT2x2(), 3), // no need to check nodes in setup state
        make_pair(GoofSpielDomain::IIGS(5), 0)
    };

    for (EFGNodeGenerator &generator : testGenerators) {
        for (const auto &[domain, skipDepth] : domains) {
            auto cache = InfosetCache(dynamic_cast<const Domain &>(*domain));
            cache.buildTree();

            auto mapping = cache.getInfoset2NodeMapping();
            const auto historySort = [](const shared_ptr<EFGNode> &a,
                                        const shared_ptr<EFGNode> &b) {
                return a->getHistory() < b->getHistory();
            };

            for (auto&[targetInfoset, expectedNodes] : mapping) {
                if (expectedNodes[0]->efgDepth() < 2 * skipDepth) continue;
                if (expectedNodes[0]->type_ != PlayerNode) continue;
                if (!targetInfoset->isPlayerActing()) continue;

                vector<shared_ptr<EFGNode>> actualNodes;
                generator(*domain, targetInfoset, BudgetIterations,
                          expectedNodes.size() + 1, // let's make sure we get all the nodes
                          [&](const shared_ptr<EFGNode> &node) { actualNodes.push_back(node); });

                std::sort(actualNodes.begin(), actualNodes.end(), historySort);
                std::sort(expectedNodes.begin(), expectedNodes.end(), historySort);

                bool ok = std::equal(begin(actualNodes), end(actualNodes),
                                     begin(expectedNodes), end(expectedNodes),
                                     [](const shared_ptr<EFGNode> lhs,
                                        const shared_ptr<EFGNode> rhs) { return *lhs == *rhs; });
                EXPECT_TRUE(ok);
            }
        }
    }
}
}
