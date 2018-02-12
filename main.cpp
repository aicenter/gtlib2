//
// Created by rozliv on 02.08.2017.
//

//#include <gtest/gtest.h>

#include "algorithms/normalFormLP.h"
#include "domains/phantomTTT.h"
#include "LPsolvers/CplexLPSolver.h"
//#include "glpkfiles/GlpkLPSolver.h"
#include "domains/pursuit.h"
#include "base/efg.h"
#include "base/efg_old.h"
#include "domains/matching_pennies.h"
#include "algorithms/treeWalk.h"
#include "algorithms/common.h"
#include "algorithms/bestResponse.h"
#include "algorithms/equilibrium.h"
#include "domains/goofSpiel.h"
#include "algorithms/utility.h"

//#include <boost/test/unit_test.hpp>

using std::endl;

using namespace GTLib2;

void goofSpiel5() {
    domains::GoofSpielDomain gsd(4);
    int numberOfNodes = algorithms::countNodes(gsd);
    cout << "Number of nodes: " << numberOfNodes << endl;
}

void goofSpiel() {
    //assert(false);
    domains::GoofSpielDomain gsd(4,8);

    int player1 = gsd.getPlayers()[0];
    int player2 = gsd.getPlayers()[1];

    BehavioralStrategy player2Strat;

    //auto rootNodes = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(gsd.getRootStatesDistribution());
    auto lowestCardAction = make_shared<domains::GoofSpielAction>(1);
    auto secondLowestCardAction = make_shared<domains::GoofSpielAction>(2);
    auto thirdLowestCardAction = make_shared<domains::GoofSpielAction>(3);
    auto fourthLowestCardAction = make_shared<domains::GoofSpielAction>(4);



    auto setAction = [&player2Strat, &lowestCardAction, &secondLowestCardAction, &thirdLowestCardAction, &fourthLowestCardAction](shared_ptr<EFGNode> node) {
        if (node->getDistanceFromRoot() == 0) {
            player2Strat[node->getAOHInfSet()] = {{lowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 2) {
            player2Strat[node->getAOHInfSet()] = {{secondLowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 4) {
            player2Strat[node->getAOHInfSet()] = {{thirdLowestCardAction, 1.0}};
        } else if (node->getDistanceFromRoot() == 6) {
            player2Strat[node->getAOHInfSet()] = {{fourthLowestCardAction, 1.0}};
        }
    };

    algorithms::treeWalkEFG(gsd, setAction);


    auto player1BestResponse = algorithms::bestResponseTo(player2Strat, player2, player1, gsd);

    cout << "Value of the best response: " << player1BestResponse.second << endl;

    auto val = algorithms::computeUtilityTwoPlayersGame(gsd,player2Strat, player1BestResponse.first, player2, player1);

    cout << "val" << val.first << endl;




}

void goofSpiel2() {
    domains::GoofSpielDomain domain(3);

    int player1 = domain.getPlayers()[0];
    int player2 = domain.getPlayers()[1];
    auto player1InfSetsAndActions = algorithms::generateInformationSetsAndAvailableActions(domain, player1);
    auto player2InfSetsAndActions = algorithms::generateInformationSetsAndAvailableActions(domain, player2);


    //int cnt = algorithms::countNodes(d);

    cout << "Goof spiel number of infSets of p1: " << player1InfSetsAndActions.size() << endl;
    cout << "Goof spiel number of infSets of p2: " << player2InfSetsAndActions.size() << endl;

}

void equilibrium() {
    MatchingPenniesDomain d;
    auto v = algorithms::findEquilibriumTwoPlayersZeroSum(d);

    double val = std::get<0>(v);

    auto strat = std::get<1>(v);

    auto actionHeads = make_shared<MatchingPenniesAction>(Heads);

    auto actionTails = make_shared<MatchingPenniesAction>(Tails);

    double headsProb = (*strat.begin()).second[actionHeads];

    double tailsProb = (*strat.begin()).second[actionTails];

    cout << headsProb << " " << tailsProb << endl;


    cout << "Value of equilibrium: " << std::get<0>(v) << endl;

}

void pavelTreeWalkTest3() {
    MatchingPenniesDomain d;

    auto initNodes = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(d.getRootStatesDistribution());
    auto firstNode = (*initNodes.begin()).first;

    auto player0Is = firstNode->getAOHInfSet();
    auto secondNode = (*firstNode->performAction(make_shared<MatchingPenniesAction>(Heads)).begin()).first;

    auto player1Is = secondNode->getAOHInfSet();


    BehavioralStrategy stratHeads;

    auto action = make_shared<MatchingPenniesAction>(Heads);

    stratHeads[player0Is] = {{action, 1.0}};


    auto brsVal = algorithms::bestResponseTo(stratHeads, 0, 1, d, 5);

    auto optAction = (*brsVal.first[player1Is].begin()).first;


    cout << "Best response: ";


}


int main(int argc, char *argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));
    clock_t begin = clock();


    goofSpiel5();

    clock_t end = clock();
    double elapsed_secs = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
    cout << "hotovo: time " << elapsed_secs << "s" << '\n';

    return 0;
}
