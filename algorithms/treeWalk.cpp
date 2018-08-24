//
// Created by Pavel Rytir on 1/13/18.
//

#include "treeWalk.h"
#include <tuple>
#include "common.h"

using std::tuple;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2 {
namespace algorithms {
void treeWalkEFG(const Domain &domain,
                 std::function<void(shared_ptr<EFGNode>)> function) {
  treeWalkEFG(domain, move(function), domain.getMaxDepth());
}

void treeWalkEFG(const Domain &domain,
                 std::function<void(shared_ptr<EFGNode>)> function, int maxDepth) {
  auto traverse = [&function, &domain]
      (const shared_ptr<EFGNode> &node, int depth, const auto &traverse) {
    // Call the provided function on the current node.
    // Prob is the probability that this node is reached due to nature,
    // given that the players played
    // the required actions to reach this node.

    if (depth <= 0) {
      return;
    }
    function(node);
    const auto actions = node->availableActions();
    for (const auto &action : actions) {
      auto newNodes = node->performAction(action);  // Non-deterministic - can get multiple nodes
      for (auto const &it : newNodes) {
        int newDepth = it.first->getState() == node->getState() ? depth : depth - 1;
        traverse(it.first, newDepth, traverse);
      }
    }
  };

  auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
      domain.getRootStatesDistribution());
  for (auto nodeProb : rootNodes) {
    traverse(nodeProb.first, maxDepth, traverse);
  }
}

int countNodes(const Domain &domain) {
  int nodesCounter = 0;
  auto sequences = unordered_map<int, unordered_set<ActionSequence>>();
  sequences[domain.getPlayers()[0]] = unordered_set<ActionSequence>();
  sequences[domain.getPlayers()[1]] = unordered_set<ActionSequence>();
  int statesCounter = 0;
  int st = 0;
  int st2 = 0;
  auto infSets = unordered_set<shared_ptr<AOH>>();
  auto countingFunction = [&nodesCounter, &sequences, &infSets, &statesCounter,
      &domain, &st, &st2](shared_ptr<EFGNode> node) {
    if (node->getCurrentPlayer()) {
      ++nodesCounter;
      int currentPlayer = *node->getCurrentPlayer();
      if (currentPlayer == domain.getPlayers()[0])
        ++st;
      else
        ++st2;
      auto infSet = node->getAOHInfSet();
      infSets.emplace(infSet);
      for (auto &player : domain.getPlayers()) {
        auto seq = node->getActionsSeqOfPlayer(player);
        sequences[player].emplace(seq);
      }
//              if (nodesCounter % 10000 == 0) {
//                cout << "Number of nodes: " << nodesCounter << "\n";
//              }
    }

    if (!node->getParent() || node->getParent()->getState() != node->getState()) {
      ++statesCounter;
    }
  };
  treeWalkEFG(domain, countingFunction, domain.getMaxDepth());
  cout << "Number of states: " << statesCounter << "\n";
  cout << "Number of nodes of P1: " << st << "\n";
  cout << "Number of nodes of P2: " << st2 << "\n";
  cout << "Number of IS: " << infSets.size() << "\n";
  cout << sequences.at(domain.getPlayers()[0]).size() <<
       " " << sequences.at(domain.getPlayers()[1]).size() << "\n";
  return nodesCounter;
}
}  // namespace algorithms
}  // namespace GTLib2

#pragma clang diagnostic pop
