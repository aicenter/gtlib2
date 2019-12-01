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
TEST(DDISMCTS, CheckInfosetConsistency) {
    vector<pair<shared_ptr<ConstrainingDomain>, int>> domains = {make_pair(GTLib2::domains::StrategoDomain::STRAT2x2(), 3),
                                                                 make_pair(GTLib2::domains::GoofSpielDomain::IIGS(5), 0)};
    for(const auto & [domain, depth] : domains)
    {
        auto cache = InfosetCache(dynamic_cast<const Domain &>(*domain));
        cache.buildTree();
        auto mapping = cache.getInfoset2NodeMapping();
        const auto historySort = [](const shared_ptr<EFGNode> &a, const shared_ptr<EFGNode> &b) {
            return a->getHistory() < b->getHistory();
        };
        for (auto&[infoset, expectedNodes] : mapping) {
            if (expectedNodes[0]->efgDepth() < 2*depth) continue; //no need for nodes in setup state
            if (expectedNodes[0]->type_ != PlayerNode) continue;
            // a shorthand for distinguishing augmented infosets from ordinary ones
            if (expectedNodes[0]->getPlayer() != infoset->getPlayer()) continue;
            ConstraintsMap revealed_;
            domain->initializeEnumerativeConstraints(revealed_);
            long ind = depth-1;
            domain->updateConstraints(infoset, ind, revealed_);
            vector<shared_ptr<EFGNode>> actualNodes;
            domain->generateNodes(infoset, revealed_, BudgetIterations,
                                  expectedNodes.size() + 1, // let's make sure we get all the nodes
                                  [&](const shared_ptr<EFGNode> &node) -> double {
                                      actualNodes.push_back(node);
                                      return 0;
                                  });
            std::sort(actualNodes.begin(), actualNodes.end(), historySort);
            std::sort(expectedNodes.begin(), expectedNodes.end(), historySort);
            bool ok = equal(begin(actualNodes), end(actualNodes), begin(expectedNodes), end(expectedNodes),
                            [](const shared_ptr<EFGNode> lhs, const shared_ptr<EFGNode> rhs) { return *lhs == *rhs; });
            EXPECT_TRUE(ok);
        }
    }
}
}
