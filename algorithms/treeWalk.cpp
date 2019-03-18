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


#include <unordered_set>
#include "algorithms/treeWalk.h"
#include "algorithms/common.h"

using std::unordered_set;
using std::cout;

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
  auto traverse = [&function, &domain, maxDepth]
      (const shared_ptr<EFGNode> &node, const auto &traverse) {
    // Call the provided function on the current node.
    // Prob is the probability that this node is reached due to nature,
    // given that the players played
    // the required actions to reach this node.

    if (node->getDepth() == maxDepth) {
      return;
    }
    function(node);
    const auto actions = node->availableActions();
    for (const auto &action : actions) {
      auto newNodes = node->performAction(action);  // Non-deterministic - can get multiple nodes
      for (auto const &it : newNodes) {
        traverse(it.first, traverse);
      }
    }
  };

  auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
      domain.getRootStatesDistribution());
  for (auto nodeProb : rootNodes) {
    traverse(nodeProb.first, traverse);
  }
}

int countNodesInfSetsSequencesStates(const Domain &domain) {
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
    }

    if (!node->getParent() || node->getParent()->getDepth() != node->getDepth()) {
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

int countNodes(const Domain &domain) {
  int numberOfNodes = 0;
  auto countingFunction = [&numberOfNodes, &domain](shared_ptr<EFGNode> node) {
    if (node->getCurrentPlayer()) {
      ++numberOfNodes;
      if (numberOfNodes % 10000 == 0) {
        cout << numberOfNodes <<"\n";
      }
    }
  };
  algorithms::treeWalkEFG(domain, countingFunction, domain.getMaxDepth());
  return numberOfNodes;
}

}  // namespace algorithms
}  // namespace GTLib2

#pragma clang diagnostic pop
