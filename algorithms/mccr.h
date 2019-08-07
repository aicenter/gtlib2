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
    enum RetentionPolicy { ResetData, KeepData };

    RetentionPolicy retentionPolicy = ResetData;

    template<class Archive>
    void serialize(Archive &archive) {
        archive(CEREAL_NVP(samplingBlock),
                CEREAL_NVP(accumulatorWeighting),
                CEREAL_NVP(regretMatching),
                CEREAL_NVP(targeting),
                CEREAL_NVP(playStrategy),
                CEREAL_NVP(samplingScheme),
                CEREAL_NVP(avgStrategyComputation),
                CEREAL_NVP(baseline),
                CEREAL_NVP(exploration),
                CEREAL_NVP(targetBiasing),
                CEREAL_NVP(approxRegretMatching),
                CEREAL_NVP(batchSize),
                CEREAL_NVP(seed),
                CEREAL_NVP(retentionPolicy));
    }
};

class MCCRResolver: public OOSAlgorithm {
 public:
    MCCRResolver(const Domain &domain, Player playingPlayer, OOSData &cache, const OOSSettings &cfg)
        : OOSAlgorithm(domain, playingPlayer, cache, cfg) {}

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
    void updateGadgetInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                                    CFRData::InfosetData &data,
                                    double us_h_cn, double rm_zha_all, double rm_ha_all);

    // only MCCRAlgorithm can update gadget
    friend MCCRAlgorithm;
    void updateGadget(GadgetGame *newGadget);

    double updateGadgetInfosetData();
    void updateGadgetBiasingProbs(double playInfosetReachProb);

    GadgetGame *gadget_ = nullptr; // in root, we don't have a gadget
    unordered_map<shared_ptr<AOH>, CFRData::InfosetData> gadgetInfosetData_;
    ProbDistribution gadgetChanceProbs_ = ProbDistribution(OOS_MAX_ACTIONS);
    double gadgetBsum_;
};

class MCCRAlgorithm: public ContinualResolving {
    unique_ptr<MCCRResolver> resolver_;
    MCCRSettings cfg_;

 public:
    MCCRAlgorithm(const Domain &domain, Player playingPlayer, OOSData &cache, MCCRSettings settings)
        : ContinualResolving(domain, playingPlayer, cache),
          resolver_(make_unique<MCCRResolver>(domain, playingPlayer, cache, settings)),
          cfg_(settings) {}
    ~MCCRAlgorithm() override = default;

    void updateGadget() override;

    void solveEntireGame(int preplayBudget, int resolveBudget, BudgetType type) override;
    PlayControl preplayIteration(const shared_ptr<EFGNode> &rootNode) override;
    PlayControl resolveIteration(const shared_ptr<GadgetRootNode> &rootNode,
                                 const shared_ptr<AOH> &currentInfoset) override;

};

};

#endif //GTLIB2_MCCR_H
