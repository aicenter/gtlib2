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


#include <limits>
#include <algorithm>
#include "algorithms/bestResponse.h"
#include "base/efg.h"
#include "algorithms/common.h"
#include "algorithms/tree.h"

namespace GTLib2::algorithms {

pair<BehavioralStrategy, double> bestResponseTo(const BehavioralStrategy &opoStrat,
                                                Player opponent, Player player,
                                                const Domain &domain,
                                                int maxDepth) {
    unordered_map<shared_ptr<EFGNode>, pair<BehavioralStrategy, double>> cache;
    unordered_map<shared_ptr<InformationSet>, EFGNodesDistribution> nodesInSameInfSet;
    int nodes = 0;

    function<pair<BehavioralStrategy, double>(shared_ptr<EFGNode>, int, double)> bestResp =
        [&player, &opponent, &domain, &nodes, &bestResp, &nodesInSameInfSet, &opoStrat, &cache]
            (shared_ptr<EFGNode> node, int depth, double prob) {

            ++nodes;
            if (cache.find(node) != cache.end()) {
                return cache.at(node);
            }
            if (prob == 0) {
                return pair<BehavioralStrategy, double>(BehavioralStrategy(), 0.0);
            }
            if ((!node->getCurrentPlayer() || depth <= 0)) {
                return pair<BehavioralStrategy, double>(BehavioralStrategy(),
                                                        node->rewards_[player] * prob);
            }

            auto infoSet = node->getAOHInfSet();
            // todo: not sure why, but putting directly
            //    if(*node->getCurrentPlayer() == player)
            //    does not work :/ why?
            Player curPlayer = *node->getCurrentPlayer();
            if (curPlayer == player) {
                // Player's node
                EFGNodesDistribution allNodesInTheSameInfSet;
                if (nodesInSameInfSet.find(infoSet) != nodesInSameInfSet.end()) {
                    allNodesInTheSameInfSet = nodesInSameInfSet[infoSet];
                } else {
                    allNodesInTheSameInfSet =
                        getAllNodesInTheInformationSetWithNatureProbability(infoSet, domain);
                }
                shared_ptr<Action> bestAction;
                double bestActionExpectedVal = -std::numeric_limits<double>::infinity();
                BehavioralStrategy brs;
                unordered_map<shared_ptr<Action>, unordered_map<shared_ptr<EFGNode>, double>>
                    actionNodeVal;
                for (const auto &action : node->availableActions()) {
                    unordered_map<shared_ptr<EFGNode>, double> siblingsVal;
                    double actionExpectedValue = 0.0;
                    for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                        double natureProb = siblingNatureProb.second;
                        const auto &sibling = siblingNatureProb.first;
                        double seqProb =
                            sibling->getProbabilityOfActionsSeqOfPlayer(opponent, opoStrat);
                        double val = 0;
                        for (auto siblingProb : sibling->performAction(action)) {
                            auto brs_val = bestResp(siblingProb.first,
                                                    siblingProb.first->getDepth()
                                                        == sibling->getDepth() ?
                                                    depth : depth - 1,
                                                    natureProb * seqProb * siblingProb.second);
                            val += brs_val.second;
                            brs.insert(brs_val.first.begin(), brs_val.first.end());
                        }
                        actionExpectedValue += val;

                        if (siblingsVal.find(sibling) != siblingsVal.end()) {
                            cout << "error" << endl;
                        }
                        siblingsVal[sibling] = val;
                    }

                    actionNodeVal[action] = siblingsVal;
                    if (actionExpectedValue > bestActionExpectedVal) {
                        bestActionExpectedVal = actionExpectedValue;
                        bestAction = action;
                    }
                }
                brs[infoSet] = {{bestAction, 1.0}};

                for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                    const auto &sibling = siblingNatureProb.first;
                    auto bestActionVal = actionNodeVal.at(bestAction).at(sibling);
                    cache[sibling] = pair<BehavioralStrategy, double>(brs, bestActionVal);
                }
                return cache.at(node);
            } else {
                // Opponent's node
                double val = 0;
                BehavioralStrategy brs;
                auto &stratAtTheNode = opoStrat.at(infoSet);
                for (const auto &action : node->availableActions()) {
                    if (stratAtTheNode.find(action) != stratAtTheNode.end()) {
                        double actionProb = stratAtTheNode.at(action);
                        for (auto childProb : node->performAction(action)) {
                            auto brs_val = bestResp(childProb.first,
                                                    childProb.first->getDepth() == node->getDepth()
                                                    ?
                                                    depth : depth - 1,
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
        if (node->getCurrentPlayer()) {
            auto infSet = node->getAOHInfSet();
            if (nodesInSameInfSet.find(infSet) == nodesInSameInfSet.end()) {
                nodesInSameInfSet.emplace(infSet, EFGNodesDistribution());
            }
            nodesInSameInfSet[infSet].emplace_back(node, node->natureProbability_);
        }
    };
    treeWalkEFG(domain, getAllNodesInInfSet, maxDepth);

    auto initNodesProb = createRootEFGNodes(
        domain.getRootStatesDistribution());
    BehavioralStrategy brs;
    double expVal = 0.0;
    for (const auto &nodeProb : initNodesProb) {
        auto node = nodeProb.first;
        auto prob = nodeProb.second;
        auto bestStratVal = bestResp(node, maxDepth, prob);
        expVal += bestStratVal.second;
        brs.insert(bestStratVal.first.begin(), bestStratVal.first.end());
    }
    return pair<BehavioralStrategy, double>(brs, expVal);
}


pair<BehavioralStrategy, double> bestResponseToPrunning(const BehavioralStrategy &opoStrat,
                                                        Player opponent, Player player,
                                                        const Domain &domain, int maxDepth) {
    unordered_map<shared_ptr<EFGNode>, pair<BehavioralStrategy, double>> cache;
    unordered_map<shared_ptr<InformationSet>, EFGNodesDistribution> nodesInInfSet;
    int nodes = 0;
    function<pair<BehavioralStrategy, double>(shared_ptr<EFGNode>, int, double, double)>
        bestResp =
        [&player, &opponent, &domain, &bestResp, &nodesInInfSet, &nodes, &opoStrat, &cache]
            (shared_ptr<EFGNode> node, int depth, double prob, double lowerBound) {
            ++nodes;
            if (cache.find(node) != cache.end()) {
                return cache.at(node);
            }
            if (prob == 0) {
                return pair<BehavioralStrategy, double>(BehavioralStrategy(), 0.0);
            }

            if ((!node->getCurrentPlayer() || depth <= 0)) {
                return pair<BehavioralStrategy, double>(BehavioralStrategy(),
                                                        node->rewards_[player] * prob);
            }
            auto infoSet = node->getAOHInfSet();
            if (*node->getCurrentPlayer() == player) {
                // Player's node
                EFGNodesDistribution allNodesInTheSameInfSet;
                if (nodesInInfSet.find(infoSet) != nodesInInfSet.end()) {
                    allNodesInTheSameInfSet = nodesInInfSet[infoSet];
                } else {
                    allNodesInTheSameInfSet =
                        getAllNodesInTheInformationSetWithNatureProbability(infoSet, domain);
                }

                std::sort(allNodesInTheSameInfSet.begin(), allNodesInTheSameInfSet.end(),
                          [](const pair<shared_ptr<EFGNode>, double> &a,
                             const pair<shared_ptr<EFGNode>, double> &b) {
                              return (a.second > b.second);
                          });

                shared_ptr<Action> bestAction;
                double bestActionExpectedVal = -std::numeric_limits<double>::infinity();
                BehavioralStrategy brs;
                double nodesProb = 0.0;
                for (auto &i : allNodesInTheSameInfSet) {
                    nodesProb += i.second;
                }
                unordered_map<shared_ptr<Action>,
                              unordered_map<shared_ptr<EFGNode>, double>> actionNodeVal;
                for (const auto &action : node->availableActions()) {
                    unordered_map<shared_ptr<EFGNode>, double> siblingsVal;
                    double actionExpectedValue = 0.0;
                    double remainingNodesProb = nodesProb;
                    for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                        double natureProb = siblingNatureProb.second;
                        const auto &sibling = siblingNatureProb.first;
                        if (max(lowerBound, bestActionExpectedVal) > remainingNodesProb *
                            domain.getMaxUtility() + actionExpectedValue) {  // TODO: not prunning
                            siblingsVal[sibling] = 0;
                            break;
                        }
                        double newLowerBound = bestActionExpectedVal - actionExpectedValue -
                            (remainingNodesProb - natureProb) * domain.getMaxUtility();
                        double seqProb =
                            sibling->getProbabilityOfActionsSeqOfPlayer(opponent, opoStrat);
                        if (newLowerBound > seqProb * natureProb * domain.getMaxUtility()) {
                            break;
                        }
                        double val = 0;
                        for (auto siblingProb : sibling->performAction(action)) {
                            auto brs_val = bestResp(siblingProb.first,
                                                    siblingProb.first->getDepth() ==
                                                        sibling->getDepth() ? depth : depth - 1,
                                                    natureProb * seqProb * siblingProb.second,
                                                    newLowerBound);
                            val += brs_val.second;
                            brs.insert(brs_val.first.begin(), brs_val.first.end());
                        }
                        remainingNodesProb -= natureProb;
                        actionExpectedValue += val;

                        if (siblingsVal.find(sibling) != siblingsVal.end()) {
                            cout << "error" << endl;
                        }
                        siblingsVal[sibling] = val;
                    }

                    actionNodeVal[action] = siblingsVal;
                    if (actionExpectedValue > bestActionExpectedVal) {
                        bestActionExpectedVal = actionExpectedValue;
                        bestAction = action;
                    }
                }
                brs[infoSet] = {{bestAction, 1.0}};

                for (const auto &siblingNatureProb : allNodesInTheSameInfSet) {
                    const auto &sibling = siblingNatureProb.first;
                    auto bestActionVal = actionNodeVal.at(bestAction).at(sibling);
                    cache[sibling] = pair<BehavioralStrategy, double>(brs, bestActionVal);
                }
                return cache.at(node);

            } else {
                // Opponent's node
                double val = 0;
                BehavioralStrategy brs;
                auto &stratAtTheNode = opoStrat.at(infoSet);
                for (const auto &action : node->availableActions()) {
                    if (stratAtTheNode.find(action) != stratAtTheNode.end()) {
                        double actionProb = stratAtTheNode.at(action);
                        for (auto childProb : node->performAction(action)) {
                            auto brs_val = bestResp(childProb.first,
                                                    childProb.first->getDepth() ==
                                                        node->getDepth() ? depth : depth - 1,
                                                    prob * childProb.second * actionProb,
                                                    lowerBound);
                            val += brs_val.second;
                            brs.insert(brs_val.first.begin(), brs_val.first.end());
                        }
                    }
                }
                return pair<BehavioralStrategy, double>(brs, val);
            }
        };

    auto getAllNodesInInfSet = [&nodesInInfSet, &domain](shared_ptr<EFGNode> node) {
        if (node->getCurrentPlayer()) {
            auto infSet = node->getAOHInfSet();
            if (nodesInInfSet.find(infSet) == nodesInInfSet.end()) {
                nodesInInfSet.emplace(infSet, EFGNodesDistribution());
            }
            nodesInInfSet[infSet].emplace_back(node, node->natureProbability_);
        }
    };
    treeWalkEFG(domain, getAllNodesInInfSet, maxDepth);

    auto initNodesProb = createRootEFGNodes(
        domain.getRootStatesDistribution());
    BehavioralStrategy brs;
    double expVal = 0.0;
    for (const auto &nodeProb : initNodesProb) {
        auto node = nodeProb.first;
        auto prob = nodeProb.second;
        auto bestStratVal = bestResp(node, maxDepth, prob, -domain.getMaxUtility());
        expVal += bestStratVal.second;
        brs.insert(bestStratVal.first.begin(), bestStratVal.first.end());
    }
    return pair<BehavioralStrategy, double>(brs, expVal);
}
}  // namespace GTLib2
