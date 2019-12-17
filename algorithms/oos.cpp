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

#include "algorithms/oos.h"

namespace GTLib2::algorithms {

// so that we have slightly nicer code
#define baseline(node, actionIdx) (cache_.getBaselineFor(node, actionIdx, exploringPl))


bool Targetor::updateCurrentPosition(const optional<shared_ptr<AOH>> &infoset,
                                     const optional<shared_ptr<PublicState>> &pubState) {
    if (targeting_ == OOSSettings::Targeting::NoTargeting) {
        weightingFactor_ = 1.0;
        return true;
    }

    if (!infoset) {
        weightingFactor_ = 1.0;
        return true;
    }
    if (currentInfoset_ == infoset) return true;

    currentInfoset_ = *infoset;
    currentPubState_ = *pubState;

    const auto[bsSum, usSum] = updateWeighting(cache_.getRootNode(), 1.0, 1.0);
    if (usSum == 0.0) return false;
    assert(usSum > 0.0);
    weightingFactor_ = (1 - targetBiasing_) + targetBiasing_ * bsSum / usSum;
    return true;
}

pair<double, double> Targetor::updateWeighting(const shared_ptr<EFGNode> &h,
                                               double bs_h_all, double us_h_all) {
    const auto updateInfoset = h->getAOHInfSet();

    if (*updateInfoset == *currentInfoset_) {
        return make_pair(bs_h_all, us_h_all); // do not go below
    }

    double biasedSum = 0.0;
    const auto actions = h->availableActions();
    ProbDistribution dist;

    switch (h->type_) {
        case ChanceNode:
            dist = h->chanceProbs();
            break;
        case PlayerNode: {
            CFRData::InfosetData data = cache_.infosetData.at(updateInfoset);
            dist = vector<double>(data.regrets.size());
            calcRMProbs(data.regrets, &dist, 0.001); // todo: eps parameter?
            break;
        }
        case TerminalNode:
            unreachable("no need to update weighting for terminal nodes!");
        default:
            unreachable("unrecognized option!");
    }

    for (const auto &action: actions) {
        if (!isAllowedAction(h, action)) dist[action->getId()] = 0.0;
    }
    for (const auto &action: actions) biasedSum += dist[action->getId()];

    double ussum = 0.0, bssum = 0.0;
    for (const auto &action: actions) {
        const double pa = dist[action->getId()];
        if (pa <= 0.0) continue;

        // Target only parts of the tree we have built.
        // We have checked that the infoset which we want to target
        //   does indeed exist!
        if (!cache_.hasChildren(h, action)) continue;

        const auto[updateUs, updateBs] = updateWeighting(h->performAction(action),
                                                         us_h_all * pa,
                                                         bs_h_all * pa / biasedSum);
        ussum += updateUs;
        bssum += updateBs;
    }
    return make_pair(ussum, bssum);
}

bool Targetor::isAllowedAction(const shared_ptr<EFGNode> &h, const shared_ptr<Action> &action) {
    const auto nextH = h->performAction(action);

    switch (targeting_) {
        case OOSSettings::InfosetTargeting:
            return isAOCompatible(currentInfoset_->getAOids(),
                                  nextH->getAOids(currentInfoset_->getPlayer()));

        case OOSSettings::PublicStateTargeting:
            return isCompatible(currentPubState_->getHistory(), nextH->getPubObsIds());

        case OOSSettings::NoTargeting:
            return true;

        default:
            unreachable("unrecognized option!");
    }
}


PlayControl OOSAlgorithm::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    // we can't make targetting, if the infoset is not in cache. Give up - play randomly
    // todo: make partial targetting up to the last known infoset at least?
    if (currentInfoset && !cache_.hasInfoset(*currentInfoset)) return GiveUp;

    playInfoset_ = currentInfoset;
    playPublicState_ = playInfoset_ && cache_.hasPublicStateFor(*playInfoset_)
                       ? optional(cache_.getPublicStateFor(*playInfoset_))
                       : nullopt;

    if (!targetor_.updateCurrentPosition(playInfoset_, playPublicState_))
        return GiveUp;

    for (unsigned int t = 0; t < cfg_.batchSize; ++t) {
        for (int exploringPl = 0; exploringPl < 2; ++exploringPl) {
            isBiasedIteration_ = dist_(generator_) <= cfg_.targetBiasing;
            isBelowTargetIS_ = currentInfoset == nullopt
                || (*currentInfoset)->getAOids().size() == 0;

            rootIteration(1 / targetor_.compensateTargeting(), Player(exploringPl));
        }
    }

    return ContinueImproving;
}

optional<ProbDistribution>
OOSAlgorithm::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
    const auto infosetLoc = cache_.infosetData.find(currentInfoset);
    if (infosetLoc == cache_.infosetData.end()) return nullopt;
    const auto strategyData = infosetLoc->second;

    switch (cfg_.playStrategy) {
        case OOSSettings::PlayUsingAvgStrategy:
            return calcAvgProbs(strategyData.avgStratAccumulator);
        case OOSSettings::PlayUsingRMStrategy:
            return calcRMProbs(strategyData.regrets);
        default:
            unreachable("unrecognized option!");
    }
}

void OOSAlgorithm::rootIteration(double compensation, Player exploringPl) {
    iteration(cache_.getRootNode(), 1.0, 1.0, compensation, compensation, 1.0, exploringPl);
    ++stats_.rootVisits;
}

double OOSAlgorithm::iteration(const shared_ptr<EFGNode> &h,
                               double rm_h_pl, double rm_h_opp,
                               double bs_h_all, double us_h_all,
                               double us_h_cn,
                               Player exploringPl) {

    ++stats_.nodesVisits;

    switch (h->type_) {
        case TerminalNode:
            ++stats_.terminalsVisits;
            return handleTerminalNode(h, bs_h_all, us_h_all, exploringPl);
        case ChanceNode:
            return handleChanceNode(h, rm_h_pl, rm_h_opp, bs_h_all, us_h_all, us_h_cn, exploringPl);
        case PlayerNode:
            if (h->getSpecialization() == NoSpecialization) {
                // do not generate unncessary objects (AOH/PublicState), just compare ids

                if (playInfoset_
                    && h->getAOids(h->getPlayer()) == (*playInfoset_)->getAOids()
                    && (*playInfoset_)->getPlayer() == h->getPlayer())
                    ++stats_.infosetVisits;

                if (playPublicState_
                    && h->getPubObsIds() == (*playPublicState_)->getHistory())
                    ++stats_.pubStateVisits;
            }

            return handlePlayerNode(h, rm_h_pl, rm_h_opp, bs_h_all, us_h_all, us_h_cn, exploringPl);
        default:
            unreachable("unrecognized option!");
    }
}

double OOSAlgorithm::handleTerminalNode(const shared_ptr<EFGNode> &h,
                                        double bs_h_all, double us_h_all,
                                        Player exploringPl) {
    s_z_all_ = bias(bs_h_all, us_h_all);
    u_z_ = h->getUtilities()[exploringPl];
    assert(!isnan(u_z_) && !isinf(u_z_));
    return u_z_;
}

double OOSAlgorithm::handleChanceNode(const shared_ptr<EFGNode> &h,
                                      double rm_h_pl, double rm_h_opp,
                                      double bs_h_all, double us_h_all, double us_h_cn,
                                      Player exploringPl) {
    const auto &actions = h->availableActions();
    const auto probs = h->chanceProbs();

    const auto[biasApplicableActions, bsum] = calcBiasing(h, actions, bs_h_all);
    const auto ai = selectChanceAction(h, bsum);
    double bs_ha_all = bsum > 0
                       ? (*pBiasedProbs_)[ai] / bsum
                       : h->chanceProbForAction(ai);

    const double us_ha_all = probs[ai];

    const auto &nextNode = cache_.getChildFor(h, actions[ai]);
    const double u_ha = iteration(nextNode,
                                  rm_h_pl, rm_h_opp,
                                  bs_h_all * bs_ha_all, us_h_all * us_ha_all, us_h_cn * us_ha_all,
                                  exploringPl);
    rm_zh_all_ *= probs[ai];

    // compute baseline-augmented utilities
    double s_ha_all = bias(bs_ha_all, us_ha_all);
    double u_h = (u_ha - baseline(h, ai) * probs[ai]) / s_ha_all;
    for (const auto &action : actions) {
        u_h += probs[action->getId()] * baseline(h, action->getId());
    }

    assert(!isnan(u_h) && !isinf(u_h));
    return u_h;
}

double OOSAlgorithm::handlePlayerNode(const shared_ptr<EFGNode> &h,
                                      double rm_h_pl, double rm_h_opp,
                                      double bs_h_all, double us_h_all, double us_h_cn,
                                      Player exploringPl) {
    const auto &actions = h->availableActions();
    const auto &infoset = cache_.getInfosetFor(h);
    const double s_h_all = bias(bs_h_all, us_h_all);
    CFRData::InfosetData &data = cache_.infosetData.at(infoset);

    if (playInfoset_ != nullopt && **playInfoset_ == *infoset)
        isBelowTargetIS_ = true; // Change the state. All next histories will be under the target IS

    // @formatter:off
    const auto[ai, rm_ha_all, u_h, u_x] = (cache_.hasAnyChildren(h))
        ? sampleExistingTree(h, actions, rm_h_pl, rm_h_opp, bs_h_all, us_h_all, us_h_cn,
                             data, infoset, exploringPl)
        : incrementallyBuildTree(h, actions, bias(bs_h_all, us_h_all), exploringPl);
    // @formatter:on
    rm_zh_all_ *= rm_ha_all;

    updateEFGNodeExpectedValue(exploringPl, h, u_h, rm_h_pl, rm_h_opp, us_h_cn, s_h_all);

    if (h->getPlayer() == exploringPl)
        updateInfosetRegrets(h, exploringPl, data, ai, u_x, u_h,
                             rm_h_opp * us_h_cn / s_h_all);
    else
        updateInfosetAcc(h, data, rm_h_opp * us_h_cn / s_h_all);

    assert(!isnan(u_h) && !isinf(u_h));
    return u_h;
}

PlayerNodeOutcome OOSAlgorithm::sampleExistingTree(const shared_ptr<EFGNode> &h,
                                                   const vector<shared_ptr<Action>> &actions,
                                                   double rm_h_pl, double rm_h_opp,
                                                   double bs_h_all, double us_h_all, double us_h_cn,
                                                   CFRData::InfosetData &data,
                                                   const shared_ptr<AOH> &,
                                                   Player exploringPl) {
    assert(h->type_ == PlayerNode);
    assert(!isnan(rm_h_pl) && !isnan(rm_h_opp) && !isnan(bs_h_all) && !isnan(us_h_all)
               && !isnan(us_h_cn));

    const bool exploringMoveInNode = h->getPlayer() == exploringPl;
    calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);
#ifndef NDEBUG
    if (cfg_.approxRegretMatching > 0)
        for (unsigned int i = 0; i < data.regrets.size(); ++i) assert(rmProbs_[i] > 0);
#endif

    const auto&[biasApplicableActions, bsum] = calcBiasing(h, actions, bs_h_all);
    const auto ai = exploringMoveInNode
                    ? selectExploringPlayerAction(h, biasApplicableActions, bsum)
                    : selectNonExploringPlayerAction(h, bsum);

    const double rm_ha_all = rmProbs_[ai];
    const double bs_ha_prob = (*pBiasedProbs_)[ai];
    const double us_ha_all = exploringMoveInNode
                             ? explore(1. / actions.size(), rmProbs_[ai])
                             : rmProbs_[ai];
    const double bs_ha_all = exploringMoveInNode
                             ? (bs_ha_prob > 0.0)
                               ? explore(1.0 / biasApplicableActions, bs_ha_prob / bsum) : 0.0
                             : bs_ha_prob / bsum;

    // precompute baseline components now, because after child iteration RM probs will change
    double u_h = 0.;
    for (unsigned int i = 0; i < actions.size(); ++i) {
        if (i == ai) continue;
        u_h += rmProbs_[i] * baseline(h, i);
    }

    const auto &nextNode = cache_.getChildFor(h, actions[ai]);
    const double u_ha = iteration(nextNode,
                                  (exploringMoveInNode) ? rm_h_pl * rm_ha_all : rm_h_pl,
                                  (exploringMoveInNode) ? rm_h_opp : rm_h_opp * rm_ha_all,
                                  bs_h_all * bs_ha_all, us_h_all * us_ha_all, us_h_cn,
                                  exploringPl);

    // finish computing baseline-augmented utilities
    const double s_ha_all = bias(bs_ha_all, us_ha_all);
    const double u_x = (u_ha - baseline(h, ai)) / s_ha_all + baseline(h, ai);
    u_h += u_x * rm_ha_all;

    assert(!isnan(rm_ha_all) && !isnan(u_h) && !isnan(u_x));
    return PlayerNodeOutcome(ai, rm_ha_all, u_h, u_x);
}

PlayerNodeOutcome OOSAlgorithm::incrementallyBuildTree(const shared_ptr<EFGNode> &h,
                                                       const vector<shared_ptr<Action>> &actions,
                                                       double s_h_all, Player exploringPl) {
    assert(!isnan(s_h_all));

    const auto[ai, leaf] = selectLeaf(h, actions);
    const double rm_ha_all = 1.0 / actions.size();
    u_z_ = leaf.utilities[exploringPl];
    rm_zh_all_ = leaf.reachProb(); // "* rm_ha_all" will be added at the bottom
    s_z_all_ = s_h_all * leaf.reachProb() * rm_ha_all;

    assert(!isnan(rm_ha_all) && !isnan(u_z_) && !isnan(s_z_all_));
    return PlayerNodeOutcome(ai, rm_ha_all, u_z_, u_z_);
}

pair<int, double> OOSAlgorithm::calcBiasing(const shared_ptr<EFGNode> &h,
                                            const vector<shared_ptr<Action>> &actions,
                                            double bs_h_all) {

    if (cfg_.targetBiasing == 0.0 || bs_h_all == 0.0 || isBelowTargetIS_) { // no biasing case
        pBiasedProbs_ = &rmProbs_; // do not compute RM probs again
        return make_pair(actions.size(), 1.0);
    }

    pBiasedProbs_ = &tmpProbs_; // we will need to store biased probs somewhere
    double bsum = 0.0;
    int biasApplicableActions = 0;

    const auto probs = h->type_ == PlayerNode ? rmProbs_ : h->chanceProbs();
    for (unsigned int i = 0; i < actions.size(); ++i) {
        if (targetor_.isAllowedAction(h, actions[i])) {
            (*pBiasedProbs_)[i] = rmProbs_[i];
            bsum += rmProbs_[i];
            ++biasApplicableActions;
        } else (*pBiasedProbs_)[i] = -0.0; // negative zeros denote the banned actions
    }

    if (bsum == 0) { // if there is now way how to get to the target
        pBiasedProbs_ = &rmProbs_;
        return make_pair(actions.size(), 1.0);
    }

    return make_pair(biasApplicableActions, bsum);
}


pair<ActionId, RandomLeafOutcome>
OOSAlgorithm::selectLeaf(const shared_ptr<EFGNode> &h, const vector<shared_ptr<Action>> &actions) {
    const int ai = pickUniform(actions.size(), generator_);
    cache_.getChildFor(h, actions[ai]); // build the node in cache
    const auto leaf = pickRandomLeaf(h, actions[ai], generator_);
    return make_pair(ai, leaf);
}

ActionId OOSAlgorithm::selectChanceAction(const shared_ptr<EFGNode> &h, double bsum) {
    // avoid using random number generator when outcome is sure
    if (h->countAvailableActions() == 1) return 0;
    return isBiasedIteration_ && bsum > 0 ? pickRandom(*pBiasedProbs_, bsum, generator_)
                                          : pickRandom(*h, generator_);
}

ActionId OOSAlgorithm::selectExploringPlayerAction(const shared_ptr<EFGNode> &h,
                                                   int biasApplicableActions, double bsum) {

    const bool shouldExplore = dist_(generator_) <= cfg_.exploration;
    const bool shouldBias = isBiasedIteration_;

    if (!shouldBias) {
        if (shouldExplore) return pickUniform(h->countAvailableActions(), generator_);
        else return pickRandom(rmProbs_, generator_);
    } else {
        if (shouldExplore) {
            ActionId ai = 0;
            int j = pickUniform(biasApplicableActions, generator_);
            // the following sets ai to the j-th allowed action
            while (is_negative_zero((*pBiasedProbs_)[ai]) || j-- > 0) ai++;
            return ai;
        } else return pickRandom(*pBiasedProbs_, bsum, generator_);
    }
}

ActionId OOSAlgorithm::selectNonExploringPlayerAction(const shared_ptr<EFGNode> &h, double bsum) {
    // avoid using random number generator when outcome is sure
    if (h->countAvailableActions() == 1) return 0;
    return isBiasedIteration_ && bsum > 0 ? pickRandom(*pBiasedProbs_, bsum, generator_)
                                          : pickRandom(rmProbs_, generator_);
}

void OOSAlgorithm::updateEFGNodeExpectedValue(Player exploringPl, const shared_ptr<EFGNode> &h,
                                              double u_h, double rm_h_pl, double rm_h_opp,
                                              double us_h_cn, double s_h_all) {

    // let's make sure that the utility is always for player 0
    // updateVal we get is for the exploring player
    u_h *= exploringPl == Player(0) ? 1 : -1;

    const auto &baselineIdx = cache_.baselineValues.find(h);
    assert(baselineIdx != cache_.baselineValues.end());
    auto baseline = baselineIdx->second;

    double a = 0.0, b = 0.0;
    double reach;
    switch (cfg_.baseline) {
        case OOSSettings::WeightedActingPlayerBaseline:
            reach = h->getPlayer() == exploringPl ? rm_h_pl : rm_h_opp;
            a = reach * u_h;
            b = reach / s_h_all;
            break;
        case OOSSettings::WeightedAllPlayerBaseline:
            reach = rm_h_pl * rm_h_opp * us_h_cn;
            a = reach * u_h;
            b = reach / s_h_all;
            break;
        case OOSSettings::WeightedTimeBaseline:
            a = u_h;
            b = 1.0;
            break;
        case OOSSettings::NoBaseline: // nothing to do
            break;
        case OOSSettings::OracleBaseline:
            unreachable("todo:");
        default:
            unreachable("unrecognized option!");
    }

    baseline.nominator += a;
    baseline.denominator += b;
}

void OOSAlgorithm::updateInfosetAcc(const shared_ptr<EFGNode> &, CFRData::InfosetData &data,
                                    double s) {
    double w = 1.0;
    switch (cfg_.accumulatorWeighting) {
        case AccumulatorWeighting::UniformAccWeighting:
            w = 1.0;
            break;
        case AccumulatorWeighting::LinearAccWeighting:
            w = stats_.terminalsVisits + 1;
            break;
        default:
            unreachable("unrecognized option!");
    }

    switch (cfg_.avgStrategyComputation) {
        case OOSSettings::StochasticallyWeightedAveraging:
            calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);
            for (unsigned int i = 0; i < data.avgStratAccumulator.size(); i++) {
                data.avgStratAccumulator[i] += w * s * rmProbs_[i];
                assert(data.avgStratAccumulator[i] > 0.0);
            }
            break;
        case OOSSettings::LazyWeightedAveraging:
            unreachable("todo: implement");
        default:
            unreachable("unrecognized option!");
    }
}

void OOSAlgorithm::updateInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                                        CFRData::InfosetData &data, unsigned int ai,
                                        double u_x, double u_h, double w) {
    assert(!isnan(u_x) && !isnan(u_h) && !isnan(w));

    auto &reg = data.regrets;

    switch (cfg_.regretMatching) {
        case RegretMatching::RegretMatchingPlus:
            for (unsigned int i = 0; i < reg.size(); i++) {
                if (i == ai) reg[i] = fmax(0, reg[i] + (u_x - u_h) * w);
                else {
                    reg[i] = fmax(0, reg[i] + (baseline(h, i) - u_h) * w);
                }
            }
            break;
        case RegretMatching::RegretMatchingNormal:
            for (unsigned int i = 0; i < reg.size(); i++) {
                if (i == ai) reg[i] += (u_x - u_h) * w;
                else {
                    reg[i] += (baseline(h, i) - u_h) * w;
                }
            }
            break;
        default:
            unreachable("unrecognized option!");
    }
}

}  // namespace GTLib2
