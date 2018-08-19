//
// Created by Pavel Rytir on 1/21/18.
//

#include "bestResponse.h"
#include "../base/efg.h"
#include "common.h"
#include "treeWalk.h"


namespace GTLib2 {

    pair<BehavioralStrategy, double>
    algorithms::bestResponseTo(const BehavioralStrategy &opoStrat, int opponent, int player, const Domain &domain) {
        return algorithms::bestResponseTo(opoStrat,opponent,player,domain, domain.getMaxDepth());
    }

    pair<BehavioralStrategy, double> algorithms::bestResponseTo(const BehavioralStrategy &opoStrat, const int opponent, //TODO: Not working
                                                                const int player, const Domain &domain,
                                                                const int maxDepth) { // TODO: bad results

        unordered_map<shared_ptr<EFGNode>,pair<BehavioralStrategy, double>> cache;
        unordered_map<shared_ptr<InformationSet>, EFGNodesDistribution> nodesInSameInfSet;

        std::function<pair<BehavioralStrategy, double>(shared_ptr<EFGNode>, int, double)> bestResp =
                [&player, &opponent, &domain, &bestResp, &nodesInSameInfSet, &opoStrat, &cache]
                (shared_ptr<EFGNode> node, int depth, double prob) {

                    if (cache.find(node)!=cache.end()) {
                        return cache.at(node);
                    }
                    if (prob == 0) {
                        return pair<BehavioralStrategy, double>(BehavioralStrategy(), 0.0);
                    }
                    if ((!node->getCurrentPlayer() || depth <= 0)) {
                        double reward = node->rewards[player];
                        auto expectedReward = reward * prob;
                        return pair<BehavioralStrategy, double>(BehavioralStrategy(), expectedReward);
                    }
                    auto infSet = node->getAOHInfSet();
                    if (*node->getCurrentPlayer() == player) {
                        // Player's node
                        EFGNodesDistribution allNodesInTheSameInfSet;
                        if(nodesInSameInfSet.find(infSet) != nodesInSameInfSet.end()) {
                            allNodesInTheSameInfSet = nodesInSameInfSet[infSet];
                        } else {
                            allNodesInTheSameInfSet = getAllNodesInTheInformationSetWithNatureProbability(
                                    infSet, domain); // TODO: does not work
                        }
                        shared_ptr<Action> bestAction;
                        double bestActionExpectedVal = -std::numeric_limits<double>::infinity();
                        BehavioralStrategy brs;

                        unordered_map<shared_ptr<Action>, unordered_map<shared_ptr<EFGNode>,double>> actionNodeVal;
                        for (const auto &action : node->availableActions()) {
                            unordered_map<shared_ptr<EFGNode>,double> siblingsVal;
                            double actionExpectedValue = 0.0;
                            for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                                double natureProb = siblingNatureProb.second;
                                const auto &sibling = siblingNatureProb.first;
//                                double seqProb = sibling->getProbabilityOfActionsSeqOfPlayer(opponent,opoStrat);
                                double val = 0;
                                for (auto siblingProb : sibling->performAction(action)) {

                                    auto brs_val = bestResp(siblingProb.first,
                                            siblingProb.first->getState()== sibling->getState()?
                                            depth : depth-1,
                                            natureProb /** seqProb*/ * siblingProb.second);
                                    val += brs_val.second;
                                    brs.insert(brs_val.first.begin(), brs_val.first.end());
                                }
                                actionExpectedValue += val;

                                if (siblingsVal.find(sibling) != siblingsVal.end()) {
                                    cout << "error" <<std::endl;
                                }
                                siblingsVal[sibling] = val;
                            }

                            actionNodeVal[action] = siblingsVal;
                            if (actionExpectedValue > bestActionExpectedVal) {
                                bestActionExpectedVal = actionExpectedValue;
                                bestAction = action;
                            }
                        }
                        brs[infSet] = {{bestAction, 1.0}};

                        for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                            const auto &sibling = siblingNatureProb.first;
                            auto bestActionVal = actionNodeVal.at(bestAction).at(sibling);
                            cache[sibling] = pair<BehavioralStrategy, double>(brs, bestActionVal);
                            if(*node.get() == *sibling.get()) {
                                return cache.at(sibling);
                            }
                        }
                        auto bstVal = cache.at(node);

                        return bstVal;
                    } else {
                        // Opponent's node
                        double val = 0;
                        BehavioralStrategy brs;
                        auto stratAtTheNode = opoStrat.at(infSet);
                        for (const auto &action : node->availableActions()) {
                            if (stratAtTheNode.find(action) != stratAtTheNode.end()) {
                                double actionProb = stratAtTheNode.at(action);
                                for (auto childProb : node->performAction(action)) {
                                    auto brs_val = bestResp(childProb.first,
                                            childProb.first->getState()== node->getState()?
                                            depth : depth-1,
                                            prob * childProb.second * actionProb);
                                    val += brs_val.second;
                                    brs.insert(brs_val.first.begin(), brs_val.first.end());
                                }
                            }
                        }
                        return pair<BehavioralStrategy, double>(brs, val);
                    }
                };

        auto getAllNodesInInfSet = [&nodesInSameInfSet, &domain](shared_ptr<EFGNode> node) {
          if(node->getCurrentPlayer()) {
            auto infSet = node->getAOHInfSet();
            if(nodesInSameInfSet.find(infSet) == nodesInSameInfSet.end()) {
              nodesInSameInfSet.emplace(infSet, EFGNodesDistribution());
            }
          nodesInSameInfSet[infSet].emplace_back(node, node->natureProbability);
          }
        };
        algorithms::treeWalkEFG(domain, getAllNodesInInfSet, domain.getMaxDepth());

//        cout << nodesInSameInfSet.size() <<"pocet\n";
        auto initNodesProb = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
                domain.getRootStatesDistribution());
        BehavioralStrategy brs;
        double expVal = 0.0;
        for (auto nodeProb : initNodesProb) {
            auto node = nodeProb.first;
            auto prob = nodeProb.second;
            auto bestStratVal = bestResp(node, maxDepth, prob);
            expVal += bestStratVal.second;
            brs.insert(bestStratVal.first.begin(), bestStratVal.first.end());
        }
        return pair<BehavioralStrategy, double>(brs, expVal);
    }

  pair<BehavioralStrategy, double>
  algorithms::bestResponseToPrunning(const BehavioralStrategy &opoStrat, int opponent, int player, const Domain &domain) {
      return algorithms::bestResponseToPrunning(opoStrat,opponent,player,domain, domain.getMaxDepth());
  }

  pair<BehavioralStrategy, double> algorithms::bestResponseToPrunning(const BehavioralStrategy &opoStrat, const int opponent,
                                                              const int player, const Domain &domain,
                                                              const int maxDepth) { //TODO: Not working

      unordered_map<shared_ptr<EFGNode>,pair<BehavioralStrategy, double>> cache;
      unordered_map<shared_ptr<InformationSet>, EFGNodesDistribution> nodesInSameInfSet;

      std::function<pair<BehavioralStrategy, double>(shared_ptr<EFGNode>, int, double)> bestResp =
              [&player, &opponent, &domain, &bestResp, &nodesInSameInfSet, &opoStrat, &cache]
                      (shared_ptr<EFGNode> node, int depth, double prob) {

                if (cache.find(node)!=cache.end()) {
                    return cache.at(node);
                }
                if (prob == 0) {
                    return pair<BehavioralStrategy, double>(BehavioralStrategy(), 0.0);
                }
                if ((!node->getCurrentPlayer() || depth <= 0)) {
                    double reward = node->rewards[player];
                    auto expectedReward = reward * prob;
                    return pair<BehavioralStrategy, double>(BehavioralStrategy(), expectedReward);
                }
                auto infSet = node->getAOHInfSet();
                if (*node->getCurrentPlayer() == player) {
                    // Player's node
                    EFGNodesDistribution allNodesInTheSameInfSet;
                    if(nodesInSameInfSet.find(infSet) != nodesInSameInfSet.end()) {
                        allNodesInTheSameInfSet = nodesInSameInfSet[infSet];
                    } else {
                        allNodesInTheSameInfSet =
                                getAllNodesInTheInformationSetWithNatureProbability(infSet, domain);
                    }
                    std::sort(allNodesInTheSameInfSet.begin(), allNodesInTheSameInfSet.end(),
                            [](const pair<shared_ptr<EFGNode>,double> &a,
                                    const pair<shared_ptr<EFGNode>,double> &b)
                                    { return (a.second > b.second); });

                    shared_ptr<Action> bestAction;
                    double bestActionExpectedVal = -std::numeric_limits<double>::infinity();
                    BehavioralStrategy brs;
                    double nodesProb = 0.0;
                    for(auto &i : allNodesInTheSameInfSet) {
                        nodesProb +=i.second;
                    }
                    unordered_map<shared_ptr<Action>, unordered_map<shared_ptr<EFGNode>,double>> actionNodeVal;
                    for (const auto &action : node->availableActions()) {
                        unordered_map<shared_ptr<EFGNode>,double> siblingsVal;
                        double actionExpectedValue = 0.0;
                        double remainingNodesProb = nodesProb;
                        for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                            double natureProb = siblingNatureProb.second;
                            const auto &sibling = siblingNatureProb.first;
                            if(remainingNodesProb * domain.getMaxUtility() + actionExpectedValue < bestActionExpectedVal) {
                                break;
                            }
                            double seqProb = sibling->getProbabilityOfActionsSeqOfPlayer(opponent,opoStrat);
                            double val = 0;
                            for (auto siblingProb : sibling->performAction(action)) {
                                auto brs_val = bestResp(siblingProb.first,
                                                        siblingProb.first->getState()== sibling->getState()?
                                                        depth : depth-1,
                                                        natureProb * seqProb * siblingProb.second);
                                val += brs_val.second;
                                brs.insert(brs_val.first.begin(), brs_val.first.end());
                            }
                            remainingNodesProb -= natureProb;
                            actionExpectedValue += val;

                            if (siblingsVal.find(sibling) != siblingsVal.end()) {
                                cout << "error" <<std::endl;
                            }
                            siblingsVal[sibling] = val;
                        }

                        actionNodeVal[action] = siblingsVal;
                        if (actionExpectedValue > bestActionExpectedVal) {
                            bestActionExpectedVal = actionExpectedValue;
                            bestAction = action;
                        }
                    }
                    brs[infSet] = {{bestAction, 1.0}};

                    for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                        const auto &sibling = siblingNatureProb.first;

                        auto bestActionVal = actionNodeVal.at(bestAction).at(sibling);
                        cache[sibling] = pair<BehavioralStrategy, double>(brs, bestActionVal);
                        if(*node.get() == *sibling.get()) {
                            return cache.at(sibling);
                        }
                    }
                    return cache.at(node);

                } else {
                    // Opponent's node
                    double val = 0;
                    BehavioralStrategy brs;
                    auto stratAtTheNode = opoStrat.at(infSet);
                    for (const auto &action : node->availableActions()) {
                        if (stratAtTheNode.find(action) != stratAtTheNode.end()) {
                            double actionProb = stratAtTheNode.at(action);
                            for (auto childProb : node->performAction(action)) {
                                auto brs_val = bestResp(childProb.first,
                                                        childProb.first->getState()== node->getState()?
                                                        depth : depth-1,
                                                        prob * childProb.second * actionProb);
                                val += brs_val.second;
                                brs.insert(brs_val.first.begin(), brs_val.first.end());
                            }
                        }
                    }
                    return pair<BehavioralStrategy, double>(brs, val);
                }
              };

      auto getAllNodesInInfSet = [&nodesInSameInfSet, &domain](shared_ptr<EFGNode> node) {
        if(node->getCurrentPlayer()) {
            auto infSet = node->getAOHInfSet();
            if(nodesInSameInfSet.find(infSet) == nodesInSameInfSet.end()) {
                nodesInSameInfSet.emplace(infSet, EFGNodesDistribution());
            }
            nodesInSameInfSet[infSet].emplace_back(node, node->natureProbability);
        }
      };
      algorithms::treeWalkEFG(domain, getAllNodesInInfSet, domain.getMaxDepth());

      auto initNodesProb = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
              domain.getRootStatesDistribution());
      BehavioralStrategy brs;
      double expVal = 0.0;
      for (auto nodeProb : initNodesProb) {
          auto node = nodeProb.first;
          auto prob = nodeProb.second;
          auto bestStratVal = bestResp(node, maxDepth, prob);
          expVal += bestStratVal.second;
          brs.insert(bestStratVal.first.begin(), bestStratVal.first.end());
      }
      return pair<BehavioralStrategy, double>(brs, expVal);
  }

}