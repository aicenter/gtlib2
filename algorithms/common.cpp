//
// Created by Pavel Rytir on 1/21/18.
//

#include "common.h"
#include "treeWalk.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {

    EGGNodesDistribution
    algorithms::createRootEFGNodesFromInitialOutcomeDistribution(const OutcomeDistribution &probDist) {
        //auto dummyChanceNodeAboveRootNodes = make_shared<EFGNode>();

        EGGNodesDistribution nodes;

        for (auto outcomeProb: probDist) {
            auto outcome = outcomeProb.first;
            auto prob = outcomeProb.second;
            auto state = outcome.state;
            auto node = make_shared<EFGNode>(state, nullptr,
                                             outcome.observations,
                                             outcome.rewards,
                                             unordered_map<int, shared_ptr<Action>>(),
                                             prob, nullptr,
                                             outcome.observations);
            nodes.emplace_back(node, prob);
        }
        return nodes;
    }

    BehavioralStrategy algorithms::mixedStrategyToBehavioralStrategy(const vector<BehavioralStrategy> &pureStrats,
                                                                     const vector<double> &distribution,
                                                                     const Domain &domain) {
        BehavioralStrategy behavStrat;

        int stratIndex = 0;
        for (const auto &pureStrat : pureStrats) {
            double stratProb = distribution[stratIndex];
            auto rootNodes = createRootEFGNodesFromInitialOutcomeDistribution(domain.getRootStatesDistribution());

            // Inner function
            std::function<void(const shared_ptr<EFGNode> &)> updateBehavStratStartingFromNode =
                    [&behavStrat, &pureStrat, &stratProb, &updateBehavStratStartingFromNode]
                            (const shared_ptr<EFGNode> &node) {
                        auto infSet = node->getAOHInfSet();

                        if (pureStrat.find(infSet) == pureStrat.end()) {
                            return;
                        }
                        auto strategyAction = (*pureStrat.at(infSet).begin()).first;

                        //Update behavioral strategy
                        auto infSetActionsDistr = (behavStrat.find(infSet) == behavStrat.end()) ?
                                                  unordered_map<shared_ptr<Action>, double>() : behavStrat[infSet];
                        auto actionProb = (infSetActionsDistr.find(strategyAction) == infSetActionsDistr.end()) ?
                                          0.0 : infSetActionsDistr[strategyAction];
                        infSetActionsDistr[strategyAction] = actionProb + stratProb;
                        behavStrat[infSet] = infSetActionsDistr;
                        // End of update of the bahavioral strategy

                        //Proceed to the next node(nodes in case of stochastic game)


                        auto newNodes = node->performAction(strategyAction);
                        for (const auto &newNode : newNodes) {
                            updateBehavStratStartingFromNode(newNode.first);
                        }
                    };
            // End inner function

            for (const auto &rootNode : rootNodes) {
                updateBehavStratStartingFromNode(rootNode.first);
            }
            stratIndex++;
        }

        return behavStrat;
    }


    //TODO: Write a test
    EGGNodesDistribution
    algorithms::gelAllNodesInTheInformationSetWithNatureProbability(const shared_ptr<AOH> &infSet,
                                                                    const Domain &domain) {


        vector<pair<shared_ptr<EFGNode>, double>> nodes;

        auto aoh = infSet->getAOHistory();
        int player = infSet->getPlayer();

        std::function<void(shared_ptr<EFGNode>, int, int)> traverse =
                [&nodes, &aoh, &player, &traverse, &infSet](shared_ptr<EFGNode> node,
                                                   int actionIndex, int observationIdToCheck) {

//                    if (actionIndex >= aoh.size()) {
//                        if (node->getLastObservationIdOfCurrentPlayer() == observationIdToCheck &&
//                                node->getCurrentPlayer()
//                            && *node->getCurrentPlayer() == player) {
//                            nodes.emplace_back(node, node->natureProbability);
//                            assert(node->isContainedInInformationSet(infSet));
//                            cout << "x"<<std::endl;
//                        }
//                        return;
//                    }

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
                };



//        auto checkAndAdd = [&infSet, &nodes](shared_ptr<EFGNode> node) {
//            if (node->isContainedInInformationSet(infSet)) {
//                nodes.emplace_back(node,node->natureProbability);
//            }
//        };
//
//        treeWalkEFG(domain, checkAndAdd, domain.getMaxDepth());

        auto rootNodes = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
                domain.getRootStatesDistribution());


        for (const auto &rootNode : rootNodes) {
            traverse(rootNode.first, 0, infSet->getInitialObservationId());
        }


        return nodes;
    }


}

#pragma clang diagnostic pop