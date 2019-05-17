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

#include "base/base.h"
#include "algorithms/cfr.h"
#include "algorithms/tree.h"
#include "algorithms/common.h"
#include "algorithms/utility.h"


namespace GTLib2::algorithms {

/**
 * Get probability of a (possibly missing) action in the ActionProbDistribution map.
 */
inline double getActionProb(const ActionProbDistribution &dist, const shared_ptr<Action> &action) {
    if (dist.find(action) == dist.end()) return 0.0;
    return dist.at(action);
}

struct StrategyValue {
    BehavioralStrategy strategy;
    double value; // at root node

    inline StrategyValue() : strategy(BehavioralStrategy()), value(0) {}
    inline StrategyValue(BehavioralStrategy strategy, double value)
        : strategy(move(strategy)), value(value) {}
};

/**
 * Calculate average strategy for each player.
 */
StrategyProfile getAverageStrategy(CFRData &data, int maxDepth);

inline StrategyProfile getAverageStrategy(CFRData &data) {
    return getAverageStrategy(data, data.getDomainMaxStateDepth());
}

/**
 * Create strategy profile where both players play with uniform strategies.
 */
StrategyProfile getUniformStrategy(InfosetCache &data, int maxDepth);

inline StrategyProfile getUniformStrategy(InfosetCache &data) {
    return getUniformStrategy(data, data.getDomainMaxStateDepth());
}

/**
 * Play selected action with 100% probability in given distribution
 */
void playOnlyAction(ProbDistribution &dist, unsigned long actionIdx);

/**
 * Play selected action with 100% probability in given distribution
 */
void playOnlyAction(ActionProbDistribution &dist, const shared_ptr<Action> &action);


ActionProbDistribution mapDistribution(const ProbDistribution &dist,
                                       const vector<shared_ptr<Action>> &actions);

}  // namespace GTLib2


#endif //GTLIB2_STRATEGY_H
