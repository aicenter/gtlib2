/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


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
