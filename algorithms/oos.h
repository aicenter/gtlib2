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


#ifndef ALGORITHMS_OOS_H_
#define ALGORITHMS_OOS_H_

#include "base/random.h"
#include "algorithms/cfr.h"
#include "algorithms/tree.h"
#include "algorithms/common.h"


namespace GTLib2::algorithms {

class OOSData: public virtual CFRData, public virtual PublicStateCache {
 public:
    inline explicit OOSData(const Domain &domain) :
        EFGCache(domain), InfosetCache(domain),
        CFRData(domain, HistoriesUpdating), PublicStateCache(domain) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createOOSBaselineData(n); });
    }

    inline double getBaselineFor(const shared_ptr<EFGNode> h, ActionId action, Player exploringPl) {
        return baselineValues_.at(h).value() * (exploringPl == Player(0) ? 1 : -1);
    }

    struct Baseline {
        double nominator = 0.;
        double denominator = 1.;
        inline double value() const { return nominator / denominator; }
    };

    unordered_map<shared_ptr<EFGNode>, Baseline> baselineValues_;

 private:
    void createOOSBaselineData(const shared_ptr<EFGNode> &node) {
        baselineValues_.emplace(make_pair(node, Baseline()));
    }

};


struct OOSSettings {
    enum SamplingBlock { OutcomeSampling, ExternalSampling };
    enum AccumulatorWeighting { UniformAccWeighting, LinearAccWeighting, XLogXAccWeighting };
    enum RegretMatching { RegretMatchingNormal, RegretMatchingPlus };
    enum Targeting { InfosetTargeting, PublicStateTargeting };
    enum PlayStrategy { PlayUsingAvgStrategy, PlayUsingRMStrategy };
    enum SamplingScheme { EpsilonOnPolicySampling, UniformSampling };
    enum AvgStrategyComputation { StochasticallyWeightedAveraging, LazyWeightedAveraging };
    // enum NodeAvgValueWeighting { WeightingActingPlayer, WeightingAllPlayers, WeightingTime };
    enum Baseline { // equivalent to NodeAvgValueWeighting
        NoBaseline, OracleBaseline,
        WeightedActingPlayerBaseline, WeightedAllPlayerBaseline, WeightedTimeBaseline
    };

    SamplingBlock samplingBlock = OutcomeSampling;
    AccumulatorWeighting accumulatorWeighting = UniformAccWeighting;
    RegretMatching regretMatching = RegretMatchingNormal;
    Targeting targeting = InfosetTargeting;
    PlayStrategy playStrategy = PlayUsingAvgStrategy;
    // NodeAvgValueWeighting nodeAvgValueWeighting = WeightingActingPlayer;
    SamplingScheme samplingScheme = EpsilonOnPolicySampling;
    AvgStrategyComputation avgStrategyComputation = StochasticallyWeightedAveraging;
    Baseline baseline = NoBaseline;

    double exploration = 0.6;
    double targetBiasing = 0.0;
    double gadgetExploration = 0.0;
    double gadgetInfosetBiasing = 0.0;
    double approxRegretMatching = 0.001;

    /**
     * Number of iterations that should be run at each invokation of runPlayIteration.
     */
    unsigned long batchSize = 1;

    unsigned long seed = 0;
};

/**
 * Runtime statistics for online algorithms
 */
struct OnlineStats {
    int nodesVisits = 0;
    int terminalsVisits = 0;
    int infosetVisits = 0;
    int pubStateVisits = 0;

    void reset() {
        nodesVisits = 0;
        terminalsVisits = 0;
        infosetVisits = 0;
        pubStateVisits = 0;
    }
};


class Targetor {
 public:
    explicit inline Targetor(OOSData &cache, OOSSettings::Targeting type, double targetBiasing) :
        cache_(cache), targeting_(type), targetBiasing_(targetBiasing) {};

    void updateCurrentPosition(const optional<shared_ptr<AOH>> &infoset,
                               const optional<shared_ptr<EFGPublicState>> &pubState);
    bool isAllowedAction(const shared_ptr<EFGNode> h, const shared_ptr<Action> action);

    /**
     * Retrieve the value of weighting factor according to equation (3) in [1]:
     *
     * 1/w = (1-delta) + delta * ( sum_of_unbiased_reach_probs / sum_of_biased_reach_probs )
     *
     * [1] Online monte carlo counterfactual regret minimization for search in imperfect information games
     *     Lisý, Viliam and Lanctot, Marc and Bowling, Michael
     *
     * @return the value w
     */
    inline double compensateTargetting() {
        return weightingFactor_;
    }

 private:
    double weightingFactor_ = 1.0;

    shared_ptr<AOH> currentInfoset_;
    shared_ptr<EFGPublicState> currentPubState_;

    double bsSum_ = 0.0, usSum_ = 0.0;

    OOSSettings::Targeting targeting_;
    OOSData &cache_;
    double targetBiasing_;

    void updateWeighting(const shared_ptr<EFGNode> &dist, double bs_h_all, double us_h_all);
};

/**
 * Online Outcome Sampling algorithm
 *
 * When this algorithm is instantiated with delta = 0. it becomes  MCCFR with incremental
 * tree building. You can prebuild the tree yourself, and you get MCCFR.
 *
 * There are some important conventions in variable naming: X_Y_Z
 *
 * X corresponds to strategy
 *    rm_: regret matching (or also called current) strategy
 *    avg_: average strategy
 *    bs_: biased sampling strategy
 *    us_: unbiased sampling strategy
 *    unif_: uniform strategy
 *    s_: sampling strategy
 *
 * Y corresponds to some node
 *    h - current history
 *    z - leaf node
 *    zh - from current history to the leaf, i.e. z|h
 *    zha - from current history and playing action a with 100% prob to the leaf, i.e. z|h.a
 *    ha - at the current history playing action a, i.e. h.a
 *
 * Z corresponds to player
 *    pl - current player
 *    opp - opponent player (without chance)
 *    cn - chance player
 *    both - current and opponent player (without chance)
 *    all - all the players (including chance)
 *
 * Note that in implementation opponent always means only the other player (never including chance)!
 *
 * This algorithm is based on following works:
 *
 * - Monte Carlo sampling for regret minimization in extensive games,
 *   Lanctot, Marc and Waugh, Kevin and Zinkevich, Martin and Bowling, Michael
 * - Online monte carlo counterfactual regret minimization for search in imperfect information games
 *   Lisý, Viliam and Lanctot, Marc and Bowling, Michael
 * - Monte Carlo Continual Resolving for Online Strategy Computation in Imperfect Information Games
 *   Sustr, Michal and Kovarik, Vojtech and Lisy, Viliam
 * - Variance reduction in monte carlo counterfactual regret minimization (vr-mccfr) for extensive form games using baselines.
 *   Schmid, Martin and Burch, Neil and Lanctot, Marc and Moravcik, Matej and Kadlec, Rudolf and Bowling, Michael
 */
class OOSAlgorithm: public GamePlayingAlgorithm {

 public:
    inline explicit OOSAlgorithm(const Domain &domain,
                                 Player playingPlayer,
                                 OOSData &cache,
                                 OOSSettings cfg)
        : GamePlayingAlgorithm(domain, playingPlayer),
          cache_(cache),
          cfg_(cfg),
          targetor_(Targetor(cache_, cfg_.targeting, cfg_.targetBiasing)) {
        generator_ = std::mt19937(cfg_.seed);
        dist_ = std::uniform_real_distribution<double>(0.0, 1.0);
    };

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

 private:
    /**
     * The main function for OOS iteration.
     *
     * Utilities are always for the current exploring player.
     *
     * @param n         current node
     * @param rm_h_pl   reach prob of the searching player to the current node using RM strategy
     * @param rm_h_opp  reach prob of the opponent  to the current node using RM strategy
     * @param rm_h_cn   reach prob of chance player to the current node using RM strategy
     * @param bs_h_all  reach prob of all players to the current node using biased sampling strategy
     * @param us_h_all  reach prob of all players to the current node using unbiased sampling strategy
     * @param exploringPl the exploring player for this iteration
     * @return expected baseline-augmented utility of current node for the exploring player
     */
    double iteration(const shared_ptr<EFGNode> &h, double rm_h_pl, double rm_h_opp,
                     double rm_h_cn, double bs_h_all, double us_h_all, Player exploringPl);

    pair<int, double> calcBiasing(const shared_ptr<EFGNode> &h, const shared_ptr<AOH> &infoset,
                                  double bs_h_all, int numActions);

    pair<int, double> updateBiasing(const shared_ptr<EFGNode> &h);

    pair<int, double> selectChanceAction(const shared_ptr<EFGNode> &h);

    pair<int, double> selectExploringPlayerAction(int numActions, int biasApplicableActions,
                                                  double bsum);

    pair<int, double> selectNonExploringPlayerAction(const shared_ptr<EFGNode> &h, double bsum);

    void updateHistoryExpectedValue(Player exploringPl, const shared_ptr<EFGNode> &h,
                                    double u_h,
                                    double rm_h_pl, double rm_h_opp, double rm_h_cn,
                                    double s_h_all);
    void updateInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                              CFRData::InfosetData &data,
                              int ai, double pai,
                              double u_z, double u_x, double u_h,
                              double rm_h_cn, double rm_h_opp, double rm_zha_all, double s_h_all);

    inline double bias(double biased, double nonBiased) const {
        return cfg_.targetBiasing * biased + (1 - cfg_.targetBiasing) * nonBiased;
    }
    inline double explore(double exploring, double nonExploring) const {
        return cfg_.exploration * exploring + (1 - cfg_.exploration) * nonExploring;
    }

    OOSData cache_;
    OOSSettings cfg_;
    std::mt19937 generator_;
    std::uniform_real_distribution<double> dist_;
    Targetor targetor_;

    OnlineStats stats_;

    double rm_zh_all_ = -1;
    double s_z_all_ = -1;
    double u_z_;

    #define OOS_MAX_ACTIONS 1000

    ProbDistribution rmProbs_ = ProbDistribution(OOS_MAX_ACTIONS);
    // Array of actual biased probabilities, but pBiasedProbs_ can point to rmProbs_
    // to prevent unnecessary copying of arrays
    ProbDistribution tmpProbs_ = ProbDistribution(OOS_MAX_ACTIONS);
    ProbDistribution *pBiasedProbs_ = &tmpProbs_;

    // should current iteration make a biased sample? (with prob. delta)
    bool isBiasedIteration_ = false;

    // are we deeper in the tree, "below" target IS?
    // If yes, we dont need to bias samples anymore, any sampling strategy is fine
    bool isBelowTargetIS_ = false;

    optional<shared_ptr<AOH>> playInfoset_ = nullopt;
    optional<shared_ptr<EFGPublicState>> playPublicState_ = nullopt;

    double normalizingUtils = 1.; // todo: remove
};

}  // namespace GTLib2


#endif  // ALGORITHMS_OOS_H_
