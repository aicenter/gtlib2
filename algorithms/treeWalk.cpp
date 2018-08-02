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
        algorithms::treeWalkEFG(domain, move(function),domain.getMaxDepth());
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
        auto sequences = unordered_map<int,unordered_set<ActionSequence>>();
        sequences[domain.getPlayers()[0]] = unordered_set<ActionSequence>();
        sequences[domain.getPlayers()[1]] = unordered_set<ActionSequence>();
        int statesCounter = 0;
        int st = 0;
        int st2 = 0;
        auto infSets = unordered_set<shared_ptr<AOH>>();
        auto countingFunction = [&nodesCounter, &sequences, &infSets, &statesCounter, &domain, &st, &st2](shared_ptr<EFGNode> node) {

            if(node->getCurrentPlayer()) {
              ++nodesCounter;
              int currentPlayer = *node->getCurrentPlayer();
              if(currentPlayer == domain.getPlayers()[0]) ++st;
              else ++st2;
              auto infSet = node->getAOHInfSet();
              infSets.emplace(infSet);
              for(auto &player : domain.getPlayers()) {
                  auto seq = node->getActionsSeqOfPlayer(player);
                  sequences[player].emplace(seq);
              }
              if (nodesCounter % 10000 == 0) {
                cout << "Number of nodes: " << nodesCounter << "\n";
              }
            }

            if(!node->getParent() || node->getParent()->getState() != node->getState()) {
              ++statesCounter;
            }

        };
        algorithms::treeWalkEFG(domain, countingFunction, domain.getMaxDepth());
        cout << "Number of states: " << statesCounter <<"\n";
        cout << "Number of nodes of P1: " << st <<"\n";
        cout << "Number of nodes of P2: " << st2 <<"\n";
        cout << "Number of IS: " << infSets.size()  <<"\n";
        cout << sequences.at(domain.getPlayers()[0]).size() <<" " << sequences.at(domain.getPlayers()[1]).size() <<"\n";
        return nodesCounter;
    }
}

#pragma clang diagnostic pop