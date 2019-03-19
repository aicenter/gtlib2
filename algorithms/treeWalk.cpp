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


#include "algorithms/treeWalk.h"

#include <unordered_set>
#include <utility>
#include <boost/range/combine.hpp>

#include "algorithms/common.h"

using std::unordered_set;
using std::cout;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {
namespace algorithms {


void treeWalkEFG(const Domain &domain, EFGNodeCallback function, int maxDepth) {
    auto traverse = [&function, &domain, maxDepth]
        (const shared_ptr<EFGNode> &node, const auto &traverse) {

        // Call the provided function on the current node.
        function(node);

        if (node->getDepth() >= maxDepth) return;

        const auto actions = node->availableActions();
        for (const auto &action : actions) {
            // Non-deterministic - can get multiple nodes due to chance
            auto nodesDistribution = node->performAction(action);
            for (auto const &nodeDist : nodesDistribution) {
                traverse(nodeDist.first, traverse);
            }
        }
    };

    auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
        domain.getRootStatesDistribution());
    for (const auto &nodeDist : rootNodes) {
        traverse(nodeDist.first, traverse);
    }
}

}  // namespace algorithms
}  // namespace GTLib2

#pragma clang diagnostic pop
