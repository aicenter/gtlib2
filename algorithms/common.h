//
// Created by Pavel Rytir on 1/21/18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef ALGORITHMS_COMMON_H_
#define ALGORITHMS_COMMON_H_

#include "../base/base.h"
#include "../base/efg.h"

namespace GTLib2 {
namespace algorithms {

EFGNodesDistribution createRootEFGNodesFromInitialOutcomeDistribution(
    const OutcomeDistribution &probDist);

BehavioralStrategy mixedToBehavioralStrategy(const vector<BehavioralStrategy> &pureStrats,
                                             int player1,
                                             const vector<double> &distribution,
                                             const Domain &domain);

EFGNodesDistribution
getAllNodesInTheInformationSetWithNatureProbability(const shared_ptr<AOH> &infSet,
                                                    const Domain &domain);

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_COMMON_H_

#pragma clang diagnostic pop
