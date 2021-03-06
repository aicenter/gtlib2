#include <utility>

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


#ifndef GTLIB2_MCCR_H
#define GTLIB2_MCCR_H

#include "algorithms/oos.h"
#include "algorithms/continualResolving.h"

namespace GTLib2::algorithms {

class MCCRAlgorithm;

struct MCCRSettings: OOSSettings {
    enum RetentionPolicy { ResetData, NaiveKeepData, ReweighKeepData };

    RetentionPolicy retentionPolicy = ResetData;
    NodeAvgValueCalculation terminateCFVs = WeightedAllPlayers;

    //@formatter:off
    inline void update(const string  &k, const string &v) override {
        if(k == "retentionPolicy" && v == "ResetData")              retentionPolicy = ResetData;              else
        if(k == "retentionPolicy" && v == "NaiveKeepData")          retentionPolicy = NaiveKeepData;          else
        if(k == "retentionPolicy" && v == "ReweighKeepData")        retentionPolicy = ReweighKeepData;        else
        if(k == "terminateCFVs"   && v == "NoAvgValueCalculation")  terminateCFVs   = NoAvgValueCalculation;  else
        if(k == "terminateCFVs"   && v == "OracleExactCalculation") terminateCFVs   = OracleExactCalculation; else
        if(k == "terminateCFVs"   && v == "WeightedActingPlayer")   terminateCFVs   = WeightedActingPlayer;   else
        if(k == "terminateCFVs"   && v == "WeightedAllPlayers")     terminateCFVs   = WeightedAllPlayers;     else
        if(k == "terminateCFVs"   && v == "WeightedTime")           terminateCFVs   = WeightedTime;           else
        OOSSettings::update(k, v);
    }

    inline string toString() const override {
        std::stringstream ss;
        ss << "; MCCR" << endl;
        if(retentionPolicy == ResetData)                ss << "retentionPolicy        = ResetData"               << endl;
        if(retentionPolicy == NaiveKeepData)            ss << "retentionPolicy        = NaiveKeepData"           << endl;
        if(retentionPolicy == ReweighKeepData)          ss << "retentionPolicy        = ReweighKeepData"         << endl;
        if(terminateCFVs   == NoAvgValueCalculation)    ss << "terminateCFVs          = NoAvgValueCalculation"   << endl;
        if(terminateCFVs   == OracleExactCalculation)   ss << "terminateCFVs          = OracleExactCalculation"  << endl;
        if(terminateCFVs   == WeightedActingPlayer)     ss << "terminateCFVs          = WeightedActingPlayer"    << endl;
        if(terminateCFVs   == WeightedAllPlayers)       ss << "terminateCFVs          = WeightedAllPlayers"      << endl;
        if(terminateCFVs   == WeightedTime)             ss << "terminateCFVs          = WeightedTime"            << endl;
        ss << OOSSettings::toString();
        return ss.str();
    }
    //@formatter:on
};

typedef OOSData MCCRData;

class MCCRResolver: public OOSAlgorithm {
 public:
    MCCRResolver(const Domain &domain,
                 Player playingPlayer,
                 MCCRData &cache,
                 const MCCRSettings &cfg)
        : OOSAlgorithm(domain, playingPlayer, cache, cfg),
          mccr_cfg_(cfg),
          keep_(cache) {

        if (mccr_cfg_.terminateCFVs == OOSSettings::NoAvgValueCalculation) {
            LOG_ERROR("There cannot be NoAvgValueCalculation for MCCR! "
                      "It needs to save values for resolving.")
            exit(1);
        }
    }
    virtual ~MCCRResolver() = default;

 protected:
    double handleChanceNode(const shared_ptr<EFGNode> &h, double rm_h_pl, double rm_h_opp,
                            double bs_h_all, double us_h_all, double us_h_cn,
                            Player exploringPl) override;
    double handlePlayerNode(const shared_ptr<EFGNode> &h, double rm_h_pl, double rm_h_opp,
                            double bs_h_all, double us_h_all, double us_h_cn,
                            Player exploringPl) override;
    double handleTerminalNode(const shared_ptr<EFGNode> &h,
                              double bs_h_all, double us_h_all,
                              Player exploringPl) override;

    void updateGadgetInfosetRegrets(const shared_ptr<EFGNode> &h,
                                    Player exploringPl, CFRData::InfosetData &data,
                                    double p_follow, double u_follow);

    void updateEFGNodeExpectedValue(Player exploringPl, const shared_ptr<EFGNode> &h,
                                    double u_h, double rm_h_pl, double rm_h_opp,
                                    double us_h_cn, double s_h_all) override;


    // only MCCRAlgorithm can update gadget
    friend MCCRAlgorithm;
    void updateGadget(GadgetGame *newGadget);

    double calcPlayInfosetReachProb();
    void updateGadgetBiasingProbs(double playInfosetReachProb);

    GadgetGame *gadget_ = nullptr; // in root, we don't have a gadget
    unordered_map<shared_ptr<AOH>, CFRData::InfosetData> gadgetInfosetData_;
    ProbDistribution gadgetChanceProbs_ = ProbDistribution(OOS_MAX_ACTIONS);
    double gadgetBsum_;
    MCCRSettings mccr_cfg_;
    MCCRData &keep_;
    double leafWeight_;
    double updateMagnitude_ = 0.0;
};

class MCCRAlgorithm: public ContinualResolving {
    MCCRData &cache_;
    unique_ptr<MCCRResolver> resolver_;
    MCCRSettings cfg_;

 public:
    MCCRAlgorithm(const Domain &domain, Player playingPlayer, MCCRData &data,
                  unique_ptr<MCCRResolver> resolver, MCCRSettings settings)
        : ContinualResolving(domain, playingPlayer),
          cache_(data),
          resolver_(move(resolver)),
          cfg_(move(settings)) {}

    MCCRAlgorithm(const Domain &domain, Player playingPlayer, MCCRData &data, MCCRSettings settings)
        : MCCRAlgorithm(domain, playingPlayer, data,
                        make_unique<MCCRResolver>(domain, playingPlayer, data, settings),
                        settings) {}
    ~MCCRAlgorithm() override = default;

    const MCCRSettings &getSettings() { return cfg_; }
    void updateGadget() override;

    const MCCRData &getCache() const override { return cache_; }
    PlayControl preplayIteration(const shared_ptr<EFGNode> &rootNode) override;
    PlayControl resolveIteration(const shared_ptr<GadgetRootNode> &gadgetRoot,
                                 const shared_ptr<AOH> &currentInfoset) override;
    MCCRResolver *getResolver() { return resolver_.get(); }

    double calcProbOfLastAction();
};

}

#endif //GTLIB2_MCCR_H
