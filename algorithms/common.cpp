//
// Created by Pavel Rytir on 1/21/18.
//

#include "common.h"
#include "treeWalk.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
namespace GTLib2 {

    EFGNodesDistribution
    algorithms::createRootEFGNodesFromInitialOutcomeDistribution(const OutcomeDistribution &probDist) {
        //auto dummyChanceNodeAboveRootNodes = make_shared<EFGNode>();

        EFGNodesDistribution nodes;

        for (auto outcomeProb: probDist) {
            auto outcome = outcomeProb.first;
            auto prob = outcomeProb.second;
            auto state = outcome.state;
            auto node = make_shared<EFGNode>(state, nullptr,
                                             outcome.observations,
                                             outcome.rewards,
                                             vector<pair<int, shared_ptr<Action>>>(),
                                             prob, nullptr,
                                             outcome.observations);
            nodes.emplace_back(node, prob);
        }
        return nodes;
    }



    BehavioralStrategy
    algorithms::mixedToBehavioralStrategy(const vector<BehavioralStrategy> &pureStrats, int player1,
                                          const vector<double> &distribution, const Domain &domain) {
      BehavioralStrategy behavStrat;

      auto updateBehavStrategy = [&behavStrat, &player1](shared_ptr<EFGNode> node) {
        if(*node->getCurrentPlayer() == player1) {
          auto infSet = node->getAOHInfSet();
          if(behavStrat.find(infSet) == behavStrat.end()) {
            behavStrat[infSet] = unordered_map<shared_ptr<Action>, double>();
            for(auto &action :node->availableActions()) {
              behavStrat[infSet][action] = 0.0;
            }
          }
        }
      };

      algorithms::treeWalkEFG(domain, updateBehavStrategy, domain.getMaxDepth());

      for(int i = 0; i < pureStrats.size(); ++i) {
        for(auto &strat : pureStrats[i]) {
           for(auto &action : strat.second) {
            behavStrat[strat.first][action.first] += action.second * distribution[i];
           }
        }
      }
      return behavStrat;
    }


    //TODO: Write a test
    EFGNodesDistribution
    algorithms::getAllNodesInTheInformationSetWithNatureProbability(const shared_ptr<AOH> &infSet,
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