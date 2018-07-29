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

        std::function<void(const shared_ptr<EFGNode>&, int)> traverse = [&function,&domain, &traverse](
                const shared_ptr<EFGNode>& node, int depth) {

            // Call the provided function on the current node.
            // Prob is the probability that this node is reached due to nature, given that the players played
            // the required actions to reach this node.

            if (depth <= 0) {
                return;
            }
          function(node);
          const auto actions = node->availableActions();
          for (const auto &action : actions) {
            auto newNodes = node->performAction(action); // Non-deterministic - can get multiple nodes
            for (auto const& it : newNodes) {
                auto newNode = it.first;
                int newDepth = newNode->getState()== node->getState() ? depth : depth -1;
                traverse(newNode, newDepth);
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
        auto sequences = unordered_map<int,std::set<ActionSequence>>();
        sequences[0] = std::set<ActionSequence>();
        sequences[1] = std::set<ActionSequence>();
        int statesCounter = 0;
        auto infSets = unordered_set<shared_ptr<AOH>>();
        auto countingFunction = [&nodesCounter, &sequences, &infSets, &statesCounter](shared_ptr<EFGNode> node) {

            int currentPlayer = *node->getCurrentPlayer();
            ++nodesCounter;
            auto infSet = node->getAOHInfSet();
            infSets.emplace(infSet);
            auto seq = node->getActionsSeqOfPlayer(currentPlayer);
            sequences[currentPlayer].emplace(seq);
            if(!node->getParent() || node->getParent()->getState() != node->getState()) {
              ++statesCounter;
            }
            if (nodesCounter % 10000 == 0) {
                cout << "Number of nodes: " << nodesCounter << "\n";
            }
        };
        algorithms::treeWalkEFG(domain, countingFunction, domain.getMaxDepth());
        cout << sequences.at(0).size() <<" " << sequences.at(1).size() <<"\n";
        cout << "Number of IS: " << infSets.size()  <<"\n";
        cout << "Number of states: " << statesCounter <<"\n";
        return nodesCounter;
    }
}

#pragma clang diagnostic pop