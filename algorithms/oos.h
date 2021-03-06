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

#include "base/gadget.h"
#include "base/random.h"
#include "algorithms/cfr.h"
#include "algorithms/strategy.h"

#include "algorithms/common.h"


namespace GTLib2::algorithms {

class OOSData: public virtual CFRData, public virtual PublicStateCache {
 public:
    inline explicit OOSData(const Domain &domain) :
        EFGCache(domain), InfosetCache(domain),
        CFRData(domain, HistoriesUpdating), PublicStateCache(domain) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createOOSBaselineData(n); });
        this->createOOSBaselineData(getRootNode());
    }

    inline OOSData(const OOSData &other) :
        EFGCache(other),
        InfosetCache(other),
        CFRData(other),
        PublicStateCache(other) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createOOSBaselineData(n); });
        baselineValues = other.baselineValues;
        nodeValues = other.nodeValues;
    }

    void reset() override {
        CFRData::reset();
        for (auto &[node, data] : baselineValues) data.reset();
        for (auto &[node, data] : nodeValues) data.reset();
    }


    inline void clear() override {
        CFRData::clear();
        PublicStateCache::clear();
        baselineValues.clear();
        nodeValues.clear();
        this->createOOSBaselineData(getRootNode());
    }

    inline double
    getBaselineFor(const shared_ptr<EFGNode> &h, ActionId , Player exploringPl) {
        return baselineValues.at(h).value() * (exploringPl == Player(0) ? 1 : -1);
    }

    // always stored for Player(0)
    struct Fraction {
        double nominator = 0.;
        double denominator = 1.;
        inline double value() const {
            double v = nominator / denominator;
            assert(!isnan(v));
            assert(!isinf(v));
            return v;
        }
        inline void reset() {
            nominator = 0.;
            denominator = 1.;
        }
    };

    unordered_map<shared_ptr<EFGNode>, Fraction> baselineValues;
    unordered_map<shared_ptr<EFGNode>, Fraction> nodeValues;

    PublicStateSummary getPublicStateSummary(const shared_ptr<PublicState> &ps) const {
        // todo: make more efficient
        const auto &histories = getNodesForPubState(ps);
        vector<shared_ptr<EFGNode>> topmostHistories_;
        for (const auto &a : histories) {
            bool hasTopperHistory = false;
            for (const auto &b : histories) {
                if (isExtension(b->getHistory(), a->getHistory())) {
                    hasTopperHistory = true;
                    break;
                }
            }

            if (!hasTopperHistory) topmostHistories_.push_back(a);
        }
        std::sort(topmostHistories_.begin(), topmostHistories_.end(), nodeCompare<EFGNode>);


        vector<array<double, 3>> topmostHistoriesReachProbs_;
        topmostHistoriesReachProbs_.reserve(topmostHistories_.size());
        for (const auto &h : topmostHistories_) {
            topmostHistoriesReachProbs_.emplace_back(calcReachProbs(h, this));
        }

        // todo: maybe we want to make separate storage of cfv values from baselines?
        vector<double> topmostValues;
        topmostValues.reserve(topmostHistories_.size());
        for (const auto &h : topmostHistories_) {
            topmostValues.emplace_back(nodeValues.at(h).value());
        }

        return PublicStateSummary(ps, topmostHistories_,
                                  topmostHistoriesReachProbs_, topmostValues);
    }

 private:
    void createOOSBaselineData(const shared_ptr<EFGNode> &node) {
        if (node->type_ == TerminalNode) {
            // todo: should we create this for NoAvgValueCalculation?
            baselineValues.emplace(node, Fraction{node->getUtilities()[0], 1.});
            nodeValues.emplace(node, Fraction{node->getUtilities()[0], 1.});
        } else {
            baselineValues.emplace(node, Fraction());
            nodeValues.emplace(node, Fraction());
        }
    }

};


struct OOSSettings: AlgConfig {
    enum SamplingBlock { OutcomeSampling, ExternalSampling };
    enum Targeting { NoTargeting, InfosetTargeting, PublicStateTargeting };
    enum PlayStrategy { PlayUsingAvgStrategy, PlayUsingRMStrategy };
    enum SamplingScheme { EpsilonOnPolicySampling, UniformSampling };
    enum AvgStrategyComputation { StochasticallyWeightedAveraging, LazyWeightedAveraging };
    enum NodeAvgValueCalculation {
        // i.e. put zeros
        NoAvgValueCalculation,
        // for the avg strategy, i.e. u(h) under avg strategy specified by the avgStratAccumulator
        OracleExactCalculation,
        // Ordinary average without weights
        WeightedTime,
        // Acting + Chance
        WeightedActingPlayer,
        // Acting + opponent + chance
        WeightedAllPlayers,
    };

    SamplingBlock samplingBlock = OutcomeSampling;
    AccumulatorWeighting accumulatorWeighting = UniformAccWeighting;
    RegretMatching regretMatching = RegretMatchingNormal;
    Targeting targeting = InfosetTargeting;
    PlayStrategy playStrategy = PlayUsingAvgStrategy;
    SamplingScheme samplingScheme = EpsilonOnPolicySampling;
    AvgStrategyComputation avgStrategyComputation = StochasticallyWeightedAveraging;
    NodeAvgValueCalculation baseline = NoAvgValueCalculation;

    double exploration = 0.6;
    double targetBiasing = 0.0;
    double approxRegretMatching = 0.001;

    /**
     * Number of iterations that should be run at each invokation of runPlayIteration.
     */
    unsigned long batchSize = 1;

    unsigned long seed = 0;

    //@formatter:off
    void update(const string &k, const string &v) override {
        if(k == "samplingBlock"          && v == "OutcomeSampling")                 samplingBlock        = OutcomeSampling;                   else
        if(k == "samplingBlock"          && v == "ExternalSampling")                samplingBlock        = ExternalSampling;                  else
        if(k == "accumulatorWeighting"   && v == "UniformAccWeighting")             accumulatorWeighting = UniformAccWeighting;               else
        if(k == "accumulatorWeighting"   && v == "LinearAccWeighting")              accumulatorWeighting = LinearAccWeighting;                else
        if(k == "regretMatching"         && v == "RegretMatchingNormal")            regretMatching       = RegretMatchingNormal;              else
        if(k == "regretMatching"         && v == "RegretMatchingPlus")              regretMatching       = RegretMatchingPlus;                else
        if(k == "targeting"              && v == "NoTargeting")                     targeting            = NoTargeting;                       else
        if(k == "targeting"              && v == "InfosetTargeting")                targeting            = InfosetTargeting;                  else
        if(k == "targeting"              && v == "PublicStateTargeting")            targeting            = PublicStateTargeting;              else
        if(k == "playStrategy"           && v == "PlayUsingAvgStrategy")            playStrategy         = PlayUsingAvgStrategy;              else
        if(k == "playStrategy"           && v == "PlayUsingRMStrategy")             playStrategy         = PlayUsingRMStrategy;               else
        if(k == "samplingScheme"         && v == "EpsilonOnPolicySampling")         samplingScheme       = EpsilonOnPolicySampling;           else
        if(k == "samplingScheme"         && v == "UniformSampling")                 samplingScheme       = UniformSampling;                   else
        if(k == "avgStrategyComputation" && v == "StochasticallyWeightedAveraging") avgStrategyComputation = StochasticallyWeightedAveraging; else
        if(k == "avgStrategyComputation" && v == "LazyWeightedAveraging")           avgStrategyComputation = LazyWeightedAveraging;           else
        if(k == "baseline"               && v == "NoAvgValueCalculation")           baseline             = NoAvgValueCalculation;             else
        if(k == "baseline"               && v == "OracleExactCalculation")          baseline             = OracleExactCalculation;            else
        if(k == "baseline"               && v == "WeightedActingPlayer")            baseline             = WeightedActingPlayer;              else
        if(k == "baseline"               && v == "WeightedAllPlayers")              baseline             = WeightedAllPlayers;                else
        if(k == "baseline"               && v == "WeightedTime")                    baseline             = WeightedTime;                      else
        if(k == "exploration")           exploration           = std::stod(v); else
        if(k == "targetBiasing")         targetBiasing         = std::stod(v); else
        if(k == "approxRegretMatching")  approxRegretMatching  = std::stod(v); else
        if(k == "batchSize")             batchSize             = std::stoi(v); else
        if(k == "seed")                  seed                  = std::stoi(v); else
        AlgConfig::update(k, v);
    }

    inline string toString() const override {
        std::stringstream ss;
        ss << "; OOS" << endl;
        if(samplingBlock          == OutcomeSampling)                 ss << "samplingBlock          = OutcomeSampling"                 << endl;
        if(samplingBlock          == ExternalSampling )               ss << "samplingBlock          = ExternalSampling"                << endl;
        if(accumulatorWeighting   == UniformAccWeighting)             ss << "accumulatorWeighting   = UniformAccWeighting"             << endl;
        if(accumulatorWeighting   == LinearAccWeighting)              ss << "accumulatorWeighting   = LinearAccWeighting"              << endl;
        if(regretMatching         == RegretMatchingNormal)            ss << "regretMatching         = RegretMatchingNormal"            << endl;
        if(regretMatching         == RegretMatchingPlus)              ss << "regretMatching         = RegretMatchingPlus"              << endl;
        if(targeting              == NoTargeting)                     ss << "targeting              = NoTargeting"                     << endl;
        if(targeting              == InfosetTargeting)                ss << "targeting              = InfosetTargeting"                << endl;
        if(targeting              == PublicStateTargeting)            ss << "targeting              = PublicStateTargeting"            << endl;
        if(playStrategy           == PlayUsingAvgStrategy)            ss << "playStrategy           = PlayUsingAvgStrategy"            << endl;
        if(playStrategy           == PlayUsingRMStrategy)             ss << "playStrategy           = PlayUsingRMStrategy"             << endl;
        if(samplingScheme         == EpsilonOnPolicySampling)         ss << "samplingScheme         = EpsilonOnPolicySampling"         << endl;
        if(samplingScheme         == UniformSampling)                 ss << "samplingScheme         = UniformSampling"                 << endl;
        if(avgStrategyComputation == StochasticallyWeightedAveraging) ss << "avgStrategyComputation = StochasticallyWeightedAveraging" << endl;
        if(avgStrategyComputation == LazyWeightedAveraging)           ss << "avgStrategyComputation = LazyWeightedAveraging"           << endl;
        if(baseline               == NoAvgValueCalculation)           ss << "baseline               = NoAvgValueCalculation"           << endl;
        if(baseline               == OracleExactCalculation)          ss << "baseline               = OracleExactCalculation"          << endl;
        if(baseline               == WeightedActingPlayer)            ss << "baseline               = WeightedActingPlayer"            << endl;
        if(baseline               == WeightedAllPlayers)              ss << "baseline               = WeightedAllPlayers"              << endl;
        if(baseline               == WeightedTime)                    ss << "baseline               = WeightedTime"                    << endl;
        ss << "exploration            = " << exploration           << endl;
        ss << "targetBiasing          = " << targetBiasing         << endl;
        ss << "approxRegretMatching   = " << approxRegretMatching  << endl;
        ss << "batchSize              = " << batchSize             << endl;
        ss << "seed                   = " << seed                  << endl;
        return ss.str();
    }
    //@formatter:on
};

/**
 * Runtime statistics for online algorithms
 */
struct OnlineStats {
    unsigned int rootVisits = 0;
    unsigned int nodesVisits = 0;
    unsigned int terminalsVisits = 0;
    unsigned int infosetVisits = 0;
    unsigned int pubStateVisits = 0;

    void reset() {
        rootVisits = 0;
        nodesVisits = 0;
        terminalsVisits = 0;
        infosetVisits = 0;
        pubStateVisits = 0;
    }
};


class Targetor {
 public:
    explicit inline Targetor(OOSData &cache, OOSSettings cfg) : cache_(cache), cfg_(move(cfg)) {};

    bool updateCurrentPosition(const optional<shared_ptr<AOH>> &infoset,
                               const optional<shared_ptr<PublicState>> &pubState);
    bool isAllowedAction(const shared_ptr<EFGNode> &h, const shared_ptr<Action> &action);

    /**
     * Retrieve the value of weighting factor according to equation (3) in [1]:
     *
     * 1/w = (1-targetBiasing) + targetBiasing * ( sum_of_unbiased_reach_probs / sum_of_biased_reach_probs )
     *
     * [1] Online monte carlo counterfactual regret minimization for search in imperfect information games
     *     Lisý, Viliam and Lanctot, Marc and Bowling, Michael
     *
     * @return the value w
     */
    inline double compensateTargeting() const { return weightingFactor_; }

 private:
    double weightingFactor_ = 1.0;
    shared_ptr<AOH> currentInfoset_;
    shared_ptr<PublicState> currentPubState_;

    const OOSData &cache_;
    const OOSSettings cfg_;

    pair<double, double>
    updateWeighting(const shared_ptr<EFGNode> &dist, double bs_h_all, double us_h_all);
};

/**
 * unsigned int ai    action index
 * double rm_ha_all   probability of taking this action (according to RM)
 * double u_h         baseline-augmented estimate of expected utility for current history
 * double u_x         baseline-augmented estimate of expected utility for next history,
 *                       if we go there with 100% probability from current history, i.e. u_b(h.a)
 */
typedef tuple<unsigned int, double, double, double> PlayerNodeOutcome;

/**
 * Online Outcome Sampling algorithm
 *
 * When this algorithm is instantiated with targetBiasing = 0. it becomes  MCCFR with incremental
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
 *    ha - at the current history playing action a, i.e. resulting to child history h.a
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
    inline explicit OOSAlgorithm(const Domain &domain, Player playingPlayer,
                                 OOSData &cache, OOSSettings cfg)
        : GamePlayingAlgorithm(domain, playingPlayer),
          cache_(cache),
          cfg_(move(cfg)),
          targetor_(Targetor(cache_, cfg_)) {
        generator_ = std::mt19937(cfg_.seed);
        dist_ = std::uniform_real_distribution<double>(0.0, 1.0);
    };

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

    const OOSData &getCache() { return cache_; }
    const OOSSettings &getSettings() { return cfg_; }

 protected:
    virtual void
    rootIteration(const shared_ptr<EFGNode> &rootNode, double compensation, Player exploringPl);
    void rootIteration(double compensation, Player exploringPl);

    /**
     * The main function for OOS iteration.
     *
     * Utilities are always for the current exploring player.
     *
     * @param h         current node
     * @param rm_h_pl   reach prob of the searching player to the current node using RM strategy
     * @param rm_h_opp  reach prob of the opponent  to the current node using RM strategy
     * @param bs_h_all  reach prob of all players to the current node using biased sampling strategy
     * @param us_h_all  reach prob of all players to the current node using unbiased sampling strategy
     * @param exploringPl the exploring player for this iteration
     * @return expected baseline-augmented utility of current node for the exploring player
     */
    double iteration(const shared_ptr<EFGNode> &h,
                     double rm_h_pl, double rm_h_opp,
                     double bs_h_all, double us_h_all,
                     double us_h_cn,
                     Player exploringPl);
    virtual double handleTerminalNode(const shared_ptr<EFGNode> &h,
                                      double bs_h_all, double us_h_all,
                                      Player exploringPl);
    virtual double handleChanceNode(const shared_ptr<EFGNode> &h,
                                    double rm_h_pl, double rm_h_opp,
                                    double bs_h_all, double us_h_all,
                                    double us_h_cn,
                                    Player exploringPl);
    virtual double handlePlayerNode(const shared_ptr<EFGNode> &h,
                                    double rm_h_pl, double rm_h_opp,
                                    double bs_h_all, double us_h_all,
                                    double us_h_cn,
                                    Player exploringPl);

    // Simulate an outcome from specified node
    PlayerNodeOutcome incrementallyBuildTree(const shared_ptr<EFGNode> &h,
                                             const vector<shared_ptr<Action>> &actions,
                                             double s_h_all,
                                             Player exploringPl);
    virtual PlayerNodeOutcome sampleExistingTree(const shared_ptr<EFGNode> &h,
                                                 const vector<shared_ptr<Action>> &actions,
                                                 double rm_h_pl,
                                                 double rm_h_opp,
                                                 double bs_h_all,
                                                 double us_h_all,
                                                 double us_h_cn,
                                                 CFRData::InfosetData &data,
                                                 const shared_ptr<AOH> &infoset,
                                                 Player exploringPl);

    // Internally mutates pBiasedProbs for subsequent use.
    // Returns biasApplicableActions and sum over biased probs
    pair<int, double> calcBiasing(const shared_ptr<EFGNode> &h,
                                  const vector<shared_ptr<Action>> &actions,
                                  double bs_h_all);

    virtual pair<ActionId, RandomLeafOutcome> selectLeaf(const shared_ptr<EFGNode> &h,
                                                         const vector<shared_ptr<Action>> &actions);
    virtual ActionId selectChanceAction(const shared_ptr<EFGNode> &h, double bsum);
    virtual ActionId selectExploringPlayerAction(const shared_ptr<EFGNode> &h,
                                                 int biasApplicableActions, double bsum);
    virtual ActionId selectNonExploringPlayerAction(const shared_ptr<EFGNode> &h, double bsum);

    virtual void updateEFGNodeExpectedValue(Player exploringPl, const shared_ptr<EFGNode> &h,
                                            double u_h, double rm_h_pl, double rm_h_opp,
                                            double us_h_cn, double s_h_all);
    pair<double, double> calcEFGNodeUpdate(double u_h, bool isExploringPlayer,
                                           double rm_h_pl, double rm_h_opp, double us_h_cn,
                                           OOSSettings::NodeAvgValueCalculation setting);
    virtual void updateInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                                      CFRData::InfosetData &data, unsigned int ai,
                                      double u_x, double u_h, double w);
    void updateInfosetAcc(const shared_ptr<EFGNode> &h,
                          CFRData::InfosetData &data,
                          double importanceSamplingRatio);

    inline double bias(double biased, double nonBiased) const {
        return cfg_.targetBiasing * biased + (1 - cfg_.targetBiasing) * nonBiased;
    }
    inline double explore(double exploring, double nonExploring) const {
        return cfg_.exploration * exploring + (1 - cfg_.exploration) * nonExploring;
    }

    OOSData &cache_;
    const OOSSettings cfg_;
    std::mt19937 generator_;
    std::uniform_real_distribution<double> dist_;
    Targetor targetor_;

    OnlineStats stats_;

    // chance probs are used for chance player, since he doesn't have RM defined
    double rm_zh_all_ = -1;
    // probability of sampling leaf
    double s_z_all_ = -1;
    // current leaf value
    double u_z_ = 0.0;

    constexpr static int OOS_MAX_ACTIONS = 1000;

    // Careful! Following are mutable data structures.
    // Values will change throughout the traversal of the tree, based on current state!!!

    // unbiased probabilties of current strategy, i.e. RM current strategy or chance
    ProbDistribution usProbs_ = ProbDistribution(OOS_MAX_ACTIONS);
    // Array of actual biased probabilities, but pBiasedProbs_ can point to usProbs_
    // to prevent unnecessary copying of arrays
    ProbDistribution tmpProbs_ = ProbDistribution(OOS_MAX_ACTIONS);
    ProbDistribution *pBiasedProbs_ = &tmpProbs_;

    // should current iteration make a biased sample? (with probability of targetBiasing)
    bool isBiasedIteration_ = false;

    // are we deeper in the tree, "below" target IS?
    // If yes, we dont need to bias samples anymore, any sampling strategy is fine
    bool isBelowTargetIS_ = false;

    optional<shared_ptr<AOH>> playInfoset_ = nullopt;
    optional<shared_ptr<PublicState>> playPublicState_ = nullopt;
};

}  // namespace GTLib2


#endif  // ALGORITHMS_OOS_H_
