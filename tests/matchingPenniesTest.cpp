//
// Created by Pavel Rytir on 09/01/2018.
//

#include "../domains/matching_pennies.h"
#include "../algorithms/treeWalk.h"
#include "../algorithms/bestResponse.h"
#include "../algorithms/common.h"
#include "../algorithms/equilibrium.h"


#define BOOST_TEST_DYN_LINK // For linking with dynamic libraries.

#define BOOST_TEST_MODULE MatchPenTest test

#include <boost/test/unit_test.hpp>

using namespace GTLib2;

BOOST_AUTO_TEST_SUITE(Matching_Pennies)

    BOOST_AUTO_TEST_CASE( best_response_to_equilibrium ) {
        MatchingPenniesDomain d;
        auto v = GTLib2::algorithms::findEquilibriumTwoPlayersZeroSum(d);

        auto strat = std::get<1>(v);

        auto brsVal = algorithms::bestResponseTo(strat, 0, 1, d, 5);

        double val = std::get<1>(brsVal);

        BOOST_CHECK(val == 0.0);


    }


    BOOST_AUTO_TEST_CASE( equilibrium_normal_form_lp_test ) {
        MatchingPenniesDomain d;
        auto v = GTLib2::algorithms::findEquilibriumTwoPlayersZeroSum(d);

        auto strat = std::get<1>(v);

        auto actionHeads = make_shared<MatchingPenniesAction>(Heads);

        auto actionTails = make_shared<MatchingPenniesAction>(Tails);

        double headsProb = (*strat.begin()).second[actionHeads];

        double tailsProb = (*strat.begin()).second[actionTails];

        BOOST_CHECK(std::get<0>(v) == 0);

        BOOST_CHECK(headsProb == 0.5 && tailsProb == 0.5);

    }


    BOOST_AUTO_TEST_CASE(best_response_test) {
        MatchingPenniesDomain d;

        auto initNodes = GTLib2::algorithms::createEFGNodesFromDomainInitDistr(*d.getRootStateDistributionPtr());
        auto firstNode = (*initNodes.begin()).first;

        auto player0Is = firstNode->pavelgetAOHInfSet();
        auto secondNode = (*firstNode->pavelPerformAction(make_shared<MatchingPenniesAction>(Heads)).begin()).first;

        auto player1Is = secondNode->pavelgetAOHInfSet();


        BehavioralStrategy stratHeads;

        auto action = make_shared<MatchingPenniesAction>(Heads);

        stratHeads[player0Is] = {{action, 1.0}};


        auto brsVal = algorithms::bestResponseTo(stratHeads, 0, 1, d, 5);

        auto optAction = (*brsVal.first[player1Is].begin()).first;

        auto tailAction = make_shared<MatchingPenniesAction>(Tails);

        BOOST_CHECK(*optAction == *tailAction);
    }


    BOOST_AUTO_TEST_CASE(num_states_test) {
        MatchingPenniesDomain d;
        int stateCounter = 0;
        auto countingFunction = [&stateCounter](shared_ptr<EFGNode> node, double prob) {
            stateCounter += 1;
        };

        algorithms::pavelEFGTreeWalk(d, countingFunction, 10);

        BOOST_CHECK(stateCounter == 7);
    }


    BOOST_AUTO_TEST_CASE(num_states_test_simultaneous) {
        SimultaneousMatchingPenniesDomain d;
        int stateCounter = 0;
        auto countingFunction = [&stateCounter](shared_ptr<EFGNode> node, double prob) {
            stateCounter += 1;
        };

        algorithms::pavelEFGTreeWalk(d, countingFunction, 10);

        BOOST_CHECK(stateCounter == 7);
    }

    BOOST_AUTO_TEST_CASE(num_inf_set_test) {
        MatchingPenniesDomain d;
        auto player1InfSet = unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>();
        auto player2InfSet = unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>();

        auto infSetsFunction = [&player1InfSet, &player2InfSet](shared_ptr<EFGNode> node, double prob) {
            optional<int> player = node->getCurrentPlayer();

            if (player) {
                auto aoh = node->pavelgetAOHInfSet();
                if (aoh != nullptr) {
                    auto actions = node->availableActions();
                    if (*player == 0) {
                        player1InfSet[aoh] = actions;
                    } else {
                        player2InfSet[aoh] = actions;
                    }
                }
            }


        };

        algorithms::pavelEFGTreeWalk(d, infSetsFunction, 10);

        BOOST_CHECK(player1InfSet.size() == 1 && player2InfSet.size() == 1 &&
                    player1InfSet.begin()->second.size() == 2 &&
                    player2InfSet.begin()->second.size() == 2);
    }

BOOST_AUTO_TEST_SUITE_END()