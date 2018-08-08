//
// Created by Jakub Rozlivek on 8/8/18.
//

#include "cfr.h"
#include "treeWalk.h"
#include "common.h"
#include "utility.h"


namespace GTLib2 {
  namespace algorithms {
    BehavioralStrategy getStrategyFor(const Domain &domain, int player,
                                      const unordered_map<shared_ptr<InformationSet>,
                                              std::pair<vector<double>, vector<double>>> &allMP) {
      auto strategy = BehavioralStrategy();
      auto processed = unordered_set<shared_ptr<InformationSet>>();
      processed.reserve(allMP.size());
      strategy.reserve(allMP.size());
      auto getStrategy = [&strategy, &processed, &player, &allMP](shared_ptr<EFGNode> node) {
        if (node->getCurrentPlayer() && *node->getCurrentPlayer() == player) {
          auto infSet = node->getAOHInfSet();
          if (processed.find(infSet) == processed.end()) {
            auto actionDistribution = unordered_map<shared_ptr<Action>, double>();
            const auto mp = allMP.at(infSet).second;
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
      algorithms::treeWalkEFG(domain, getStrategy, domain.getMaxDepth());
      return move(strategy);
    }

    pair<double,double>
    CFR(const Domain &domain, int iterations) { // TODO: second function with time
      auto regrets = unordered_map<shared_ptr<InformationSet>, pair<vector<double>, vector<double>>>();
      int nbSamples = 0;
      auto used = unordered_set<shared_ptr<InformationSet>>();
      bool firstIteration = true;
      auto aoh1 = vector<int>();
      auto aoh2 = vector<int>();

      const auto iteration = [&regrets, &nbSamples, &aoh1, &aoh2, &firstIteration](
              shared_ptr<EFGNode> node, double pi1, double pi2, int player, const auto &iteration) {
        if (pi1 == 0 && pi2 == 0) {
          return 0.0;
        }
        auto actions = node->availableActions();
        if (actions.empty()) {
          return node->rewards[player];
        }
        const int currentplayer = *node->getCurrentPlayer();
        const auto K = static_cast<const unsigned int>(actions.size());
        vector<pair<int, int>> aohistory{};
        if (currentplayer == 0) {
          for (int i = 0; i < aoh1.size(); i += 2) {
            aohistory.emplace_back(aoh1[i], aoh1[i + 1]);
          }
        } else {
          for (int i = 0; i < aoh2.size(); i += 2) {
            aohistory.emplace_back(aoh2[i], aoh2[i + 1]);
          }
        }
        auto is = make_shared<AOH>(currentplayer, node->initialObservations[currentplayer]->getId(),
                                   aohistory);
        if (firstIteration && regrets.find(is) == regrets.end()) {
          regrets[is] = std::make_pair(vector<double>(K), vector<double>(K));
        }
        auto[r, mp] = regrets.at(is);

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
          auto newNodes = node->performAction(action);
          if (currentplayer == 0) {
            aoh1.push_back(action->getId());
            aoh2.push_back(-1);
          } else {
            aoh1.push_back(-1);
            aoh2.push_back(action->getId());
          }
          for (const auto &newNode : newNodes) {
            double new_p1 = player == 1 ? pi1 * newNode.second : pi1;
            double new_p2 = player == 0 ? pi2 * newNode.second : pi2;
            aoh1.push_back(newNode.first->observations[0]->getId());
            aoh2.push_back(newNode.first->observations[1]->getId());
            if (currentplayer == 0) {
              tmpV[i] += newNode.second *
                         iteration(newNode.first, new_p1 * rmProbs[i], new_p2, player, iteration);
            } else {
              tmpV[i] += newNode.second *
                         iteration(newNode.first, new_p1, rmProbs[i] * new_p2, player, iteration);
            }
            aoh1.pop_back();
            aoh2.pop_back();
          }
          ev += rmProbs[i] * tmpV[i];
          aoh1.pop_back();
          aoh2.pop_back();
        }

        if (currentplayer == player) {
          for (int j = 0; j != K; j++) {
            r[j] += (player == 0 ? pi2 : pi1) * (tmpV[j] - ev);
          }
          for (int j = 0; j != K; j++) {
            mp[j] += (player == 0 ? pi1 : pi2) * rmProbs[j];
          }
          regrets[is] = std::make_pair(r, mp);
          nbSamples++;
        }

        return ev;
      };
      auto rootNodes = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
              domain.getRootStatesDistribution());

      for (int i = 0; i < iterations; ++i) {
        double v1 = 0, v2 = 0;
        for (const auto &nodeProb : rootNodes) {
          v1 += nodeProb.second * iteration(nodeProb.first, 1, nodeProb.second, 0, iteration);
        }
        for (const auto &nodeProb : rootNodes) {
          v2 += nodeProb.second * iteration(nodeProb.first, nodeProb.second, 1, 1, iteration);
        }
        firstIteration = false;
        cout << v1 << " " << v2 << "\n";
      }
      auto strat1 = getStrategyFor(domain, 0, regrets);
      auto strat2 = getStrategyFor(domain, 1, regrets);
      return algorithms::computeUtilityTwoPlayersGame(domain, strat1, strat2, 0, 1);
    }
  }
}