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

enum AccumulatorWeighting { UniformAccWeighting, LinearAccWeighting };

enum RegretMatching { RegretMatchingNormal, RegretMatchingPlus };

/**
 * Should the "CFR iteration" update regret matching strategy after passing through individual
 * histories, or after passing entire infosets? Passing through infosets requires additional
 * memory and is slower, but is guaranteed to converge as stated in the original CFR paper [1].
 *
 * [1] Zinkevich, Martin, et al. "Regret minimization in games with incomplete information."
 *     Advances in neural information processing systems. 2008.
 */
enum CFRUpdating { HistoriesUpdating, InfosetsUpdating };


struct CFRSettings {
    // todo: CFR+
    AccumulatorWeighting accumulatorWeighting = UniformAccWeighting;
    RegretMatching regretMatching = RegretMatchingNormal;
    CFRUpdating cfrUpdating = HistoriesUpdating;
};


/**
 * Container for regrets and average strategy accumulators
 */
class CFRData: public InfosetCache {

 public:
    inline explicit CFRData(const OutcomeDistribution &rootProbDist, CFRUpdating updatingPolicy) :
        InfosetCache(rootProbDist), updatingPolicy_(updatingPolicy) {}
    inline explicit CFRData(const EFGNodesDistribution &rootNodes, CFRUpdating updatingPolicy) :
        InfosetCache(rootNodes), updatingPolicy_(updatingPolicy) {}

    struct InfosetData {
        vector<double> regrets;
        vector<double> avgStratAccumulator;
        vector<double> regretUpdates;

        /**
         * Disable updating RM strategy in this infoset
         */
        bool fixRMStrategy = false;

        /**
         * Disable updating avg strategy accumulator in this infoset
         */
        bool fixAvgStrategy = false;

        explicit InfosetData(unsigned long numActions, CFRUpdating updatingPolicy) {
            regrets = vector<double>(numActions, 0.0);
            avgStratAccumulator = vector<double>(numActions, 0.0);
            if (updatingPolicy == HistoriesUpdating) regretUpdates = vector<double>(0);
            else regretUpdates = vector<double>(numActions, 0.);
        }
    };

    unordered_map<shared_ptr<AOH>, InfosetData> infosetData;

 protected:
    void createNode(const shared_ptr<EFGNode> &node) override {
        InfosetCache::createNode(node);

        if (node->isTerminal()) return;

        auto infoSet = node->getAOHInfSet();
        if (infosetData.find(infoSet) == infosetData.end()) {
            infosetData.emplace(make_pair(
                infoSet, CFRData::InfosetData(node->countAvailableActions(), updatingPolicy_)));
        }

    }

    CFRUpdating updatingPolicy_ = HistoriesUpdating;
};


/**
 * Index of the chance (nature) player
 */
constexpr int CHANCE_PLAYER = 2;


/**
 * Run CFR on EFG tree for a number of iterations for both players.
 * This implementation is based on Algorithm 1 in M. Lanctot PhD thesis.
 */
class CFRAlgorithm: public GamePlayingAlgorithm {
 public:
    CFRAlgorithm(const Domain &domain, Player playingPlayer, CFRSettings settings);
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

    // todo: move into private!
    void delayedApplyRegretUpdates();

 private:
    CFRData cache_;
    CFRSettings settings_;

    void nodeUpdateRegrets(shared_ptr<EFGNode> node);

};

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_CFR_H_
