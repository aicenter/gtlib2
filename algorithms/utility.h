//
// Created by Pavel Rytir on 1/21/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_UTILITY_H
#define PURSUIT_UTILITY_H

#include "../base/base.h"

namespace GTLib2 {
    namespace algorithms {
        pair<double,double> computeUtilityTwoPlayersGame(const Domain &domain, const BehavioralStrategy &player1Strat,
        const BehavioralStrategy &player2Strat, int player1, int player2);

        tuple<vector<double>,int,int> constructUtilityMatrixFor(const Domain &domain, int player,
                                                                const vector<BehavioralStrategy> &player1PureStrats,
                                                                const vector<BehavioralStrategy> &player2PureStrats);

        unordered_map<shared_ptr<AOH>,vector<shared_ptr<Action>>> generateInformationSetsAndAvailableActions(const Domain &domain, int player);

        vector<BehavioralStrategy> generateAllPureStrategies(
                const unordered_map<shared_ptr<AOH>,vector<shared_ptr<Action>>> &infSetsAndActions);

    }
}

#endif //PURSUIT_UTILITY_H

#pragma clang diagnostic pop