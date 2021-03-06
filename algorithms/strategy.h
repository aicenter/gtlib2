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


inline ProbDistribution normalizeProbability(vector<double> probabilities) {
    ProbDistribution normProbs = vector<double>(probabilities.size());
    double sum = 0;
    for (unsigned int i = 0; i < probabilities.size(); i++) {
        sum += probabilities[i];
    }
    for (unsigned int i = 0; i < probabilities.size(); i++) {
        normProbs[i] = probabilities[i] / sum;
    }
    return normProbs;
}

struct StrategyValue {
    BehavioralStrategy strategy;
    double value; // at root node for player 0

    inline StrategyValue() : strategy(BehavioralStrategy()), value(0) {}
    inline StrategyValue(BehavioralStrategy strategy_, double value_)
        : strategy(move(strategy_)), value(value_) {}
    inline StrategyValue(const StrategyValue &other) {
        value = other.value;
        strategy = other.strategy;
    }
};

/**
 * Calculate average strategy profile for each player.
 */
StrategyProfile getAverageStrategy(CFRData &data);

/**
 * Create strategy profile where both players play with uniform strategies.
 */
StrategyProfile getUniformStrategy(InfosetCache &data);

/**
 * Play selected action with 100% probability in given distribution
 */
void playOnlyAction(ProbDistribution &dist, unsigned long actionIdx);

/**
 * Play selected action with 100% probability in given distribution
 */
void playOnlyAction(ActionProbDistribution &dist, const shared_ptr<Action> &action);

/**
 * Play uniformly over n actions
 */
ProbDistribution playUniformly(unsigned long numActions);


ActionProbDistribution mapDistribution(const ProbDistribution &dist,
                                       const vector<shared_ptr<Action>> &actions);

/**
 * Calculate reach probabilities for each player
 */
array<double, 3> calcReachProbs(const shared_ptr<EFGNode> &h, const StrategyCache* cache);

}  // namespace GTLib2


#endif //GTLIB2_STRATEGY_H
