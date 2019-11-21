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

#include "base/algorithm.h"

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


struct CFRSettings: AlgConfig {
    // todo: CFR+
    AccumulatorWeighting accumulatorWeighting = UniformAccWeighting;
    RegretMatching regretMatching = RegretMatchingNormal;
    CFRUpdating cfrUpdating = HistoriesUpdating;

    //@formatter:off
    void update(const string &k, const string &v) override {
        if(k == "accumulatorWeighting" && v == "UniformAccWeighting")  accumulatorWeighting = UniformAccWeighting;  else
        if(k == "accumulatorWeighting" && v == "LinearAccWeighting")  accumulatorWeighting = LinearAccWeighting;   else
        if(k == "regretMatching"       && v == "RegretMatchingNormal") regretMatching       = RegretMatchingNormal; else
        if(k == "regretMatching"       && v == "RegretMatchingPlus")  regretMatching       = RegretMatchingPlus;   else
        if(k == "cfrUpdating"          && v == "HistoriesUpdating")    cfrUpdating          = HistoriesUpdating;    else
        if(k == "cfrUpdating"          && v == "InfosetsUpdating")    cfrUpdating          = InfosetsUpdating;     else
        AlgConfig::update(k, v);
    }

    inline string toString() const override {
        std::stringstream ss;
        ss << "; CFR" << endl;
        if(accumulatorWeighting == UniformAccWeighting)   ss << "accumulatorWeighting = UniformAccWeighting"  << endl;
        if(accumulatorWeighting == LinearAccWeighting )   ss << "accumulatorWeighting = LinearAccWeighting"   << endl;
        if(regretMatching       == RegretMatchingNormal)  ss << "regretMatching       = RegretMatchingNormal" << endl;
        if(regretMatching       == RegretMatchingPlus )   ss << "regretMatching       = RegretMatchingPlus"   << endl;
        if(cfrUpdating          == HistoriesUpdating)     ss << "cfrUpdating          = HistoriesUpdating"    << endl;
        if(cfrUpdating          == InfosetsUpdating )     ss << "cfrUpdating          = InfosetsUpdating"     << endl;
        return ss.str();
    }
    //@formatter:on
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
 * Container for regrets and average strategy accumulators
 */
class CFRData: public virtual InfosetCache,
               public StrategyCache {

 public:
    inline explicit CFRData(const Domain &domain) : CFRData(domain, HistoriesUpdating) {}

    inline explicit CFRData(const Domain &domain, CFRUpdating updatingPolicy) :
        EFGCache(domain),
        InfosetCache(domain),
        updatingPolicy_(updatingPolicy) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createCFRInfosetData(n); });
        this->createCFRInfosetData(getRootNode());
    }

    inline CFRData(const CFRData &other) :
        EFGCache(other),
        InfosetCache(other) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createCFRInfosetData(n); });
        infosetData = other.infosetData;
        updatingPolicy_ = other.updatingPolicy_;
    }

    void reset() override {
        for(auto &[aoh, data] : infosetData) data.reset();
    }

    inline void clear() override {
        InfosetCache::clear();
        infosetData.clear();
        this->createCFRInfosetData(getRootNode());
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

        void reset() {
            std::fill(regrets.begin(), regrets.end(), 0.);
            std::fill(avgStratAccumulator.begin(), avgStratAccumulator.end(), 0.);
            std::fill(regretUpdates.begin(), regretUpdates.end(), 0.);
            numUpdates = 0;
        }
    };

    unordered_map<shared_ptr<AOH>, InfosetData> infosetData;

    inline optional<ProbDistribution> strategyFor(const shared_ptr<AOH> &currentInfoset) const override {
        if (infosetData.find(currentInfoset) == infosetData.end()) return nullopt;
        return calcAvgProbs(infosetData.at(currentInfoset).avgStratAccumulator);
    }

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
                 Player playingPlayer,
                 CFRData &cache,
                 CFRSettings settings);
    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

    inline double runIteration(const Player updatingPl) {
        return runIteration(cache_.getRootNode(), array<double, 3>{1., 1., 1.}, updatingPl);
    }

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

/**
 * Calculate expected utilities under RM / avg strategy for specified node
 */
ExpectedUtility calcExpectedUtility(CFRData &cache,
                                    const shared_ptr<EFGNode> &node,
                                    Player pl);

}  // namespace GTLib2

#endif  // ALGORITHMS_CFR_H_
