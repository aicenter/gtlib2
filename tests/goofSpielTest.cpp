//
// Created by Pavel Rytir on 30/01/2018.
//


#include "../algorithms/treeWalk.h"
#include "../algorithms/bestResponse.h"
#include "../algorithms/common.h"
#include "../algorithms/equilibrium.h"
#include "../domains/goofSpiel.h"
#include "../algorithms/utility.h"



#define BOOST_TEST_DYN_LINK // For linking with dynamic libraries.


#include <boost/test/unit_test.hpp>

using namespace GTLib2;

BOOST_AUTO_TEST_SUITE(GoofSpiel)

    BOOST_AUTO_TEST_CASE( bestResponseDepth2 ) {
        domains::GoofSpielDomain gsd(2);

        int player1 = gsd.getPlayers()[0];
        int player2 = gsd.getPlayers()[1];


        //Create strategy that plays the lowest card
        BehavioralStrategy player2Strat;
        auto lowestCardAction = make_shared<domains::GoofSpielAction>(1);
        auto setAction = [&player2Strat, &lowestCardAction](shared_ptr<EFGNode> node) {
            if (node->getDistanceFromRoot() == 0) {
                player2Strat[node->getAOHInfSet()] = {{lowestCardAction,1.0}};
            }
        };
        algorithms::treeWalkEFG(gsd,setAction);

        auto player1BestResponse = algorithms::bestResponseTo(player2Strat,player2,player1,gsd);


        BOOST_CHECK(std::abs(player1BestResponse.second - 7) <= 0.001);

    }

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

        // Number of infSets: rootStates: 13, p1 after first round : 13*(13*13)*12

        BOOST_CHECK(numOfInfSetsPlayer1 == 26377 && numOfInfSetsPlayer2 == 26377);


    }

    BOOST_AUTO_TEST_CASE( depth4numberOfNodes ) {
        domains::GoofSpielDomain gsd(4);


        //Long test!!! 30 mins

        //int numberOfNodes = algorithms::countNodes(gsd);

        // [(13*13)*13*12] * [12*12*11]

        //BOOST_CHECK(numberOfNodes == 42103490);


    }

BOOST_AUTO_TEST_SUITE_END()