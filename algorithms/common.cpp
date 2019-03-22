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


#include <utility>
#include "algorithms/common.h"
#include "algorithms/treeWalk.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {
namespace algorithms {
EFGNodesDistribution
createRootEFGNodes(const OutcomeDistribution &probDist) {
  EFGNodesDistribution nodes;

  for (auto &outcomeProb : probDist) {
    auto &outcome = outcomeProb.first;
    auto prob = outcomeProb.second;
    auto node = make_shared<EFGNode>(outcome.state, nullptr,
                                     outcome.observations,
                                     outcome.rewards,
                                     prob, nullptr, 0);
    nodes.emplace_back(move(node), prob);
  }
  return nodes;
}

BehavioralStrategy
mixedToBehavioralStrategy(const vector<BehavioralStrategy> &pureStrats, int player1,
                          const vector<double> &distribution, const Domain &domain) {
  BehavioralStrategy behavStrat;

  auto updateBehavStrategy = [&behavStrat, &player1](shared_ptr<EFGNode> node) {
    if (node->getCurrentPlayer() && *node->getCurrentPlayer() == player1) {
      auto infSet = node->getAOHInfSet();
      if (behavStrat.find(infSet) == behavStrat.end()) {
        behavStrat[infSet] = unordered_map<shared_ptr<Action>, double>();
        for (auto &action : node->availableActions()) {
          behavStrat[infSet][action] = 0.0;
        }
      }
    }
  };

  treeWalkEFG(domain, updateBehavStrategy, domain.getMaxDepth());

  for (int i = 0; i < pureStrats.size(); ++i) {
    for (auto &strat : pureStrats[i]) {
      for (auto &action : strat.second) {
        behavStrat[strat.first][action.first] += action.second * distribution[i];
      }
    }
  }
  return behavStrat;
}

EFGNodesDistribution
getAllNodesInTheInformationSetWithNatureProbability(const shared_ptr<AOH> &infSet,
                                                    const Domain &domain) {
  vector<pair<shared_ptr<EFGNode>, double>> nodes;

  auto aoh = infSet->getAOHistory();
  int player = infSet->getPlayer();

  std::function<void(shared_ptr<EFGNode>, int, int)> traverse =
      [&nodes, &aoh, &player, &traverse, &infSet](shared_ptr<EFGNode> node,
                                                  int actionIndex, int observationIdToCheck) {
        if (node->getNumberOfRemainingPlayers() == 1 && node->noActionPerformedInThisRound()) {
          if (node->getCurrentPlayer() && *node->getCurrentPlayer() == player) {
            if (observationIdToCheck == node->getLastObservationIdOfCurrentPlayer()) {
              if (actionIndex >= aoh.size()) {
                if (node->isContainedInInformationSet(infSet)) {
                  nodes.emplace_back(node, node->natureProbability);
                  return;
                }
              }
              auto observationId = std::get<1>(aoh[actionIndex]);
              auto actions = node->availableActions();
              for (const auto &action : actions) {
                auto nextNodes = node->performAction(action);
                for (const auto &nextNode : nextNodes) {
                  traverse(nextNode.first, actionIndex + 1, observationId);
                }
              }
            }
          } else {
            if (node->getLastObservationOfPlayer(player) == observationIdToCheck) {
              auto actions = node->availableActions();
              auto observationId = std::get<1>(aoh[actionIndex]);
              for (const auto &action : actions) {
                auto nextNodes = node->performAction(action);
                for (const auto &nextNode : nextNodes) {
                  traverse(nextNode.first, actionIndex + 1, observationId);
                }
              }
            }
          }
        } else {
          if (node->getCurrentPlayer() && *node->getCurrentPlayer() == player) {
            if (node->getLastObservationIdOfCurrentPlayer() == observationIdToCheck) {
              if (actionIndex >= aoh.size()) {
                nodes.emplace_back(node, node->natureProbability);
                assert(node->isContainedInInformationSet(infSet));
                return;
              }
              auto actionId = std::get<0>(aoh[actionIndex]);
              auto observationId = std::get<1>(aoh[actionIndex]);
              auto actions = node->availableActions();
              for (const auto &action : actions) {
                if (action->getId() == actionId) {
                  auto nextNodes = node->performAction(action);
                  for (const auto &nextNode : nextNodes) {
                    traverse(nextNode.first, actionIndex + 1, observationId);
                  }
                }
              }
            }
          } else {
            auto actions = node->availableActions();
            for (const auto &action : actions) {
              auto nextNodes = node->performAction(action);
              for (const auto &nextNode : nextNodes) {
                traverse(nextNode.first, actionIndex, observationIdToCheck);
              }
            }
          }
        }
      };

//        auto checkAndAdd = [&infSet, &nodes](shared_ptr<EFGNode> node) {
//            if (node->isContainedInInformationSet(infSet)) {
//                nodes.emplace_back(node,node->natureProbability);
//            }
//        };
//
//        treeWalkEFG(domain, checkAndAdd, domain.getMaxDepth());

  auto rootNodes = createRootEFGNodes(
      domain.getRootStatesDistribution());

  for (const auto &rootNode : rootNodes) {
    traverse(rootNode.first, 1, infSet->getInitialObservationId());
  }

  return nodes;
}
}  // namespace algorithms
}  // namespace GTLib2

#pragma clang diagnostic pop
