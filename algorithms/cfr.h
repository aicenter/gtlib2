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

#include "base/algorithm.h"
#include "base/efg.h"
#include "base/base.h"
#include "base/cache.h"

namespace GTLib2 {
namespace algorithms {

/**
 * Container for regrets and average strategy accumulators
 */
class CFRData: public InfosetCache {

 public:
    inline explicit CFRData(const OutcomeDistribution &rootProbDist) : InfosetCache(rootProbDist) {}
    inline explicit CFRData(const EFGNodesDistribution &rootNodes) : InfosetCache(rootNodes) {}

    struct InfosetData {
        vector<double> regrets;
        vector<double> avgStratAccumulator;

        explicit InfosetData(unsigned long numActions) {
            regrets = vector<double>(numActions, 0.0);
            avgStratAccumulator = vector<double>(numActions, 0.0);
        }
    };

    unordered_map<shared_ptr<AOH>, InfosetData> infosetData;
};

constexpr int CHANCE_PLAYER = 2;

/**
 * Run CFR on EFG tree for a number of iterations for both players.
 * This implementation is based on Algorithm 1 in M. Lanctot PhD thesis.
 */
class CFRAlgorithm: public GamePlayingAlgorithm {
 public:
    CFRAlgorithm(const Domain &domain, Player playingPlayer);
    bool runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    vector<double> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

    /**
     * Run an updating iteration for specified player starting at some node.
     * @return counterfactual value for infoset under current (regret-matching) strategy.
     */
    double runIteration(const shared_ptr<EFGNode> &node, const std::array<double, 3> reachProbs,
                        const Player updatingPl);
    void runIterations(int numIterations);

    inline CFRData &getCache() {
        return cache_;
    }

 private:
    CFRData cache_;
};

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_CFR_H_
