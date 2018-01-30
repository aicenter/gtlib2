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

//#include <boost/test/unit_test.hpp>

using namespace GTLib2;

int goofSpiel() {
    domains::GoofSpielDomain d(4);

    int cnt = algorithms::countNodes(d);

    cout << "Goof spiel number of nodes: " << cnt << endl;

}

int equilibrium() {
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

int pavelTreeWalkTest3() {
    MatchingPenniesDomain d;

    auto initNodes = algorithms::createRootEFGNodesFromInitialOutcomeDistribution(d.getRootStatesDistribution());
    auto firstNode = (*initNodes.begin()).first;

    auto player0Is = firstNode->getAOHInfSet();
    auto secondNode = (*firstNode->performAction(make_shared<MatchingPenniesAction>(Heads)).begin()).first;

    auto player1Is = secondNode->getAOHInfSet();



    BehavioralStrategy stratHeads;

    auto action = make_shared<MatchingPenniesAction>(Heads);

    stratHeads[player0Is] = {{action,1.0}};


    auto brsVal = algorithms::bestResponseTo(stratHeads, 0, 1, d, 5);

    auto optAction = (*brsVal.first[player1Is].begin()).first;




    cout << "Best response: ";


}


int pursuitGame1() {
    PursuitDomain::width_ = 3;
    PursuitDomain::height_ = 3;
    PursuitDomain pd = PursuitDomain(2);
    cout << pd.getInfo();
    //shared_ptr<Domain> d = make_shared<PursuitDomain>(2);
    //shared_ptr<LPSolver> solver = make_shared<CplexLPSolver>();
    CplexLPSolver solver = CplexLPSolver();

    NormalFormLP nor(make_shared<PursuitDomain>(pd) , make_shared<CplexLPSolver>(solver));
    cout << "vysledek hry: " << nor.SolveGame() << "\n";
    cout << pd.getInfo();
    return 0;
}



int pavelTreeWalkTest2() {
    MatchingPenniesDomain d;
    int stateCounter = 0;
    auto player1InfSet = unordered_map<shared_ptr<AOH>,vector<shared_ptr<Action>>>();
    auto player2InfSet = unordered_map<shared_ptr<AOH>,vector<shared_ptr<Action>>>();

    auto countingFunction = [&stateCounter, &player1InfSet, &player2InfSet](shared_ptr<EFGNode> node) {
        stateCounter += 1;
        cout << node->description() << endl;
        cout << node->getState()->toString(-1) << endl;

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

    algorithms::treeWalkEFG(d, countingFunction, 10);

    cout << "Number of states: " << stateCounter << endl;

    cout << player1InfSet.size() << " " << player2InfSet.size() << endl;
}

int pavelTreeWalkTest() {
    SimultaneousMatchingPenniesDomain d;
    int stateCounter = 0;
    auto countingFunction = [&stateCounter](shared_ptr<EFGNode> node) {
        stateCounter += 1;
        cout << node->description() << endl;
        cout << node->getState()->toString(-1) << endl;
    };

    algorithms::treeWalkEFG(d, countingFunction, 10);

    cout << "Number of states: " << stateCounter << endl;
}

int matchingPenniesTest() {
    MatchingPenniesDomain d;


    auto rootState = d.getRootStateDistributionPtr()->GetOutcomes()[0].GetState();

    cout << rootState->toString(-1) << endl;

    vector<shared_ptr<Action>> acts;
    acts.push_back(make_shared<MatchingPenniesAction>(Tails));
    acts.push_back(make_shared<Action>(NoA));

    auto nextState = rootState->PerformAction(acts).GetOutcomes()[0].GetState();
    cout << nextState->toString(-1) << endl;

    vector<shared_ptr<Action>> acts2;
    acts2.push_back(make_shared<Action>(NoA));
    acts2.push_back(make_shared<MatchingPenniesAction>(Heads));


    auto nextState2 = nextState->PerformAction(acts2).GetOutcomes()[0].GetState();
    cout << nextState2->toString(-1) << endl;


    int stateCounter = 0;

    //Count states
    std::function<void(EFGNode*,int*)> countingFunction =
            [](EFGNode* node, int* counter) {
                (*counter)++;
                auto state = std::dynamic_pointer_cast<MatchingPenniesState>(node->getState());

                cout << "State visited -- " << state->toString(-1) << " ||| p1: ";
                auto whoPlays = state->GetPlayers();
                cout << (whoPlays[0] ? "Plays" : "Don't") << " ";
                cout << "p2: " << (whoPlays[1] ? "Plays" : "Don't");
                cout << " --- Availabe action for p1: ";

                auto availActionForP1 = state->getAvailableActionsFor(0);
                for (auto action : availActionForP1) {
                    cout << action->toString() << " - ";
                }
                auto availActionForP2 = state->getAvailableActionsFor(1);
                cout << " --- Availabe action for p2: ";
                for (auto action : availActionForP2) {
                    cout << action->toString() << " - ";
                }

                cout << endl;


            };


    OldEFGTreewalkStart(make_shared<MatchingPenniesDomain>(d),
                        std::bind(countingFunction, std::placeholders::_1, &stateCounter));

    cout << "Number of states is: " << stateCounter << endl;

}


int printTree(){

}



int main(int argc, char* argv[]) {
  srand(static_cast<unsigned int>(time(nullptr)));
  clock_t begin = clock();
  //countStates = 0;


    goofSpiel();
    //equilibrium();

  //mapa = unordered_map<size_t, vector<EFGNode>>();
//  shared_ptr<Domain> d = make_shared<PhantomTTTDomain>(4);

    //matchingPenniesTest();

   // pursuitGame1();
    
    
    
    
    
    
  //reward = vector<double>(d->GetMaxPlayers());
//  shared_ptr<LPSolver> solver = make_shared<GlpkLPSolver>();
//  NormalFormLP nor(d, solver);
//  cout << "vysledek hry: " << nor.SolveGame() << "\n\nCplex:\n";
//
//  shared_ptr<LPSolver> solver2 = make_shared<CplexLPSolver>();
//  NormalFormLP nor2(d, solver2);
//  cout << "vysledek hry: " << nor2.SolveGame() << "\n";
//  cout << d->GetInfo();
//  OldEFGTreewalkStart(d);
//  OldTreewalkStart(d);
//  PursuitStart(d);
//  for (double i : reward) {
//    cout << i << " ";
//  }
//  cout << countStates << '\n';
//  cout << mapa.size() << '\n';
//  int s = 0;
//  for (auto &i : mapa) {
//    s+= i.second.size();
//  }
//  cout << s <<"\n";
  clock_t end = clock();
  double elapsed_secs = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
  cout << "hotovo: time " << elapsed_secs << "s" << '\n';

//  getchar();
//  testing::InitGoogleTest(&argc, argv);
//  cout << RUN_ALL_TESTS();
  return 0;
}
