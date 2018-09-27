//
// Created by Jakub Rozlivek on 8/8/18.
//

#ifndef ALGORITHMS_CFR_H_
#define ALGORITHMS_CFR_H_

#include <vector>
#include <utility>
#include "base/base.h"

namespace GTLib2 {
namespace algorithms {
BehavioralStrategy getStrategyFor(const Domain &domain,
                                  int player,
                                  const unordered_map<shared_ptr<InformationSet>,
                                                      pair<vector<double>, vector<double>>> &allMP);

/**
 * The main function for CFR algorithm returning utility.
 */
pair<double, double> CFR(const Domain &domain, int iterations);

/**
 * The function for CFR iteration.
 * Implementation based on Algorithm 1 in M. Lanctot PhD thesis.
 * It should be a first choice.
 */
unordered_map<shared_ptr<InformationSet>, pair<vector<double>, vector<double>>>
CFRiterations(const Domain &domain, int iterations);

/**
 * The function for CFR iteration.
 * Implementation based on Algorithm 1 in M. Lanctot PhD thesis.
 * AOhistory created in iterations, not in nodes.
 */
unordered_map<shared_ptr<InformationSet>, pair<vector<double>, vector<double>>>
CFRiterationsAOH(const Domain &domain, int iterations);
}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_CFR_H_
