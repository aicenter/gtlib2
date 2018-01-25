//
// Created by Pavel Rytir on 1/21/18.
//

#include "bestResponse.h"
#include "../base/efg.h"
#include "common.h"


namespace GTLib2 {

    pair<BehavioralStrategy, double> algorithms::bestResponseTo(const BehavioralStrategy &opoStrat, const int opponent,
                                                                const int player, const Domain &domain,
                                                                const int maxDepth) {

        unordered_map<shared_ptr<EFGNode>,pair<BehavioralStrategy, double>> cache;


        std::function<pair<BehavioralStrategy, double>(shared_ptr<EFGNode>,
                                                       int, double)> bestResp =
                [&player, &opponent, &domain, &bestResp, &opoStrat, &cache](shared_ptr<EFGNode> node, int depth,
                                                                    double prob) {
                    if (cache.find(node)!=cache.end()) {
                        return cache.at(node);
                    }

                    if (!node->getCurrentPlayer() || depth <= 0) {
                        auto expectedReward = node->rewards[player] * prob;
                        return pair<BehavioralStrategy, double>(BehavioralStrategy(), expectedReward);
                    };

                    if (*node->getCurrentPlayer() == player) {
                        //Player's node

                        auto allNodesInTheSameInfSet = gelAllNodesInTheInformationSetWithNatureProbability(
                                node->pavelgetAOHInfSet(), domain);


                        shared_ptr<Action> bestAction;
                        double bestActionVal = -std::numeric_limits<double>::infinity();
                        //BehavioralStrategy bestStratFromBestNode;
                        BehavioralStrategy brs;

                        for (const auto &action : node->availableActions()) {

                            double actionExpectedValue = 0.0;


                            for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {

                                double natureProb = std::get<1>(siblingNatureProb);
                                const auto &sibling = std::get<0>(siblingNatureProb);
                                double seqProb = sibling->getProbabilityOfSeqOfPlayer(opponent,opoStrat);
                                double val = 0;
                                for (auto siblingProb : sibling->pavelPerformAction(action)) {

                                    auto brs_val = bestResp(siblingProb.first, depth - 1,
                                                            natureProb * seqProb * siblingProb.second);
                                    val += std::get<1>(brs_val);
                                    auto bestStrat = std::get<0>(brs_val);
                                    brs.insert(bestStrat.begin(), bestStrat.end());
                                }
                                actionExpectedValue += val;

                            }


                            if (actionExpectedValue > bestActionVal) {
                                bestActionVal = actionExpectedValue;
                                bestAction = action;
                                //bestStratFromBestNode = brs;
                            }
                        }

                        brs[node->pavelgetAOHInfSet()] = {{bestAction, 1.0}};
                        return pair<BehavioralStrategy, double>(brs, bestActionVal);
                    } else {
                        // Opponent's node
                        double val = 0;
                        BehavioralStrategy brs;
                        auto stratAtTheNode = opoStrat.at(node->pavelgetAOHInfSet());
                        for (auto action : node->availableActions()) {
                            if (stratAtTheNode.find(action) != stratAtTheNode.end()) {
                                double actionProb = stratAtTheNode.at(action);
                                for (auto childProb : node->pavelPerformAction(action)) {
                                    auto brs_val = bestResp(childProb.first, depth - 1,
                                                            prob * childProb.second * actionProb);
                                    val += std::get<1>(brs_val);
                                    auto bestStrat = std::get<0>(brs_val);
                                    brs.insert(bestStrat.begin(), bestStrat.end());
                                }
                            }
                        }
                        return pair<BehavioralStrategy, double>(brs, val);
                    }
                };

        auto initNodesProb = algorithms::createEFGNodesFromDomainInitDistr(*domain.getRootStateDistributionPtr());
        BehavioralStrategy brs;
        double expVal = 0.0;
        for (auto nodeProb : initNodesProb) {
            auto node = std::get<0>(nodeProb);
            auto prob = std::get<1>(nodeProb);
            auto bestStratVal = bestResp(node, maxDepth, prob);
            expVal += std::get<1>(bestStratVal);
            auto bestStrat = std::get<0>(bestStratVal);
            brs.insert(bestStrat.begin(), bestStrat.end());
        }
        return pair<BehavioralStrategy, double>(brs, expVal);
    }
}