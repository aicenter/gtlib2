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

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#include "base/base.h"
#include "algorithms/tree.h"

#include "base/efg.h"
#include "algorithms/common.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::algorithms {

void treeWalkEFG(EFGCache &cache, EFGNodeCallback function, int maxDepth) {
    auto traverse = [&function, &cache, maxDepth]
        (const shared_ptr<EFGNode> &node, const auto &traverse) {

        // Call the provided function on the current node.
        function(node);

        if (node->getDepth() >= maxDepth) return;

        for (const auto &action : node->availableActions()) {
            for (auto const &[nextNode, chanceProb] : cache.getChildrenFor(node, action)) {
                traverse(nextNode, traverse);
            }
        }
    };

    for (const auto &[rootNode, chanceProb] : cache.getRootNodes()) {
        traverse(rootNode, traverse);
    }
}

void treeWalkEFG(const Domain &domain, EFGNodeCallback function, int maxDepth) {
    auto traverse = [&function, &domain, maxDepth]
        (const shared_ptr<EFGNode> &node, const auto &traverse) {

        // Call the provided function on the current node.
        function(node);

        if (node->getDepth() >= maxDepth) return;

        for (const auto &action : node->availableActions()) {
            for (auto const &[nextNode, chanceProb] : node->performAction(action)) {
                traverse(nextNode, traverse);
            }
        }
    };

    for (const auto &[rootNode, chanceProb] : createRootEFGNodes(domain.getRootStatesDistribution())) {
        traverse(rootNode, traverse);
    }
}

}  // namespace GTLib2

#pragma clang diagnostic pop
