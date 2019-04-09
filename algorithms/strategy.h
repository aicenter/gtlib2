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

    You should have received a copy of the GNU Lesser General Public 
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef GTLIB2_STRATEGY_H
#define GTLIB2_STRATEGY_H

#include "algorithms/cfr.h"
#include "algorithms/tree.h"
#include "algorithms/common.h"
#include "algorithms/utility.h"

using std::make_pair;
using std::max;


namespace GTLib2 {
namespace algorithms {

/**
 * Calculate average strategy for each player.
 */
StrategyProfile getAverageStrategy(CFRData &data, int maxDepth);

inline StrategyProfile getAverageStrategy(CFRData &data) {
    return getAverageStrategy(data, INT_MAX);
}

/**
 * Create strategy profile where both players play with uniform strategies.
 */
StrategyProfile getUniformStrategy(InfosetCache &data, int maxDepth);

inline StrategyProfile getUniformStrategy(InfosetCache &data) {
    return getUniformStrategy(data, INT_MAX);
}

/**
 * Play selected action with 100% probability in given distribution
 */
void playOnlyAction(ProbDistribution &dist, unsigned long actionIdx);

}  // namespace algorithms
}  // namespace GTLib2


#endif //GTLIB2_STRATEGY_H
