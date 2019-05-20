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
#define baseline(node, actionIdx) cache_.getBaselineFor(node, actionIdx, exploringPl)


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
            ProbDistribution dist = calcRMProbs(data.regrets);
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

bool Targetor::isAllowedAction(const shared_ptr<EFGNode> h, const shared_ptr<Action> action) {
    switch (targeting_) {
        case OOSSettings::InfosetTargeting:
            return isAOCompatible(currentInfoset_->getAOids(),
                                  h->performAction(action)->getAOids(currentInfoset_->getPlayer()));

        case OOSSettings::PublicStateTargeting:
            assert(false); // todo: finish
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

optional<ProbDistribution> OOSAlgorithm::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
    const auto infosetLoc = cache_.infosetData.find(currentInfoset);
    if (infosetLoc == cache_.infosetData.end()) return nullopt;
    const auto strategyData = infosetLoc->second;

    return cfg_.playStrategy == OOSSettings::PlayUsingAvgStrategy
           ? calcAvgProbs(strategyData.avgStratAccumulator)
           : calcRMProbs(strategyData.regrets);
}

double OOSAlgorithm::iteration(const shared_ptr<EFGNode> &h,
                      double rm_h_pl, double rm_h_opp, double rm_h_cn,
                      double bs_h_all, double us_h_all,
                      Player exploringPl) {

    stats_.nodesVisits++;
    double s_h_all = bias(bs_h_all, us_h_all);

    if (h->type_ == TerminalNode) {
        s_z_all_ = s_h_all;
        u_z_ = h->getUtilities()[exploringPl] * normalizingUtils;
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
        const auto probs = h->chanceProbs(); //

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

//        if (!(n instanceof GadgetChanceNode)) updateHistoryExpectedValue(expPlayer, cn,
//                                                                         u_h, // todo: undo effect of normalizing utils!!!
//                                                                         rm_h_pl, rm_h_opp, rm_h_cn, s_h_all);

        return u_h;
    }

    assert(h->type_ == PlayerNode);

    const auto &infoset = cache_.getInfosetFor(h);
    CFRData::InfosetData &data = cache_.infosetData.at(infoset);

    double u_h;  // baseline-augmented estimate of expected utility for current history
    double u_ha; // baseline-augmented estimate of expected utility for next history
    double u_x;  // baseline-augmented estimate of expected utility for next history,
    // if we go there with 100% probability from current history
    double rm_ha_all; // probability of taking this action (according to RM)

    if (h->getAOHInfSet() == playInfoset_) stats_.infosetVisits++;
    if (h->getPublicState() == playPublicState_) stats_.pubStateVisits++;

    if (!cache_.hasAnyChildren(h)) { // new node - incrementally build tree
        ai = pickUniform(numActions, generator_);
        rm_ha_all = 1.0 / numActions;
        const auto leaf = pickRandomLeaf(h, actions[ai], generator_);
        u_z_ = leaf.utilities[exploringPl];
        s_z_all_ = bias(bs_h_all, us_h_all) * leaf.chanceReachProb * rm_ha_all;
        s_ha_all = rm_ha_all;
        rm_zh_all_ = leaf.playerReachProbs[0] * leaf.playerReachProbs[1]; // todo: check chance pl!

        // compute replacement for baseline-augmented utilities
        u_ha = u_z_ / normalizingUtils;
        u_x = u_ha;
        u_h = u_x;
    } else {
        calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);
        const auto&[biasApplicableActions, bsum] = calcBiasing(h, infoset, bs_h_all, numActions);
        u_h = 0.;

        int xxxUpdatingAction; // todo: refactor
        if (h->getPlayer() == exploringPl) { // exploring move
            tie(ai, us_ha_all) = selectExploringPlayerAction(numActions,
                                                             biasApplicableActions, bsum);
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

            xxxUpdatingAction = 0;
        } else {
            tie(ai, us_ha_all) = selectNonExploringPlayerAction(h, bsum);
            bs_ha_all = (*pBiasedProbs_)[ai] / bsum;
            rm_ha_all = rmProbs_[ai];

            // precompute baseline components now, because after child iteration RM probs will change
            for (int i = 0; i < numActions; ++i) {
                if (i == ai) continue;
                u_h += rmProbs_[i] * baseline(h, i);
            }
            xxxUpdatingAction = 1;
        }

        const auto &nextNode = cache_.getChildFor(h, actions[ai]);
        u_ha = iteration(nextNode,
                         (xxxUpdatingAction == 0) ? rm_h_pl * rm_ha_all : rm_h_pl,
                         (xxxUpdatingAction == 1) ? rm_h_opp * rm_ha_all : rm_h_opp,
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

    updateHistoryExpectedValue(exploringPl, h,
                               u_h, // todo: undo effect of normalizing utils!!!
                               rm_h_pl, rm_h_opp, rm_h_cn, s_h_all);

    updateInfosetRegrets(h, exploringPl, data,
                         ai, rm_ha_all,
                         u_z_, u_x, u_h,
                         rm_h_cn, rm_h_opp, rm_zha_all, s_h_all);

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

// todo:
//    // gadget biasing -- gadget chance node is always on the top of the tree, so no "underTargetIs" is applicable
//    if (cn instanceof GadgetChanceNode && gadgetDelta > 0 && trackingIS != null) {
//        GadgetChanceNode gcn = (GadgetChanceNode) cn;
//        double bsum = gcn.getBiasedProbs(biasedProbs, trackingIS, gadgetEpsilon, gadgetDelta);
//        int i = randomChoice(biasedProbs, bsum);
//        return new Pair<>(cn.getActions().get(i), biasedProbs[i]);
//    }

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


pair<int, double> OOSAlgorithm::selectExploringPlayerAction(int numActions,
                                                   int biasApplicableActions, double bsum) {
    int ai;
    double us_ha_all;

    // todo: gadget infoset
    if (!isBiasedIteration_) { // No biasing
        ai = dist_(generator_) <= cfg_.exploration
             ? pickUniform(numActions, generator_)    // with exploration
             : pickRandom(rmProbs_, generator_); // no exploration
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

    double pai = rmProbs_[ai];
    us_ha_all = explore(1. / numActions, pai);

    return make_pair(ai, us_ha_all);
}

pair<int, double> OOSAlgorithm::selectNonExploringPlayerAction(const shared_ptr<EFGNode> &h, double bsum) {
    int ai;
    double us_ha_all;

    // todo: gadget infoset
    if (isBiasedIteration_) ai = pickRandom(*pBiasedProbs_, bsum, generator_);
    else ai = pickRandom(rmProbs_, generator_);
    us_ha_all = rmProbs_[ai];

    return make_pair(ai, us_ha_all);
}

void OOSAlgorithm::updateHistoryExpectedValue(Player exploringPl, const shared_ptr<EFGNode> &h,
                                     double u_h,
                                     double rm_h_pl, double rm_h_opp, double rm_h_cn,
                                     double s_h_all) {

    // let's make sure that the utility is always for player 0
    // updateVal we get is for the exploring player
    u_h *= exploringPl == Player(0) ? 1 : -1;

    const auto &baselineIdx = cache_.baselineValues_.find(h);
    assert(baselineIdx != cache_.baselineValues_.end());
    OOSData::Baseline data = baselineIdx->second;

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
        default:
            break;
    }

    data.nominator += a;
    data.denominator += b;
}

void OOSAlgorithm::updateInfosetRegrets(const shared_ptr<EFGNode> &h, Player exploringPl,
                               CFRData::InfosetData &data,
                               int ai, double pai,
                               double u_z, double u_x, double u_h,
                               double rm_h_cn, double rm_h_opp, double rm_zha_all, double s_h_all) {
    auto &reg = data.regrets;

    if (h->getPlayer() == exploringPl) {
        // todo: gadget infoset
        double w = rm_h_opp * rm_h_cn / s_h_all;

        if (cfg_.regretMatching == OOSSettings::RegretMatchingPlus) {
            for (int i = 0; i < reg.size(); i++) {
                if (i == ai) reg[i] = fmax(0, reg[i] + (u_x - u_h) * w);
                else {
                    // todo: check player!
                    reg[i] = fmax(0, reg[i] + (baseline(h,i) - u_h) * w);
                }
            }
        } else {
            for (int i = 0; i < reg.size(); i++) {
                if (i == ai) reg[i] += (u_x - u_h) * w;
                else {
                    // todo: check player!
                    reg[i] += (baseline(h,i) - u_h) * w;
                }
            }
        }
    } else {
        // we use stochastically weighted averaging
        calcRMProbs(data.regrets, &rmProbs_, cfg_.approxRegretMatching);

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
        }

        for (int i = 0; i < reg.size(); i++) {
            data.avgStratAccumulator[i] += w * rm_h_opp * rm_h_cn / s_h_all * reg[i];
        }
    }
}

#undef baseline

}  // namespace GTLib2
