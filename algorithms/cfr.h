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

#include "base/base.h"

#include "base/algorithm.h"
#include "base/efg.h"
#include "base/cache.h"

namespace GTLib2::algorithms {

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
class CFRData: public virtual InfosetCache {

 public:
    inline explicit CFRData(const Domain &domain, CFRUpdating updatingPolicy) :
        EFGCache(domain),
        InfosetCache(domain),
        updatingPolicy_(updatingPolicy) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createCFRInfosetData(n); });
    }

    struct InfosetData {
        vector<double> regrets;
        vector<double> avgStratAccumulator;
        vector<double> regretUpdates;
        unsigned int numUpdates = 0;

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
    CFRUpdating updatingPolicy_ = HistoriesUpdating;

 private:
    void createCFRInfosetData(const shared_ptr<EFGNode> &node) {
        if (node->type_ != PlayerNode) return;

        auto infoSet = node->getAOHInfSet();
        if (infosetData.find(infoSet) == infosetData.end()) {
            infosetData.emplace(make_pair(
                infoSet, CFRData::InfosetData(node->countAvailableActions(), updatingPolicy_)));
        }
    }
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
    CFRAlgorithm(const Domain &domain,
                 CFRData &cache,
                 Player playingPlayer,
                 CFRSettings settings);
    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

    /**
     * Run an updating iteration for specified player starting at some node.
     * @return counterfactual value for infoset under current (regret-matching) strategy.
     */
    double runIteration(const shared_ptr<EFGNode> &node, const array<double, 3> reachProbs,
                        const Player updatingPl);
    void runIterations(int numIterations);

    inline CFRData &getCache() {
        return cache_;
    }

    // todo: move into private!
    void delayedApplyRegretUpdates();

 private:
    CFRData &cache_;
    CFRSettings settings_;

    void nodeUpdateRegrets(const shared_ptr<EFGNode> &node);

};

struct ExpectedUtility {
    double rmUtility;
    double avgUtility;

    ExpectedUtility(double rmUtility, double avgUtility) {
        this->rmUtility = rmUtility;
        this->avgUtility = avgUtility;
    }

    bool operator==(const ExpectedUtility &rhs) const {
        return rmUtility == rhs.rmUtility && avgUtility == rhs.avgUtility;
    }

    friend std::ostream &
    operator<<(std::ostream &ss, ExpectedUtility const &utils) {
        ss << "RM:  " << utils.rmUtility << "\n";
        ss << "AVG: " << utils.avgUtility << "\n";
        return ss;
    }
};

void calcRMProbs(const vector<double> &regrets, ProbDistribution *pProbs, double epsilonUniform);
void calcAvgProbs(const vector<double> &acc, ProbDistribution *pProbs);

inline void calcRMProbs(const vector<double> &regrets, ProbDistribution *pProbs) {
    calcRMProbs(regrets, pProbs, 0);
}
inline ProbDistribution calcRMProbs(const vector<double> &regrets) {
    auto rmProbs = vector<double>(regrets.size());
    calcRMProbs(regrets, &rmProbs);
    return rmProbs;
}
inline ProbDistribution calcAvgProbs(const vector<double> &acc) {
    auto avgProbs = vector<double>(acc.size());
    calcAvgProbs(acc, &avgProbs);
    return avgProbs;
}

/**
 * Calculate expected utilities under RM / avg strategy for specified node
 */
ExpectedUtility calcExpectedUtility(CFRData &cache,
                                    const shared_ptr<EFGNode> &node,
                                    Player pl);

}  // namespace GTLib2

#endif  // ALGORITHMS_CFR_H_
