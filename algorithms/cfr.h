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
#include <array>
#include <utility>
#include "domains/matching_pennies.h"
#include "base/efg.h"
#include "base/base.h"

namespace GTLib2 {
namespace algorithms {

/**
 * Container for regrets and average strategy accumulators
 */
class CFRData: public EFGCache {

 public:
    inline explicit CFRData(const OutcomeDistribution &rootProbDist) : EFGCache(rootProbDist) {}
    inline explicit CFRData(const EFGNodesDistribution &rootNodes) : EFGCache(rootNodes) {}

    struct InfosetData {
        vector<double> regrets;
        vector<double> avgStratAccumulator;

        explicit InfosetData(unsigned long numActions) {
            regrets = vector<double>(numActions, 0.);
            avgStratAccumulator = vector<double>(numActions, 0.);
        }
    };

    unordered_map<shared_ptr<AOH>, InfosetData> infosetData;
};

/**
 * Calculate average strategy for each player.
 */
StrategyProfile getAverageStrategy(CFRData *data);

double CFRiteration(CFRData *data, const shared_ptr<EFGNode> &node,
                    const std::array<double, 2> pi, const Player exploringPl);

/**
 * Run CFR on EFG tree for a number of iterations for both players.
 * This implementation is based on Algorithm 1 in M. Lanctot PhD thesis.
 */
void CFRiterations(CFRData *data, int numIterations);

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_CFR_H_
