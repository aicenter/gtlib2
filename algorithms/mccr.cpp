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


#include "algorithms/mccr.h"

namespace GTLib2::algorithms {

PlayControl MCCRAlgorithm::preplayIteration(const shared_ptr<EFGNode> &rootNode) {
    const auto &r = resolver_;

    r->isBiasedIteration_ = false; // nothing to bias towards in preplay
    r->isBelowTargetIS_ = true; // everything is below "target IS"

    for (int t = 0; t < r->cfg_.batchSize; ++t) {
        for (int exploringPl = 0; exploringPl < 2; ++exploringPl) {
            // no compensation in preplay
            r->iteration(rootNode, 1.0, 1.0, 1.0, 1.0, 1.0, exploringPl);
            ++r->stats_.rootVisits;
        }
    }

    return ContinueImproving;
}

PlayControl MCCRAlgorithm::resolveIteration(const shared_ptr<GadgetRootNode> &gadgetRoot,
                                            const shared_ptr<AOH> &currentInfoset) {
    const auto &r = resolver_;
    // Since CR has called resolve iteration, we have gadget at our disposal.
    // We thus also must have current infoset in cache.
    assert(r->cache_.hasInfoset(currentInfoset));
    assert(r->cache_.hasPublicStateFor(currentInfoset));

    r->playInfoset_ = currentInfoset;
    r->playPublicState_ = r->cache_.getPublicStateFor(*r->playInfoset_);

    // todo: take another look at weighting?
//    const auto successfulUpdate = r->targetor_.updateCurrentPosition(
//        r->playInfoset_, r->playPublicState_);
//    assert(successfulUpdate);

    double compensation = 1.; // 1. / r->targetor_.compensateTargeting();
    for (int t = 0; t < r->cfg_.batchSize; ++t) {
        for (int exploringPl = 0; exploringPl < 2; ++exploringPl) {
            r->isBiasedIteration_ = r->dist_(r->generator_) <= r->cfg_.targetBiasing;
            r->isBelowTargetIS_ = false;

            r->iteration(gadgetRoot, 1.0, 1.0, compensation, compensation, 1.0, exploringPl);
            ++r->stats_.rootVisits;
        }
    }

    return ContinueImproving;
}

void MCCRAlgorithm::updateGadget() {
    ContinualResolving::updateGadget();
    resolver_->updateGadget(gadget_.get());

    if (cfg_.retentionPolicy == MCCRSettings::ResetData) {
        for (auto&[key, val] : cache_.baselineValues) val.reset();
        for (auto&[key, val] : cache_.infosetData) val.reset();
    } else if (cfg_.retentionPolicy == MCCRSettings::ReweighKeepData) {
        // Calculate probability of reweighing -- i.e. the probability
        // of coming into current play infoset. Updates will be done
        // as we visit each history during sampling -- we will check
        // when was the last time it was updated, and multiply regrets
        // and baseline values appropriately.
        double p = calcProbOfLastAction();
        cache_.probUpdates.emplace_back(p/(1+p));
//        for (auto&[key, val] : cache_.infosetData)  // we will reset only avg strategy acc
//            std::fill(val.avgStratAccumulator.begin(), val.avgStratAccumulator.end(), 0.);
    }
}
double MCCRAlgorithm::calcProbOfLastAction() {
    const auto currentNode = (*cache_.getNodesFor(*playInfoset_).begin());

    // go up until we get previous player's node or root
    auto aNode = currentNode->getParent();
    while (aNode != nullptr &&
        !(aNode->type_ == PlayerNode && aNode->getPlayer() == playingPlayer_))
        aNode = aNode->getParent();
    if (aNode == nullptr) return 1.0;

    // assumes that trunk strategy has not changed!
    auto lastAction = currentNode->getHistory().at(aNode->getHistory().size());
    double p = (*cache_.strategyFor(aNode->getAOHInfSet())).at(lastAction);
    return p;
}

void MCCRResolver::updateGadget(GadgetGame *newGadget) {
    gadget_ = newGadget;

    double playInfosetReachProb = updateGadgetInfosetData();
    updateGadgetBiasingProbs(playInfosetReachProb);
}

double MCCRResolver::updateGadgetInfosetData() {
    gadgetInfosetData_.clear();

    const auto &summary = gadget_->summary_;
    const auto numGadgetHistories = summary.topmostHistories.size();
    double playInfosetReachProb = 0.;
    for (int i = 0; i < numGadgetHistories; ++i) {
        const auto &h = summary.topmostHistories.at(i);
        const auto infoset = h->getAOHAugInfSet(gadget_->viewingPlayer_);
        gadgetInfosetData_.emplace(infoset, CFRData::InfosetData(2, HistoriesUpdating));

        if (gadget_->targetAOH_->getAOids() == h->getAOids(gadget_->resolvingPlayer_)) {
            playInfosetReachProb += gadget_->chanceProbForAction(i);
        }
    }
    return playInfosetReachProb;
}

// Store root biased probabilities: it is a
//   epsilon-convex of uniform and (delta-convex of current infoset and subgame)
//    ^ make sure non-zero             ^ make sure we make targetting of current IS
void MCCRResolver::updateGadgetBiasingProbs(double playInfosetReachProb) {
    const auto &summary = gadget_->summary_;
    const auto numGadgetHistories = summary.topmostHistories.size();
    const double p_unif = 1. / numGadgetHistories;

    gadgetBsum_ = 0.0;
    for (int i = 0; i < numGadgetHistories; ++i) {
        const auto h = summary.topmostHistories.at(i);
        // todo: take another look at this
        // @formatter:off
        double p = explore(p_unif, bias(
            (gadget_->targetAOH_->getAOids() == h->getAOids(gadget_->resolvingPlayer_))
              ? gadget_->chanceProbForAction(i) / playInfosetReachProb
              : 0,
            gadget_->chanceProbForAction(i))
        );
        // @formatter:on
        gadgetChanceProbs_.at(i) = p;
        gadgetBsum_ += p;
    }
}

double MCCRResolver::handleChanceNode(const shared_ptr<EFGNode> &h, double rm_h_pl, double rm_h_opp,
                                      double bs_h_all, double us_h_all, double us_h_cn,
                                      Player exploringPl) {
    if (h->getSpecialization() == NoSpecialization)
        return OOSAlgorithm::handleChanceNode(
            h, rm_h_pl, rm_h_opp, bs_h_all, us_h_all, us_h_cn, exploringPl);

    pBiasedProbs_ = &gadgetChanceProbs_; // this is precomputed in "updateGadget"
    const auto ai = selectChanceAction(h, gadgetBsum_);
    const auto pai = h->chanceProbForAction(ai);
    const double bs_ha_all = (*pBiasedProbs_)[ai] / gadgetBsum_;
    const double us_ha_all = pai;

    const auto &nextNode = h->performAction(make_shared<Action>(ai));
    const double u_ha = iteration(nextNode,
                                  rm_h_pl, rm_h_opp,
                                  bs_h_all * bs_ha_all, us_h_all * us_ha_all, us_h_cn * us_ha_all,
                                  exploringPl);
    rm_zh_all_ *= pai;

    // do not use a baseline for gadget chance node
    const double s_ha_all = bias(bs_ha_all, us_ha_all);
    const double u_h = (u_ha * pai) / s_ha_all;
    return u_h;
}

double MCCRResolver::handlePlayerNode(const shared_ptr<EFGNode> &h, double rm_h_pl, double rm_h_opp,
                                      double bs_h_all, double us_h_all, double us_h_cn,
                                      Player exploringPl) {
    if (h->getSpecialization() == NoSpecialization)
        return OOSAlgorithm::handlePlayerNode(
            h, rm_h_pl, rm_h_opp, bs_h_all, us_h_all, us_h_cn, exploringPl);

    const auto &actions = h->availableActions();
    const auto &infoset = h->getAOHInfSet();
    const double s_h_all = bias(bs_h_all, us_h_all);
    CFRData::InfosetData &data = gadgetInfosetData_.at(infoset);

    isBelowTargetIS_ = **playInfoset_ == *infoset;
    const bool exploringMoveInNode = h->getPlayer() == exploringPl;
    calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);

    const auto&[biasApplicableActions, bsum] = calcBiasing(h, actions, bs_h_all);
    const auto ai = 0; // always force follow!

    const double rm_ha_all = rmProbs_[ai];
    const double bs_ha_prob = (*pBiasedProbs_)[ai];
    const double us_ha_all = 1.0;
    const double bs_ha_all = exploringMoveInNode
                             ? (bs_ha_prob > 0.0)
                               ? explore(1.0 / biasApplicableActions, bs_ha_prob / bsum) : 0.0
                             : bs_ha_prob / bsum;

    const auto &nextNode = h->performAction(actions[ai]);
    const double u_ha = iteration(nextNode,
                                  (exploringMoveInNode) ? rm_h_pl * rm_ha_all : rm_h_pl,
                                  (exploringMoveInNode) ? rm_h_opp : rm_h_opp * rm_ha_all,
                                  bs_h_all * bs_ha_all, us_h_all * us_ha_all, us_h_cn,
                                  exploringPl);

    const double s_ha_all = bias(bs_ha_all, us_ha_all);
    const double u_h = u_ha * rm_ha_all / s_ha_all;

    const double rm_zha_all = rm_zh_all_;
    rm_zh_all_ *= rm_ha_all;

    if (exploringMoveInNode)
        updateGadgetInfosetRegrets(h, exploringPl, data, us_h_cn, rm_zha_all, rm_ha_all);
    // no need to update avg strategy -- we will not need it because it's a gadget node

    return u_h;
}

PlayerNodeOutcome MCCRResolver::sampleExistingTree(const shared_ptr<EFGNode> &h,
                                                   const vector<shared_ptr<Action>> &actions,
                                                   double rm_h_pl, double rm_h_opp,
                                                   double bs_h_all, double us_h_all, double us_h_cn,
                                                   CFRData::InfosetData &data,
                                                   const shared_ptr<AOH> &infoset,
                                                   Player exploringPl) {
    assert(h->type_ == PlayerNode);

    // we may need to reweight data at infoset
    if(mccr_cfg_.retentionPolicy == MCCRSettings::ReweighKeepData) {
        const auto lastUpdate = keep_.lastReweighUpdate.find(h);
        const auto currentUpdateIdx = keep_.probUpdates.size() - 1;

        // we should keep track of all update indices!
        assert(lastUpdate != keep_.lastReweighUpdate.end());

        int& idx = lastUpdate->second;
        assert(idx >= 0);

        if(currentUpdateIdx != idx) { // should update
            assert(idx < currentUpdateIdx);

            double update = 1.0;
            for (int i = idx + 1; i <= currentUpdateIdx; ++i) update *= keep_.probUpdates.at(i);

            for (double &regret : data.regrets) regret *= update;
            for (double &avg: data.avgStratAccumulator) avg *= update;
            cache_.baselineValues.at(h).nominator *= update;

            idx = currentUpdateIdx;
        }
    }

    return OOSAlgorithm::sampleExistingTree(h, actions,
                                            rm_h_pl, rm_h_opp,
                                            bs_h_all, us_h_all, us_h_cn,
                                            data, infoset, exploringPl);
}


void MCCRResolver::updateGadgetInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                                              CFRData::InfosetData &data,
                                              double us_h_cn, double rm_zha_all, double rm_ha_all) {
    // If there is only follow action, we don't need to update anything
    // This means the gadget is "fake", the opponent cannot discern any histories
    // apart, so he doesn't need to resolve really.
    if (h->countAvailableActions() == 1) return;

    assert(data.regrets.size() == 2);
    const auto terminateNode = h->performAction(make_shared<Action>(1));
    const double u_terminate = terminateNode->getUtilities()[exploringPl] * gadget_->pubStateReach_;
    const double u_follow = u_z_ * us_h_cn * rm_zha_all / s_z_all_;
    const double p_follow = rm_ha_all;

    const double diff = u_follow - u_terminate;
    data.regrets[0] += (1 - p_follow) * diff;
    data.regrets[1] += -p_follow * diff;
}

double MCCRResolver::handleTerminalNode(const shared_ptr<EFGNode> &h,
                                        double bs_h_all, double us_h_all,
                                        Player exploringPl) {

    // We must multiply leaf utilities by pub state reach,
    // to eliminate normalization in the gadget chance node
    double reach = gadget_ ? gadget_->pubStateReach_ : 1.0; // 1.0 if not using gadget in the root
    return reach * OOSAlgorithm::handleTerminalNode(h, bs_h_all, us_h_all, exploringPl);
}

}
