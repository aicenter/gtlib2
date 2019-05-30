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


void Targetor::updateCurrentPosition(const optional<shared_ptr<AOH>> &infoset,
                                     const optional<shared_ptr<EFGPublicState>> &pubState) {
    if (infoset == nullopt) {
        weightingFactor_ = 1.0;
        return;
    }
    if (currentInfoset_ == infoset) return;

    currentInfoset_ = *infoset;
    currentPubState_ = *pubState;

    bsSum_ = 0.0;
    usSum_ = 0.0;
    updateWeighting(cache_.getRootNode(), 1.0, 1.0);
    weightingFactor_ = (1 - targetBiasing_) + targetBiasing_ * bsSum_ / usSum_;
}

void Targetor::updateWeighting(const shared_ptr<EFGNode> &h, double bs_h_all, double us_h_all) {
    const auto updateInfoset = h->getAOHInfSet();

    if (*updateInfoset == *currentInfoset_) {
        bsSum_ += bs_h_all;
        usSum_ += us_h_all;
        return; // do not go below
    }

    double biasedSum = 0.0;
    const auto actions = h->availableActions();
    ProbDistribution dist;

    switch (h->type_) {
        case ChanceNode:
            dist = h->chanceProbs();
            break;

        case PlayerNode: {
            CFRData::InfosetData &data = cache_.infosetData.at(updateInfoset);
            dist = calcRMProbs(data.regrets);
        }

        case TerminalNode:
            assert(false); // no need to update weighting for terminal nodes!

        default:
            assert(false); // unrecognized option!
    }

    for (const auto &action: actions) {
        if (!isAllowedAction(h, action)) dist[action->getId()] = 0.0;
    }
    for (const auto &action: actions) biasedSum += dist[action->getId()];
    for (const auto &action: actions) {
        const double pa = dist[action->getId()];
        if (pa <= 0) continue;
        if (!cache_.hasChildren(h, action)) continue; // target only parts of the tree we have built
        updateWeighting(h->performAction(action), us_h_all * pa, bs_h_all * pa / biasedSum);
    }
}

bool Targetor::isAllowedAction(const shared_ptr<EFGNode> &h, const shared_ptr<Action> &action) {
    switch (targeting_) {
        case OOSSettings::InfosetTargeting:
            return isAOCompatible(currentInfoset_->getAOids(),
                                  h->performAction(action)->getAOids(currentInfoset_->getPlayer()));

        case OOSSettings::PublicStateTargeting:
            assert(false); // todo: finish public state targeting
            return false;

        default:
            assert(false); // unrecognized option!
    }
}


PlayControl OOSAlgorithm::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    playInfoset_ = currentInfoset;
    playPublicState_ = playInfoset_ && cache_.hasPublicState(*playInfoset_)
                       ? optional(cache_.getPublicStateFor(*playInfoset_))
                       : nullopt;
    targetor_.updateCurrentPosition(playInfoset_, playPublicState_);

    for (int t = 0; t < cfg_.batchSize; ++t) {
        for (int exploringPl = 0; exploringPl < 2; ++exploringPl) {
            isBiasedIteration_ = dist_(generator_) <= cfg_.targetBiasing;
            isBelowTargetIS_ = currentInfoset == nullopt || (*currentInfoset)->getSize() == 0;

            iteration(cache_.getRootNode(), 1.0, 1.0, 1.0,
                      1 / targetor_.compensateTargetting(),
                      1 / targetor_.compensateTargetting(),
                      Player(exploringPl));
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
            assert(false); // unrecognized option!
    }
}

double OOSAlgorithm::iteration(const shared_ptr<EFGNode> &h,
                               double rm_h_pl, double rm_h_opp, double rm_h_cn,
                               double bs_h_all, double us_h_all,
                               Player exploringPl) {

    stats_.nodesVisits++;
    double s_h_all = bias(bs_h_all, us_h_all);

    if (h->type_ == TerminalNode) {
        s_z_all_ = s_h_all;
        u_z_ = h->getUtilities()[exploringPl];
        stats_.terminalsVisits++;
        return u_z_;
    }

    const auto &actions = h->availableActions();
    const auto numActions = actions.size();
    unsigned int ai;  // action index
    double bs_ha_all, us_ha_all, s_ha_all;

    if (h->type_ == ChanceNode) {
        // todo: add rmProbs_ optimization
        // todo: merge case with playernode as much as possible?
        const auto probs = h->chanceProbs();

        tie(ai, bs_ha_all) = selectChanceAction(h);
        us_ha_all = probs[ai];

        const auto &nextNode = cache_.getChildFor(h, actions[ai]);
        double u_ha = iteration(nextNode,
                                rm_h_pl, rm_h_opp, rm_h_cn * probs[ai],
                                bs_h_all * bs_ha_all, us_h_all * us_ha_all,
                                exploringPl);
        rm_zh_all_ *= probs[ai];

        // compute baseline-augmented utilities
        s_ha_all = bias(bs_ha_all, us_ha_all);
        double u_h = (u_ha - baseline(h, ai) * probs[ai]) / s_ha_all;
        for (const auto &action : actions) {
            u_h += probs[action->getId()] * baseline(h, action->getId());
        }

        return u_h;
    }

    assert(h->type_ == PlayerNode);

    const auto &infoset = cache_.getInfosetFor(h);
    CFRData::InfosetData &data = cache_.infosetData.at(infoset);

    // @formatter:off
    double u_h;       // baseline-augmented estimate of expected utility for current history
    double u_ha;      // baseline-augmented estimate of expected utility for next history
    double u_x;       // baseline-augmented estimate of expected utility for next history,
                      //   if we go there with 100% probability from current history
    double rm_ha_all; // probability of taking this action (according to RM)
    // @formatter:on

    if (h->getAOHInfSet() == playInfoset_) stats_.infosetVisits++;
    if (h->getPublicState() == playPublicState_) stats_.pubStateVisits++;

    if (!cache_.hasAnyChildren(h)) { // new node - incrementally build tree
        ai = pickUniform(numActions, generator_);
        rm_ha_all = 1.0 / numActions;
        const auto leaf = pickRandomLeaf(h, actions[ai], generator_);
        u_z_ = leaf.utilities[exploringPl];
        rm_zh_all_ = leaf.reachProb(); // "* rm_ha_all" will be added at the bottom
        s_z_all_ = bias(bs_h_all, us_h_all) * leaf.reachProb() * rm_ha_all;

        // compute replacement for baseline-augmented utilities
        u_ha = u_z_;
        u_x = u_ha;
        u_h = u_x;
    } else {
        calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);
        const auto&[biasApplicableActions, bsum] = calcBiasing(h, infoset, bs_h_all, numActions);
        u_h = 0.;

        bool exploringInNode = h->getPlayer() == exploringPl;
        if (exploringInNode) { // exploring move
            tie(ai, us_ha_all) = selectExploringPlayerAction(h, biasApplicableActions, bsum);
            rm_ha_all = rmProbs_[ai];

            // the following is zero for banned actions and the correct probability for allowed
            bs_ha_all = 0.;
            if ((*pBiasedProbs_)[ai] > 0.0) {
                bs_ha_all = explore(1.0 / biasApplicableActions, (*pBiasedProbs_)[ai] / bsum);
            }

            // precompute baseline components now, because after child iteration RM probs will change
            for (int i = 0; i < numActions; ++i) {
                if (i == ai) continue;
                u_h += rmProbs_[i] * baseline(h, i);
            }
        } else {
            tie(ai, us_ha_all) = selectNonExploringPlayerAction(h, bsum);
            bs_ha_all = (*pBiasedProbs_)[ai] / bsum;
            rm_ha_all = rmProbs_[ai];

            // precompute baseline components now, because after child iteration RM probs will change
            for (int i = 0; i < numActions; ++i) {
                if (i == ai) continue;
                u_h += rmProbs_[i] * baseline(h, i);
            }
        }

        const auto &nextNode = cache_.getChildFor(h, actions[ai]);
        u_ha = iteration(nextNode,
                         (exploringInNode) ? rm_h_pl * rm_ha_all : rm_h_pl,
                         (exploringInNode) ? rm_h_opp : rm_h_opp * rm_ha_all,
                         rm_h_cn,
                         bs_h_all * bs_ha_all, us_h_all * us_ha_all,
                         exploringPl);

        // finish computing baseline-augmented utilities
        s_ha_all = bias(bs_ha_all, us_ha_all);
        u_x = (u_ha - baseline(h, ai)) / s_ha_all + baseline(h, ai);
        u_h += u_x * rm_ha_all;
    }

    // regret/mean strategy update
    double rm_zha_all = rm_zh_all_;
    rm_zh_all_ *= rm_ha_all;

    updateEFGNodeExpectedValue(exploringPl, h, u_h, rm_h_pl, rm_h_opp, rm_h_cn, s_h_all);

    (h->getPlayer() == exploringPl)
    ? updateInfosetRegrets(h, exploringPl, data, ai, u_x, u_h, rm_h_opp * rm_h_cn / s_h_all)
    : updateInfosetAcc(h, data, rm_h_opp * rm_h_cn / s_h_all);

    return u_h;
}


pair<int, double> OOSAlgorithm::calcBiasing(const shared_ptr<EFGNode> &h,
                                            const shared_ptr<AOH> &infoset,
                                            double bs_h_all,
                                            int numActions) {
    double bsum = 0.0;
    int biasApplicableActions = 0;
    pBiasedProbs_ = &tmpProbs_;

    // targeting may still make a difference
    if (cfg_.targetBiasing > 0 && bs_h_all > 0 && !isBelowTargetIS_) {
        if (playInfoset_ == infoset) isBelowTargetIS_ = true;
        else tie(biasApplicableActions, bsum) = updateBiasing(h);
    }

    // if all actions were not present for the opponent or it was below the current IS
    if (bsum == 0) {
        pBiasedProbs_ = &rmProbs_;
        bsum = 1;
        biasApplicableActions = numActions;
    }

    return make_pair(biasApplicableActions, bsum);
}

pair<int, double> OOSAlgorithm::updateBiasing(const shared_ptr<EFGNode> &h) {
    const auto &actions = h->availableActions();
    double bsum = 0.;
    int biasApplicableActions = 0;

    const auto probs = h->type_ == PlayerNode ? rmProbs_ : h->chanceProbs();

    for (int i = 0; i < actions.size(); ++i) {
        if (targetor_.isAllowedAction(h, actions[i])) {
            (*pBiasedProbs_)[i] = rmProbs_[i];
            bsum += rmProbs_[i];
            ++biasApplicableActions;
        } else (*pBiasedProbs_)[i] = -0.0; // negative zeros denote the banned actions
    }

    return make_pair(biasApplicableActions, bsum);
}

pair<int, double> OOSAlgorithm::selectChanceAction(const shared_ptr<EFGNode> &h) {
    const auto numActions = h->countAvailableActions();

    // avoid using random number generator when chance is deterministic
    if (numActions == 1) return make_pair(0, 1.0);

    if (cfg_.targetBiasing == 0.) { // prevent unnecessary calculations
        int ai = pickRandom(*h, generator_);
        return make_pair(ai, h->chanceProbForAction(ai));
    }

    // now finally general chance biasing
    int biasApplicableActions = 0;
    double bsum = 0;
    if (!isBelowTargetIS_) {
        tie(biasApplicableActions, bsum) = updateBiasing(h);
    }

    int ai;
    if (isBiasedIteration_ && bsum > 0) {
        ai = pickRandom(*pBiasedProbs_, bsum, generator_);
    } else {
        ai = pickRandom(*h, generator_);
    }

    double biasedProb;
    if (bsum > 0) biasedProb = (*pBiasedProbs_)[ai] / bsum;
    else biasedProb = h->chanceProbForAction(ai);

    return make_pair(ai, biasedProb);
}


pair<int, double> OOSAlgorithm::selectExploringPlayerAction(const shared_ptr<EFGNode> &h,
                                                            int biasApplicableActions,
                                                            double bsum) {
    const auto numActions = h->countAvailableActions();
    int ai;
    double us_ha_all;

    if (!isBiasedIteration_) { // No biasing
        ai = dist_(generator_) <= cfg_.exploration
             ? pickUniform(numActions, generator_) // with exploration
             : pickRandom(rmProbs_, generator_);   // no exploration
    } else { // With biasing
        if (dist_(generator_) <= cfg_.exploration) { // with exploration
            ai = pickUniform(biasApplicableActions, generator_);
            if (biasApplicableActions != numActions) {
                int j = ai;
                ai = 0;

                // the following sets ai to the j-th allowed action
                while (is_negative_zero((*pBiasedProbs_)[ai]) || j-- > 0) ai++;
            }
        } else {
            ai = pickRandom(*pBiasedProbs_, bsum, generator_); // no exploration
        }
    }

    us_ha_all = explore(1. / numActions, rmProbs_[ai]);
    return make_pair(ai, us_ha_all);
}

pair<int, double> OOSAlgorithm::selectNonExploringPlayerAction(const shared_ptr<EFGNode> &h,
                                                               double bsum) {
    int ai;
    double us_ha_all;

    if (isBiasedIteration_) ai = pickRandom(*pBiasedProbs_, bsum, generator_);
    else ai = pickRandom(rmProbs_, generator_);
    us_ha_all = rmProbs_[ai];

    return make_pair(ai, us_ha_all);
}

void OOSAlgorithm::updateEFGNodeExpectedValue(Player exploringPl, const shared_ptr<EFGNode> &h,
                                              double u_h,
                                              double rm_h_pl, double rm_h_opp, double rm_h_cn,
                                              double s_h_all) {

    // let's make sure that the utility is always for player 0
    // updateVal we get is for the exploring player
    u_h *= exploringPl == Player(0) ? 1 : -1;

    const auto &baselineIdx = cache_.baselineValues_.find(h);
    assert(baselineIdx != cache_.baselineValues_.end());
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
            reach = rm_h_pl * rm_h_opp * rm_h_cn;
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
            assert(false); // todo:
        default:
            assert(false); // unrecognized option!
    }

    baseline.nominator += a;
    baseline.denominator += b;
}

void OOSAlgorithm::updateInfosetAcc(const shared_ptr<EFGNode> &h, CFRData::InfosetData &data,
                                    double s) {
    double w = 1.0;
    switch (cfg_.accumulatorWeighting) {
        case OOSSettings::UniformAccWeighting:
            w = 1.0;
            break;
        case OOSSettings::LinearAccWeighting:
            w = stats_.terminalsVisits + 1;
            break;
        case OOSSettings::XLogXAccWeighting:
            w = (stats_.terminalsVisits + 1) * log10(stats_.terminalsVisits + 1);
            break;
        default:
            assert(false); // unrecognized option!
    }

    switch (cfg_.avgStrategyComputation) {
        case OOSSettings::StochasticallyWeightedAveraging:
            calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);
            for (int i = 0; i < data.avgStratAccumulator.size(); i++) {
                data.avgStratAccumulator[i] += w * s * rmProbs_[i];
            }
            break;
        case OOSSettings::LazyWeightedAveraging:
            assert(false);  // todo: implement
        default:
            assert(false);  // unrecognized option!
    }
}

void OOSAlgorithm::updateInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                                        CFRData::InfosetData &data, int ai,
                                        double u_x, double u_h, double w) {

    auto &reg = data.regrets;

    switch (cfg_.regretMatching) {
        case OOSSettings::RegretMatchingPlus:
            for (int i = 0; i < reg.size(); i++) {
                if (i == ai) reg[i] = fmax(0, reg[i] + (u_x - u_h) * w);
                else {
                    reg[i] = fmax(0, reg[i] + (baseline(h, i) - u_h) * w);
                }
            }
            break;
        case OOSSettings::RegretMatchingNormal:
            for (int i = 0; i < reg.size(); i++) {
                if (i == ai) reg[i] += (u_x - u_h) * w;
                else {
                    reg[i] += (baseline(h, i) - u_h) * w;
                }
            }
            break;
        default:
            assert(false); // unrecognized option!
    }
}

#undef baseline

}  // namespace GTLib2
