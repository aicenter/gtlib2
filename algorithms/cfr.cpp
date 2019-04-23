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

#include "base/base.h"
#include "algorithms/cfr.h"

#include <array>
#include <algorithm>
#include <utility>
#include <functional>
#include <cstdio>

#include "algorithms/tree.h"
#include "algorithms/common.h"
#include "algorithms/utility.h"


namespace GTLib2::algorithms {

CFRAlgorithm::CFRAlgorithm(const Domain &domain, Player playingPlayer, CFRSettings settings) :
    GamePlayingAlgorithm(domain, playingPlayer),
    cache_(CFRData(domain_, settings.cfrUpdating)),
    settings_(settings) {}

PlayControl CFRAlgorithm::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (currentInfoset == nullopt) {
        if (cache_.isCompletelyBuilt()) return StopImproving;
        cache_.buildForest();
        return StopImproving;
    }

    // the tree has been built before, we must have this infoset in memory
    assert(!cache_.getNodesFor(*currentInfoset).empty());

    for (const auto &[node, chanceProb] : cache_.getRootNodes()) {
        runIteration(node, std::array<double, 3>{1., 1., chanceProb}, Player(0));
        delayedApplyRegretUpdates();

        runIteration(node, std::array<double, 3>{1., 1., chanceProb}, Player(1));
        delayedApplyRegretUpdates();
    }

    return ContinueImproving;
}

optional<ProbDistribution>
CFRAlgorithm::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
    const auto &data = cache_.infosetData.at(currentInfoset);
    const auto &acc = data.avgStratAccumulator;
    return calcAvgProbs(acc);
}


void CFRAlgorithm::nodeUpdateRegrets(shared_ptr<EFGNode> node) {
    if (node->isTerminal()) return;

    const auto &infoSet = cache_.getInfosetFor(node);
    auto &infosetData = cache_.infosetData;
    auto &data = infosetData.at(infoSet);
    for (int i = 0; i < data.regrets.size(); ++i) {
        data.regrets[i] += data.regretUpdates[i];
        data.regretUpdates[i] = 0.;
    }
}

void CFRAlgorithm::delayedApplyRegretUpdates() {
    if (settings_.cfrUpdating == InfosetsUpdating) {
        algorithms::treeWalkEFG(cache_, [&](shared_ptr<EFGNode> node) { nodeUpdateRegrets(node); });
    }
}


void CFRAlgorithm::runIterations(int numIterations) {
    if (!cache_.isCompletelyBuilt()) {
        cache_.buildForest();
    }

    for (int i = 0; i < numIterations; ++i) {
        for (const auto &[node, chanceProb] : cache_.getRootNodes()) {
            runIteration(node, std::array<double, 3>{1., 1., chanceProb}, Player(0));
            delayedApplyRegretUpdates();

            runIteration(node, std::array<double, 3>{1., 1., chanceProb}, Player(1));
            delayedApplyRegretUpdates();
        }
    }
}


double CFRAlgorithm::runIteration(const shared_ptr<EFGNode> &node,
                                  const std::array<double, 3> reachProbs,
                                  const Player updatingPl) {
    assert(cache_.isCompletelyBuilt());

    if (reachProbs[0] == 0 && reachProbs[1] == 0) {
        return 0.0;
    }

    if (node->isTerminal()) {
        return node->rewards_[updatingPl];
    }

    const auto actingPl = *node->getCurrentPlayer();
    const auto oppExploringPl = 1 - updatingPl;
    const auto &children = cache_.getChildrenFor(node);
    const auto &infoSet = cache_.getInfosetFor(node);
    const auto numActions = children.size();
    auto &infosetData = cache_.infosetData.at(infoSet);
    auto &reg = infosetData.regrets;
    auto &acc = infosetData.avgStratAccumulator;
    auto &regUpdates = infosetData.regretUpdates;

    auto rmProbs = calcRMProbs(reg);
    auto cfvAction = vector<double>(numActions);
    double cfvInfoset = 0.0;
    std::fill(cfvAction.begin(), cfvAction.end(), 0.0);

    for (int i = 0; i != children.size(); i++) {
        for (const auto &[nextNode, chanceProb] : *children[i]) {
            std::array<double, 3> newReachProbs = {
                reachProbs[0], reachProbs[1], reachProbs[CHANCE_PLAYER]};
            newReachProbs[CHANCE_PLAYER] *= chanceProb;
            newReachProbs[actingPl] *= rmProbs[i];

            cfvAction[i] += chanceProb * runIteration(nextNode, newReachProbs, updatingPl);
        }
        cfvInfoset += rmProbs[i] * cfvAction[i];
    }

    if (actingPl == updatingPl) {
        for (int i = 0; i < numActions; i++) {
            if (!infosetData.fixRMStrategy) {
                ((settings_.cfrUpdating == HistoriesUpdating) ? reg[i] : regUpdates[i]) +=
                    (cfvAction[i] - cfvInfoset)
                        * reachProbs[oppExploringPl] * reachProbs[CHANCE_PLAYER];
            }
            if (!infosetData.fixAvgStrategy) {
                acc[i] += reachProbs[updatingPl] * rmProbs[i];
            }
        }
    }

    return cfvInfoset;
}

vector<double> calcRMProbs(const vector<double> &regrets) {
    double posRegretSum = 0.0;
    for (double r : regrets) {
        posRegretSum += max(0.0, r);
    }

    auto rmProbs = vector<double>(regrets.size());
    if (posRegretSum > 0) {
        for (int i = 0; i < regrets.size(); i++) {
            rmProbs[i] = max(0.0, regrets[i] / posRegretSum);
        }
    } else {
        std::fill(rmProbs.begin(), rmProbs.end(), 1.0 / regrets.size());
    }
    return rmProbs;
}

vector<double> calcAvgProbs(const vector<double> &acc) {
    double sum = 0.0;
    for (double d : acc) sum += d;

    vector<double> probs = vector<double>(acc.size());
    for (int i = 0; i < acc.size(); ++i) {
        probs[i] = sum == 0.0
                   ? 1.0 / acc.size()
                   : acc[i] / sum;
    }
    return probs;
}

ExpectedUtility calcExpectedUtility(CFRData &cache,
                                    const shared_ptr<EFGNode> &node,
                                    Player pl) {
    if (node->isTerminal()) {
        return ExpectedUtility(node->rewards_[pl], node->rewards_[pl]);
    }

    const auto &children = cache.getChildrenFor(node);
    const auto &infoSet = cache.getInfosetFor(node);
    auto &infosetData = cache.infosetData.at(infoSet);

    double rmUtility = 0.;
    double avgUtility = 0.;
    auto rmProbs = calcRMProbs(infosetData.regrets);
    auto avgProbs = calcAvgProbs(infosetData.avgStratAccumulator);

    for (int i = 0; i != children.size(); i++) {
        for (const auto &[nextNode, chanceProb] : *children[i]) {
            auto childUtils = calcExpectedUtility(cache, nextNode, pl);
            rmUtility += chanceProb * rmProbs[i] * childUtils.rmUtility;
            avgUtility += chanceProb * avgProbs[i] * childUtils.avgUtility;
        }
    }
    return ExpectedUtility(rmUtility, avgUtility);
}

}  // namespace GTLib2
