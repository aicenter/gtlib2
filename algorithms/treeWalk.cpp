//
// Created by Pavel Rytir on 1/13/18.
//

#include "treeWalk.h"
#include "common.h"
#include <tuple>

using std::tuple;


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {

    void algorithms::pavelEFGTreeWalk(const Domain &domain, std::function<void(shared_ptr<EFGNode>, double)> function,
                                      const int maxDepth) {

        std::function<void(shared_ptr<EFGNode>, double, int)> traverse = [&function, &traverse](
                shared_ptr<EFGNode> node,
                double prob, int depth) {

            // Call the provided function on the current node.
            // Prob is the probability that this node is reached due to nature, given that the players played
            // the required actions to reach this node.
            function(node, prob);

            if (depth <= 0) {
                return;
            }

            const auto actions = node->availableActions();
            for (const auto &action : actions) {
                auto newNodes = node->pavelPerformAction(action); // Non-deterministic - can get multiple nodes
                for (auto newNodeProb : newNodes) {
                    traverse(newNodeProb.first, newNodeProb.second * prob, depth - 1);
                }
            }
        };

        auto rootNodes = algorithms::createEFGNodesFromDomainInitDistr(*domain.getRootStateDistributionPtr());

        for (auto nodeProb : rootNodes) {
            traverse(nodeProb.first, nodeProb.second, maxDepth);
        }
    }

    int algorithms::countNodes(const Domain &domain) {
        int nodesCounter = 0;
        auto countingFunction = [&nodesCounter](shared_ptr<EFGNode> node, double prob) {
            nodesCounter += 1;
        };
        algorithms::pavelEFGTreeWalk(domain,countingFunction,domain.getMaxDepth());
        return nodesCounter;
    }

}

#pragma clang diagnostic pop