//
// Created by Pavel Rytir on 1/21/18.
//

#include "bestResponse.h"
#include "../base/efg.h"
#include "common.h"


namespace GTLib2 {

    pair<BehavioralStrategy, double>
    algorithms::bestResponseTo(const BehavioralStrategy &opoStrat, int opponent, int player, const Domain &domain) {
        return algorithms::bestResponseTo(opoStrat,opponent,player,domain, domain.getMaxDepth());
    }

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
                        double reward = node->rewards[player];
                        auto expectedReward = reward * prob;

                        return pair<BehavioralStrategy, double>(BehavioralStrategy(), expectedReward);
                    };

                    if (*node->getCurrentPlayer() == player) {
                        //Player's node

                        auto allNodesInTheSameInfSet = gelAllNodesInTheInformationSetWithNatureProbability(
                                node->getAOHInfSet(), domain);


                        shared_ptr<Action> bestAction;
                        double bestActionExpectedVal = -std::numeric_limits<double>::infinity();
                        //BehavioralStrategy bestStratFromBestNode;
                        BehavioralStrategy brs;

                        unordered_map<shared_ptr<Action>, unordered_map<shared_ptr<EFGNode>,double>> actionNodeVal;

                        for (const auto &action : node->availableActions()) {
                            actionNodeVal[action] = unordered_map<shared_ptr<EFGNode>,double>();

                            double actionExpectedValue = 0.0;


                            for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {

                                double natureProb = siblingNatureProb.second;
                                const auto &sibling = siblingNatureProb.first;
                                double seqProb = sibling->getProbabilityOfActionsSeqOfPlayer(opponent,opoStrat);
                                double val = 0;
                                for (auto siblingProb : sibling->performAction(action)) {

                                    auto brs_val = bestResp(siblingProb.first, depth - 1,
                                                            natureProb * seqProb * siblingProb.second);
                                    val += brs_val.second;
                                    auto bestStrat = brs_val.first;
                                    brs.insert(bestStrat.begin(), bestStrat.end());
                                }
                                actionExpectedValue += val;

                                actionNodeVal[action][sibling] = val;

                            }


                            if (actionExpectedValue > bestActionExpectedVal) {
                                bestActionExpectedVal = actionExpectedValue;
                                bestAction = action;
                                //bestStratFromBestNode = brs;
                            }
                        }

                        brs[node->getAOHInfSet()] = {{bestAction, 1.0}};

                        for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                            const auto &sibling = siblingNatureProb.first;
                            auto bestActionVal = actionNodeVal[bestAction][sibling];
                            cache[sibling] = pair<BehavioralStrategy, double>(brs, bestActionVal);
                        }




                        //cache[node] = pair<BehavioralStrategy, double>(brs, bestActionVal);
                        //return pair<BehavioralStrategy, double>(brs, bestActionVal);

                        return cache.at(node);
                    } else {
                        // Opponent's node
                        double val = 0;
                        BehavioralStrategy brs;
                        auto stratAtTheNode = opoStrat.at(node->getAOHInfSet());
                        for (const auto &action : node->availableActions()) {
                            if (stratAtTheNode.find(action) != stratAtTheNode.end()) {
                                double actionProb = stratAtTheNode.at(action);
                                for (auto childProb : node->performAction(action)) {
                                    auto brs_val = bestResp(childProb.first, depth - 1,
                                                            prob * childProb.second * actionProb);
                                    val += brs_val.second;
                                    auto bestStrat = brs_val.first;
                                    brs.insert(bestStrat.begin(), bestStrat.end());
                                }
                            }
                        }
                        return pair<BehavioralStrategy, double>(brs, val);
                    }
                };

        auto initNodesProb = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
                domain.getRootStatesDistribution());
        BehavioralStrategy brs;
        double expVal = 0.0;
        for (auto nodeProb : initNodesProb) {
            auto node = nodeProb.first;
            auto prob = nodeProb.second;
            auto bestStratVal = bestResp(node, maxDepth, prob);
            expVal += bestStratVal.second;
            auto bestStrat = bestStratVal.first;
            brs.insert(bestStrat.begin(), bestStrat.end());
        }
        return pair<BehavioralStrategy, double>(brs, expVal);
    }


}