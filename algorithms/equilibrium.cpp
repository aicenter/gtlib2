//
// Created by Pavel Rytir on 22/01/2018.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

#include "equilibrium.h"
#include "utility.h"
#include "../LPsolvers/simpleCplexSolver.h"
#include "common.h"


namespace GTLib2 {
    namespace algorithms {
        tuple<double, BehavioralStrategy> findEquilibriumTwoPlayersZeroSum(const GTLib2::Domain &domain) {
            int player1 = domain.getPlayers()[0];
            int player2 = domain.getPlayers()[1];
            auto player1InfSetsAndActions = generateInformationSetsAndAvailableActions(domain, player1);
            auto player2InfSetsAndActions = generateInformationSetsAndAvailableActions(domain, player2);
            auto player1PureStrats = generateAllPureStrategies(player1InfSetsAndActions);
            auto player2PureStrats = generateAllPureStrategies(player2InfSetsAndActions);
            auto utilityMatrixPlayer1 = constructUtilityMatrixFor(domain,player1,player1PureStrats, player2PureStrats);

            // Solution is a probability distribution over player1PureStrats
            vector<double> solution(std::get<1>(utilityMatrixPlayer1)*std::get<2>(utilityMatrixPlayer1), 0.0);

            double val = solveLP(std::get<1>(utilityMatrixPlayer1),std::get<2>(utilityMatrixPlayer1),
                   std::get<0>(utilityMatrixPlayer1),solution);

            auto equilibriumStrat= mixedStrategyToBehavioralStrategy(player1PureStrats, solution, domain);


            return tuple<double, BehavioralStrategy>(val,equilibriumStrat);

        }
    }
}
#pragma clang diagnostic pop