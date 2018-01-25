//
// Created by Pavel Rytir on 1/21/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef PURSUIT_COMMON_H
#define PURSUIT_COMMON_H

#include "../base/base.h"
#include "../base/efg.h"

namespace GTLib2 {
    namespace algorithms {

        unordered_map<shared_ptr<EFGNode>, double> createEFGNodesFromDomainInitDistr(const ProbDistribution &probDist);
        BehavioralStrategy mixedStrategyToBehavioralStrategy(const vector<BehavioralStrategy> &pureStrats,
                                                             const vector<double> &distribution,
                                                             const Domain &domain);

        //Naive implementation. Maybe iterator over the nodes would be better.
        vector<pair<shared_ptr<EFGNode>,double>>
        gelAllNodesInTheInformationSetWithNatureProbability(const shared_ptr<AOH> &infSet,
                                                            const Domain &domain);





    }
}

#endif //PURSUIT_COMMON_H

#pragma clang diagnostic pop