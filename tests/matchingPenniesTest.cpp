//
// Created by Pavel Rytir on 09/01/2018.
//

#include "../domains/matching_pennies.h"
#include "../algorithms/treeWalk.h"
#include "../algorithms/bestResponse.h"
#include "../algorithms/common.h"
#include "../algorithms/equilibrium.h"
#include "../domains/goofSpiel.h"
#include "../algorithms/utility.h"


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

        auto initNodes = GTLib2::algorithms::createRootEFGNodesFromInitialOutcomeDistribution(
                d.getRootStatesDistribution());
        auto firstNode = (*initNodes.begin()).first;

        auto player0Is = firstNode->getAOHInfSet();
        auto secondNode = (*firstNode->performAction(make_shared<MatchingPenniesAction>(Heads)).begin()).first;

        auto player1Is = secondNode->getAOHInfSet();


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
        auto countingFunction = [&stateCounter](shared_ptr<EFGNode> node) {
            stateCounter += 1;

        };

        algorithms::treeWalkEFG(d, countingFunction, 10);

        BOOST_CHECK(stateCounter == 7);
    }


    BOOST_AUTO_TEST_CASE(num_states_test_simultaneous) {
        SimultaneousMatchingPenniesDomain d;
        int stateCounter = 0;
        auto countingFunction = [&stateCounter](shared_ptr<EFGNode> node) {
            stateCounter += 1;

        };

        algorithms::treeWalkEFG(d, countingFunction, 10);

        BOOST_CHECK(stateCounter == 7);
    }

    BOOST_AUTO_TEST_CASE(num_inf_set_test) {
        MatchingPenniesDomain d;
        auto player1InfSet = unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>();
        auto player2InfSet = unordered_map<shared_ptr<AOH>, vector<shared_ptr<Action>>>();

        auto infSetsFunction = [&player1InfSet, &player2InfSet](shared_ptr<EFGNode> node) {
            optional<int> player = node->getCurrentPlayer();

            if (player) {
                auto aoh = node->getAOHInfSet();
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

        algorithms::treeWalkEFG(d, infSetsFunction, 10);

        BOOST_CHECK(player1InfSet.size() == 1 && player2InfSet.size() == 1 &&
                    player1InfSet.begin()->second.size() == 2 &&
                    player2InfSet.begin()->second.size() == 2);
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(GoofSpiel)

    BOOST_AUTO_TEST_CASE( numberOfRootNodes ) {
        domains::GoofSpielDomain gsd(0);

        int numberOfNodes = algorithms::countNodes(gsd);

        //13 posible nature bids in the start

        BOOST_CHECK(numberOfNodes == 13);


    }

    BOOST_AUTO_TEST_CASE( numberOfFirstAndRootLevelNodes ) {
        domains::GoofSpielDomain gsd(1);

        int numberOfNodes = algorithms::countNodes(gsd);

        //13 root state + 13*13 next level states

        BOOST_CHECK(numberOfNodes == 182);


    }

    BOOST_AUTO_TEST_CASE( numberOfSecondAndFirstAndRootLevelNodes ) {
        domains::GoofSpielDomain gsd(2);

        int numberOfNodes = algorithms::countNodes(gsd);

        //182 previous levels + (13*13)*13*12 ==== 13*13 previous level nodes 13 player2 choices and 12 nature choices

        BOOST_CHECK(numberOfNodes == 26546);


    }

    BOOST_AUTO_TEST_CASE( numberOfInformationSetsDepth3  ) {

        domains::GoofSpielDomain domain(3);

        int player1 = domain.getPlayers()[0];
        int player2 = domain.getPlayers()[1];
        auto player1InfSetsAndActions = algorithms::generateInformationSetsAndAvailableActions(domain, player1);
        auto player2InfSetsAndActions = algorithms::generateInformationSetsAndAvailableActions(domain, player2);

        auto numOfInfSetsPlayer1 = player1InfSetsAndActions.size();
        auto numOfInfSetsPlayer2 = player2InfSetsAndActions.size();

        BOOST_CHECK(numOfInfSetsPlayer1 == 26377 && numOfInfSetsPlayer2 == 26377);


    }

    BOOST_AUTO_TEST_CASE( depth4numberOfNodes ) {
        domains::GoofSpielDomain gsd(4);


        // Long test!!! 30 mins
        //int numberOfNodes = algorithms::countNodes(gsd);

        // [(13*13)*13*12] * [12*12*11]

        //BOOST_CHECK(numberOfNodes == 42103490);


    }

BOOST_AUTO_TEST_SUITE_END()