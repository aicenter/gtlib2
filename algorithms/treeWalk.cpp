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

    void algorithms::treeWalkEFG(const Domain &domain, std::function<void(shared_ptr<EFGNode>)> function) {
        algorithms::treeWalkEFG(domain,function,domain.getMaxDepth());
    }

    void algorithms::treeWalkEFG(const Domain &domain, std::function<void(shared_ptr<EFGNode>)> function,
                                 int maxDepth) {

        std::function<void(const shared_ptr<EFGNode>&, int)> traverse = [&function, &traverse](
                const shared_ptr<EFGNode>& node, int depth) {

            // Call the provided function on the current node.
            // Prob is the probability that this node is reached due to nature, given that the players played
            // the required actions to reach this node.
            function(node);

            if (depth <= 0) {
                return;
            }

            const auto actions = node->availableActions();
            for (const auto &action : actions) {
                auto newNodes = node->performAction(action); // Non-deterministic - can get multiple nodes
                for (auto const& [newNode,prob] : newNodes) {
                    traverse(newNode, depth - 1);
                }
            }
        };

        auto rootNodes = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
                domain.getRootStatesDistribution());

        for (auto nodeProb : rootNodes) {
            traverse(nodeProb.first,  maxDepth);
        }
    }

    int algorithms::countNodes(const Domain &domain) {
        int nodesCounter = 0;
        auto countingFunction = [&nodesCounter](shared_ptr<EFGNode> node) {
            nodesCounter += 1;

            if (nodesCounter % 100000 == 0) {
                cout << "Number of nodes: " << nodesCounter << std::endl;
            }
        };
        algorithms::treeWalkEFG(domain, countingFunction, domain.getMaxDepth());
        return nodesCounter;
    }



}

#pragma clang diagnostic pop