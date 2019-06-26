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


#include "algorithms/bestResponse.h"

#include <algorithm>

namespace GTLib2::algorithms {


typedef unordered_map<shared_ptr<EFGNode>, StrategyValue> BestRespCache;
typedef unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> NodesInInfosets;

const StrategyValue &_bestResponse(const BehavioralStrategy &opoStrat,
                                   const Player responder,
                                   const Domain &domain,
                                   BestRespCache &cache,
                                   NodesInInfosets &allNodesInInfosets,
                                   const shared_ptr<EFGNode> &node,
                                   double chanceReachProb) {

//    if (reachProbChanceOpponent == 0.0) return StrategyValue();
    // We may have solved this infoset already (we traverse by histories, not infosets!)
    if (cache.find(node) != cache.end()) return cache.at(node);

    if (node->type_ == TerminalNode) {
        const double reachOpponent = node->getProbabilityOfActionSeq(opponent(responder), opoStrat);
        const double nodeValue = node->getUtilities()[responder]
            * reachOpponent * chanceReachProb;
        assert(nodeValue <= domain.getMaxUtility());
        assert(nodeValue >= domain.getMinUtility());
        auto[it, result] = cache.emplace(node, StrategyValue(BehavioralStrategy(), nodeValue));
//        cout << node->getHistory() << " ::  " << node->getUtilities() << " :: " << it->second.value << "\n";
        return it->second;
    }

    if (node->type_ == ChanceNode) {
        double nodeValue = 0.0;
        auto brs = BehavioralStrategy();
        for (const auto &action : node->availableActions()) {
            const auto response = _bestResponse(opoStrat, responder, domain, cache,
                                                allNodesInInfosets, node->performAction(action),
                                                chanceReachProb * node->chanceProbForAction(action));
            nodeValue += response.value;
            assert(nodeValue <= domain.getMaxUtility());
            assert(nodeValue >= domain.getMinUtility());
            brs.insert(response.strategy.begin(), response.strategy.end());
        }

        assert(nodeValue <= domain.getMaxUtility());
        assert(nodeValue >= domain.getMinUtility());
        auto[it, result] = cache.emplace(node, StrategyValue(brs, nodeValue));
        return it->second;
    }

    assert(node->type_ == PlayerNode);
    const auto infoset = node->getAOHInfSet();

    if (node->getPlayer() != responder) { // Opponent's node
        double nodeValue = 0.0;
        auto brs = BehavioralStrategy();
        const auto &stratAtTheNode = opoStrat.at(infoset);

        for (const auto &action : node->availableActions()) {
            const auto actionProb = getActionProb(stratAtTheNode, action);
            if (actionProb == 0.0) continue;

            const auto childNode = node->performAction(action);
            const auto &response = _bestResponse(opoStrat, responder, domain, cache,
                                                 allNodesInInfosets, childNode, chanceReachProb);
            nodeValue += response.value;
            assert(nodeValue <= domain.getMaxUtility());
            assert(nodeValue >= domain.getMinUtility());

            brs.insert(response.strategy.begin(), response.strategy.end());
        }
        assert(nodeValue <= domain.getMaxUtility());
        assert(nodeValue >= domain.getMinUtility());

        auto[it, result] = cache.emplace(node, StrategyValue(brs, nodeValue));
        return it->second;
    }

    // Player's node
    vector<shared_ptr<EFGNode>> nodesInInfoset;
    if (allNodesInInfosets.find(infoset) != allNodesInInfosets.end()) {
        nodesInInfoset = allNodesInInfosets[infoset];
    } else {
        nodesInInfoset = getAllNodesInInfoset(infoset, domain);
        allNodesInInfosets[infoset] = nodesInInfoset;
    }
    assert(!nodesInInfoset.empty());
    const auto actions = node->availableActions();
    const auto numActions = actions.size();
    const auto numNodes = nodesInInfoset.size();

    ActionId bestAction = 0;
    double bestActionExpectedVal = -std::numeric_limits<double>::infinity();
    BehavioralStrategy brs;

    // actions -> node -> value
    auto actionVals = vector<vector<double>>();
    actionVals.reserve(numActions);

    for (int i = 0; i < numActions; ++i) {
        assert(actions[i]->getId() == i);
        double actionValue = 0.0;

        vector<double> siblingsValue;
        siblingsValue.reserve(numNodes);
        for (int j = 0; j < numNodes; ++j) {
            const auto &siblingNode = nodesInInfoset[j];
            if (siblingNode->getProbabilityOfActionSeq(opponent(responder), opoStrat) == 0.0) {
                siblingsValue.push_back(0.0);
                continue;
            }

            const auto siblingChild = siblingNode->performAction(actions[i]);
            const auto &response = _bestResponse(
                opoStrat, responder, domain, cache, allNodesInInfosets, siblingChild, chanceReachProb);
            brs.insert(response.strategy.begin(), response.strategy.end());
            actionValue += response.value;
            siblingsValue.push_back(response.value);
        }

        actionVals.push_back(siblingsValue);
        if (actionValue > bestActionExpectedVal) {
            bestActionExpectedVal = actionValue;
            bestAction = i;
        }
    }
    brs[infoset] = {{actions[bestAction], 1.0}};

    for (int j = 0; j < numNodes; ++j) {
        const auto &siblingNode = nodesInInfoset[j];
        auto bestActionVal = actionVals[bestAction][j];

        assert(bestActionVal >= domain.getMinUtility());
        assert(bestActionVal <= domain.getMaxUtility());
        cache.emplace(siblingNode, StrategyValue(brs, bestActionVal));
    }

    const StrategyValue &returnValue = cache.at(node);
    assert(returnValue.value <= domain.getMaxUtility());
    assert(returnValue.value >= domain.getMinUtility());
    return returnValue;
}


const StrategyValue bestResponseTo(const BehavioralStrategy &opoStrat,
                                   const Player responder,
                                   const Domain &domain) {

    unordered_map<shared_ptr<EFGNode>, StrategyValue> cache;
    NodesInInfosets nodesInInfosets; // empty -- we will need to generate them on the fly

    const auto returnValue = _bestResponse(opoStrat, responder, domain,
                                           cache, nodesInInfosets, createRootEFGNode(domain), 1.0);
    assert(returnValue.value <= domain.getMaxUtility());
    assert(returnValue.value >= domain.getMinUtility());

    return returnValue;
}

// todo: use infoset cache
//StrategyValue bestResponseTo(const BehavioralStrategy &opoStrat, Player opponent, Player player,
//                             const InfosetCache &infosetCache) {
//
//    unordered_map<shared_ptr<EFGNode>, StrategyValue> cache;
//    auto nodesInInfosets = infosetCache.getInfoset2NodeMapping();
//
//    return _bestResponse(opoStrat, opponent, player, infosetCache.getDomainMaxStateDepth(), cache, infosetCache,
//                         nodesInInfosets, 1.0);
//}

// todo: refactor and add tests that use this method
/*
StrategyValue bestResponseToPrunning(const BehavioralStrategy &opoStrat,
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

            if ((!node->getPlayer() || depth <= 0)) {
                return pair<BehavioralStrategy, double>(BehavioralStrategy(),
                                                        node->getUtilities()[player] * prob);
            }
            auto infoSet = node->getAOHInfSet();
            if (node->getPlayer() == player) {
                // Player's node
                EFGNodesDistribution allNodesInTheSameInfSet;
                if (nodesInInfSet.find(infoSet) != nodesInInfSet.end()) {
                    allNodesInTheSameInfSet = nodesInInfSet[infoSet];
                } else {
                    allNodesInTheSameInfSet =
                        getAllNodesInInfoset(infoSet, domain);
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
                                                    siblingProb.first->stateDepth_ ==
                                                        sibling->stateDepth_ ? depth : depth - 1,
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
                                                    childProb.first->stateDepth_ ==
                                                        node->stateDepth_ ? depth : depth - 1,
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
        if (node->getPlayer()) {
            auto infSet = node->getAOHInfSet();
            if (nodesInInfSet.find(infSet) == nodesInInfSet.end()) {
                nodesInInfSet.emplace(infSet, EFGNodesDistribution());
            }
            nodesInInfSet[infSet].emplace_back(node, node->chanceReachProb_);
        }
    };
    treeWalk(domain, getAllNodesInInfSet, maxDepth);

    auto initNodesProb = createRootEFGNode(
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
*/
}  // namespace GTLib2
