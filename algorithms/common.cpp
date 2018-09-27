//
// Created by Pavel Rytir on 1/21/18.
//

#include <utility>
#include "algorithms/common.h"
#include "algorithms/treeWalk.h"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {
namespace algorithms {
EFGNodesDistribution
createRootEFGNodesFromInitialOutcomeDistribution(const OutcomeDistribution &probDist) {
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
    if (*node->getCurrentPlayer() == player1) {
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

  auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
      domain.getRootStatesDistribution());

  for (const auto &rootNode : rootNodes) {
    traverse(rootNode.first, 1, infSet->getInitialObservationId());
  }

  return nodes;
}
}  // namespace algorithms
}  // namespace GTLib2

#pragma clang diagnostic pop
