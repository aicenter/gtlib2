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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

#include "algorithms/utility.h"
#include "algorithms/common.h"
#include "algorithms/treeWalk.h"

namespace GTLib2 {
namespace algorithms {

pair<double, double> computeUtilityTwoPlayersGame(const Domain &domain,
                                                  const BehavioralStrategy &player1Strat,
                                                  const BehavioralStrategy &player2Strat,
                                                  const int player1,
                                                  const int player2) {
  // Inner function
  std::function<pair<double, double>(shared_ptr<EFGNode>, double)> calculate =
      [&player1Strat, &player2Strat, &player1, &player2, &domain, &calculate]
          (shared_ptr<EFGNode> node, double prob) {
        auto findActionProb = [](const shared_ptr<AOH> &infSet,
                                 const BehavioralStrategy &strat,
                                 const shared_ptr<Action> &action) -> double {
          return (strat.at(infSet).find(action) != strat.at(infSet).end()) ?
                 strat.at(infSet).at(action) : 0.0;
        };

        if (node->getDepth() == domain.getMaxDepth() || !node->getCurrentPlayer()) {
          return pair<double, double>(node->rewards[player1] * prob,
                                      node->rewards[player2] * prob);
        }
        double p1Util = 0.0;
        double p2Util = 0.0;
        const auto actions = node->availableActions();
        for (const auto &action : actions) {
          auto infSet = node->getAOHInfSet();
          double actionStratProb = findActionProb(infSet,
                                                  (*node->getCurrentPlayer() == player1) ?
                                                  player1Strat : player2Strat, action);
          if (actionStratProb > 0) {
            // Non-deterministic - can get multiple nodes
            auto newNodes = node->performAction(action);
            for (auto newNodeProb : newNodes) {
              auto util = calculate(newNodeProb.first,
                                    actionStratProb * newNodeProb.second * prob);
              p1Util += util.first;
              p2Util += util.second;
            }
          }
        }
        return pair<double, double>(p1Util, p2Util);
      };
  // Inner function end

  double player1Utility = 0.0;
  double player2Utility = 0.0;
  auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
      domain.getRootStatesDistribution());
  for (auto nodeProb : rootNodes) {
    auto utility = calculate(nodeProb.first, nodeProb.second);
    player1Utility += utility.first;
    player2Utility += utility.second;
  }
  return pair<double, double>(player1Utility, player2Utility);
}

unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>
generateInformationSetsAndAvailableActions(const Domain &domain, const int player) {
  unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>> infSetsAndActions;

  std::function<void(shared_ptr<EFGNode>)> extract =
      [&infSetsAndActions, &player](shared_ptr<EFGNode> node) {
        if (node->getCurrentPlayer() && *node->getCurrentPlayer() == player) {
          auto infSet = node->getAOHInfSet();
          auto actions = node->availableActions();
          if (!actions.empty()) {
            infSetsAndActions[infSet] = actions;
          }
        }
      };

  treeWalkEFG(domain, extract, domain.getMaxDepth());
  return infSetsAndActions;
}

vector<BehavioralStrategy> generateAllPureStrategies(
    const unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>> &infSetsAndActions) {

  vector<BehavioralStrategy> allPureStrats;

  vector<pair<shared_ptr<AOH>, vector<shared_ptr<Action>>>> infSetsAndActionsVector(
      infSetsAndActions.begin(), infSetsAndActions.end());

  std::function<void(BehavioralStrategy, int)> gener =
      [&gener, &infSetsAndActionsVector, &allPureStrats](BehavioralStrategy strat,
                                                         int setIndex) {
        if (setIndex >= infSetsAndActionsVector.size()) {
          allPureStrats.push_back(strat);
        } else {
          auto infSetWithActions = infSetsAndActionsVector[setIndex];
          for (auto action : infSetWithActions.second) {
            unordered_map<shared_ptr<Action>, double> actionsDistribution =
                {{action, 1.0}};
            strat[infSetWithActions.first] = actionsDistribution;
            gener(strat, setIndex + 1);
          }
        }
      };

  auto s = BehavioralStrategy();
  gener(s, 0);
  return allPureStrats;
}

tuple<vector<double>, unsigned int, unsigned int> constructUtilityMatrixFor(
    const Domain &domain, const int player,
    const vector<BehavioralStrategy> &player1PureStrats,
    const vector<BehavioralStrategy> &player2PureStrats) {

  int player1 = domain.getPlayers()[0];
  int player2 = domain.getPlayers()[1];

  const auto numberOfRows = player1PureStrats.size();
  const auto numberOfColls = player2PureStrats.size();

  vector<double> matrix(numberOfRows * numberOfColls, 0.0);
  int row = 0;
  for (const auto &stratPlayer1 : player1PureStrats) {
    int col = 0;
    for (const auto &stratPlayer2 : player2PureStrats) {
      auto u12 = computeUtilityTwoPlayersGame(domain, stratPlayer1, stratPlayer2,
                                              player1, player2);
      double u = player1 == player ? u12.first : u12.second;
      matrix[numberOfColls * row + col] = u;
      col++;
    }
    row++;
  }
  return tuple<vector<double>, unsigned int, unsigned int>(matrix, numberOfRows, numberOfColls);
}

}  // namespace algorithms
}  // namespace GTLib2
#pragma clang diagnostic pop
