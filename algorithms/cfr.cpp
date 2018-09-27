//
// Created by Jakub Rozlivek on 8/8/18.
//

#include <unordered_set>
#include "algorithms/cfr.h"
#include "algorithms/treeWalk.h"
#include "algorithms/common.h"
#include "algorithms/utility.h"

using std::make_pair;
using std::unordered_set;
using std::cout;

namespace GTLib2 {
namespace algorithms {
BehavioralStrategy getStrategyFor(const Domain &domain, int player,
    const unordered_map<shared_ptr<InformationSet>, pair<vector<double>, vector<double>>> &allMP) {
  auto strategy = BehavioralStrategy();
  auto processed = unordered_set<shared_ptr<InformationSet>>();
  processed.reserve(allMP.size());
  strategy.reserve(allMP.size());
  auto getStrategy = [&strategy, &processed, &player, &allMP](shared_ptr<EFGNode> node) {
    if (node->getCurrentPlayer() && *node->getCurrentPlayer() == player) {
      auto infSet = node->getAOHInfSet();
      if (processed.find(infSet) == processed.end()) {
        auto actionDistribution = unordered_map<shared_ptr<Action>, double>();
        const auto mp = std::get<1>(allMP.at(infSet));
        double sum = 0;
        for (double d : mp) sum += d;
        auto actions = node->availableActions();
        int i = -1;
        for (auto &action : actions) {
          actionDistribution[action] = sum == 0 ? 1.0 / mp.size() : mp[++i] / sum;
        }
        strategy.emplace(infSet, actionDistribution);
        processed.emplace(infSet);
      }
    }
  };
  treeWalkEFG(domain, getStrategy, domain.getMaxDepth());
  return strategy;
}

pair<double, double> CFR(const Domain &domain, int iterations) {
  auto regrets = CFRiterations(domain, iterations);
  auto strat1 = getStrategyFor(domain, 0, regrets);
  auto strat2 = getStrategyFor(domain, 1, regrets);
  return computeUtilityTwoPlayersGame(domain, strat1, strat2, 0, 1);
}


unordered_map<shared_ptr<InformationSet>, pair<vector<double>, vector<double>>>
CFRiterations(const Domain &domain, int iterations) {
  auto regrets = unordered_map<shared_ptr<InformationSet>,
                               pair<vector<double>, vector<double>>>();
  int nbSamples = 0;
  bool firstIteration = true;
  auto efgnodes = unordered_map<shared_ptr<EFGNode>, pair<shared_ptr<AOH>,
      unordered_map<shared_ptr<Action>, EFGNodesDistribution>>>();

  const auto iteration = [&regrets, &efgnodes, &domain, &nbSamples, &firstIteration](
      shared_ptr<EFGNode> node, double pi1, double pi2, int player, const auto &iteration) {
    if (pi1 == 0 && pi2 == 0) {
      return 0.0;
    }

    auto actions = node->availableActions();
    if (actions.empty() || node->getDepth() == domain.getMaxDepth()) {
      return node->rewards[player];
    }
    const int currentplayer = *node->getCurrentPlayer();
    const auto K = static_cast<const unsigned int>(actions.size());
    if (firstIteration) {
      efgnodes[node] = make_pair(node->getAOHInfSet(),
          unordered_map<shared_ptr<Action>, EFGNodesDistribution>());
    }
    auto[infSet, newNodesMap] = efgnodes.at(node);
    if (firstIteration && regrets.find(infSet) == regrets.end()) {
      regrets[infSet] = make_pair(vector<double>(K), vector<double>(K));
    }
    auto[r, mp] = regrets.at(infSet);
    double R = 0;
    for (double ri : r) {
      R += ri > 0 ? ri : 0;
    }
    auto rmProbs = vector<double>(K, 1.0 / K);
    if (R > 0) {
      for (int i = 0; i != K; i++)
        rmProbs[i] = r[i] > 0 ? r[i] / R : 0;
    }
    auto tmpV = vector<double>(K);
    double ev = 0;
    int i = -1;
    for (const auto &action : actions) {
      i++;
      if (firstIteration) {
        newNodesMap[action] = node->performAction(action);
      }

      auto newNodes = newNodesMap.at(action);
      for (const auto &newNode : newNodes) {
        double new_p1 = player == 1 ? pi1 * newNode.second : pi1;
        double new_p2 = player == 0 ? pi2 * newNode.second : pi2;
        if (currentplayer == 0) {
          tmpV[i] += newNode.second *
              iteration(newNode.first, new_p1 * rmProbs[i], new_p2, player, iteration);
        } else {
          tmpV[i] += newNode.second *
              iteration(newNode.first, new_p1, rmProbs[i] * new_p2, player, iteration);
        }
      }
      ev += rmProbs[i] * tmpV[i];
    }
    if (currentplayer == player) {
      for (int j = 0; j != K; j++) {
        r[j] += (player == 0 ? pi2 : pi1) * (tmpV[j] - ev);
      }
      for (int j = 0; j != K; j++) {
        mp[j] += (player == 0 ? pi1 : pi2) * rmProbs[j];
      }
      regrets[infSet] = make_pair(r, mp);
      nbSamples++;
    }
    if (firstIteration) {
      efgnodes[node] = make_pair(infSet, newNodesMap);
    }

    return ev;
  };
  auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
      domain.getRootStatesDistribution());

  for (int i = 0; i < iterations; ++i) {
    double v1 = 0, v2 = 0;
    for (const auto &nodeProb : rootNodes) {
      v1 += nodeProb.second * iteration(nodeProb.first, 1, nodeProb.second, 0, iteration);
    }
    firstIteration = false;
    for (const auto &nodeProb : rootNodes) {
      v2 += nodeProb.second * iteration(nodeProb.first, nodeProb.second, 1, 1, iteration);
    }
    cout << v1 << " " << v2 << "\n";
  }
  return regrets;
}

unordered_map<shared_ptr<InformationSet>, pair<vector<double>, vector<double>>>
CFRiterationsAOH(const Domain &domain, int iterations) {
  auto regrets = unordered_map<shared_ptr<InformationSet>,
                               pair<vector<double>, vector<double>>>();
  int nbSamples = 0;
  bool firstIteration = true;
  auto aoh1 = vector<pair<int, int>>();
  auto aoh2 = vector<pair<int, int>>();
  aoh1.reserve(domain.getMaxDepth());
  aoh2.reserve(domain.getMaxDepth());
  auto efgnodes = unordered_map<shared_ptr<EFGNode>,
                                unordered_map<shared_ptr<Action>, EFGNodesDistribution>>();
  const auto iteration = [&regrets, &efgnodes, &domain, &nbSamples, &aoh1, &aoh2,
      &firstIteration](shared_ptr<EFGNode> node, double pi1, double pi2,
                       int player, const auto &iteration) {
    if (pi1 == 0 && pi2 == 0) {
      return 0.0;
    }

    auto actions = node->availableActions();
    if (actions.empty() || node->getDepth() == domain.getMaxDepth()) {
      return node->rewards[player];
    }
    const int currentplayer = *node->getCurrentPlayer();
    const auto K = static_cast<const unsigned int>(actions.size());
    auto is =   make_shared<AOH>(currentplayer, currentplayer == 0 ? aoh1 : aoh2);

    if (firstIteration && regrets.find(is) == regrets.end()) {
      regrets[is] = make_pair(vector<double>(K), vector<double>(K));
    }
    auto[r, mp] = regrets.at(is);
    if (firstIteration) {
      efgnodes[node] = unordered_map<shared_ptr<Action>, EFGNodesDistribution>();
    }

    auto newNodesMap = efgnodes.at(node);
    double R = 0;
    for (double ri : r) {
      R += ri > 0 ? ri : 0;
    }
    auto rmProbs = vector<double>(K, 1.0 / K);
    if (R > 0) {
      for (int i = 0; i != K; ++i) {
        rmProbs[i] = r[i] > 0 ? r[i] / R : 0;
      }
    }

    auto tmpV = vector<double>(K);
    double ev = 0;

    int i = -1;
    for (const auto &action : actions) {
      i++;
      if (firstIteration) {
        newNodesMap[action] = node->performAction(action);
      }
      auto newNodes = newNodesMap.at(action);
      for (const auto &newNode : newNodes) {
        double new_p1 = player == 1 ? pi1 * newNode.second : pi1;
        double new_p2 = player == 0 ? pi2 * newNode.second : pi2;

        if (node->getNumberOfRemainingPlayers() == 1) {
          if (node->noActionPerformedInThisRound()) {
            if (currentplayer == 0) {
              aoh1.emplace_back(action->getId(), newNode.first->getLastObservationOfPlayer(0));
              aoh2.emplace_back(-1, newNode.first->getLastObservationOfPlayer(1));
            } else {
              aoh1.emplace_back(-1, newNode.first->getLastObservationOfPlayer(0));
              aoh2.emplace_back(action->getId(), newNode.first->getLastObservationOfPlayer(1));
            }
          } else {
            aoh1.emplace_back(node->getIncomingActionId(),
                              newNode.first->getLastObservationOfPlayer(0));
            aoh2.emplace_back(action->getId(), newNode.first->getLastObservationOfPlayer(1));
          }
        }
        if (currentplayer == 0) {
          tmpV[i] += newNode.second
              * iteration(newNode.first, new_p1 * rmProbs[i], new_p2, player, iteration);
        } else {
          tmpV[i] += newNode.second
              * iteration(newNode.first, new_p1, rmProbs[i] * new_p2, player, iteration);
        }
        if (node->getNumberOfRemainingPlayers() == 1) {
          aoh1.pop_back();
          aoh2.pop_back();
        }
      }
      ev += rmProbs[i] * tmpV[i];
    }
    if (currentplayer == player) {
      for (int j = 0; j < K; ++j) {
        r[j] += (player == 0 ? pi2 : pi1) * (tmpV[j] - ev);
        mp[j] += (player == 0 ? pi1 : pi2) * rmProbs[j];
      }
      regrets[is] = make_pair(r, mp);
      nbSamples++;
    }
    if (firstIteration) {
      efgnodes[node] = newNodesMap;
    }

    return ev;
  };
  auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(
      domain.getRootStatesDistribution());

  for (int i = 0; i < iterations; ++i) {
    double v1 = 0, v2 = 0;
    for (const auto &nodeProb : rootNodes) {
      aoh1.emplace_back(-1, nodeProb.first->getLastObservationOfPlayer(0));
      aoh2.emplace_back(-1, nodeProb.first->getLastObservationOfPlayer(1));
      v1 += nodeProb.second * iteration(nodeProb.first, 1, nodeProb.second, 0, iteration);
      aoh1.pop_back();
      aoh2.pop_back();
    }
    firstIteration = false;
    for (const auto &nodeProb : rootNodes) {
      aoh1.emplace_back(-1, nodeProb.first->getLastObservationOfPlayer(0));
      aoh2.emplace_back(-1, nodeProb.first->getLastObservationOfPlayer(1));
      v2 += nodeProb.second * iteration(nodeProb.first, nodeProb.second, 1, 1, iteration);
      aoh1.pop_back();
      aoh2.pop_back();
    }
    cout << v1 << " " << v2 << "\n";
  }
  return regrets;
}
}  // namespace algorithms
}  // namespace GTLib2
