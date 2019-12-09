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

#include "algorithms/cfr.h"

#include "algorithms/common.h"
#include "algorithms/utility.h"


namespace GTLib2::algorithms {

CFRAlgorithm::CFRAlgorithm(const Domain &domain,
                           Player playingPlayer,
                           CFRData &cache,
                           CFRSettings settings) :
    GamePlayingAlgorithm(domain, playingPlayer),
    cache_(cache),
    settings_(move(settings)) {}

PlayControl CFRAlgorithm::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (currentInfoset == PLAY_FROM_ROOT && !cache_.isCompletelyBuilt()) cache_.buildTree();

    // the tree has been built before, we must have this infoset in memory
    assert(currentInfoset == PLAY_FROM_ROOT || !cache_.getNodesFor(*currentInfoset).empty());

    runIteration(cache_.getRootNode(), array<double, 3>{1., 1., 1.}, Player(0));
    updateInfosetRegrets(Player(0));

    runIteration(cache_.getRootNode(), array<double, 3>{1., 1., 1.}, Player(1));
    updateInfosetRegrets(Player(1));

    return ContinueImproving;
}

optional<ProbDistribution>
CFRAlgorithm::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
    const auto &data = cache_.infosetData.at(currentInfoset);
    const auto &acc = data.avgStratAccumulator;
    return calcAvgProbs(acc);
}

void CFRAlgorithm::updateInfosetRegrets(Player updatingPl) {
    if (settings_.cfrUpdating != InfosetsUpdating) return;

    for (auto&[aoh, data]:cache_.infosetData) {
        if (aoh->getPlayer() != updatingPl) continue;

        if (settings_.accumulatorWeighting == LinearAccWeighting && !data.fixAvgStrategy) {
            ++data.numUpdates;
        }

        if (!data.fixRMStrategy) {
            for (int i = 0; i < data.regrets.size(); ++i) {
                (settings_.regretMatching == RegretMatchingNormal)
                ? data.regrets[i] += data.regretUpdates[i] :
                    data.regrets[i] += max(data.regretUpdates[i], 0.0);

                data.regretUpdates[i] = 0.0;
            }
        }
    }
}

void CFRAlgorithm::runIterations(int numIterations) {
    if (!cache_.isCompletelyBuilt()) {
        cache_.buildTree();
    }

    if (settings_.regretMatching == RegretMatchingPlus) {
        assert(settings_.cfrUpdating == InfosetsUpdating
                   && settings_.accumulatorWeighting == LinearAccWeighting);
    }

    for (int i = 0; i < numIterations; ++i) {
        runIteration(Player(0));
        updateInfosetRegrets(Player(0));

        runIteration(Player(1));
        updateInfosetRegrets(Player(1));
    }
}


double CFRAlgorithm::runIteration(const shared_ptr<EFGNode> &node,
                                  const array<double, 3> reachProbs,
                                  const Player updatingPl) {
    assert(cache_.isCompletelyBuilt());
    if (reachProbs[0] == 0 && reachProbs[1] == 0) return 0.0;
    if (node->type_ == TerminalNode) return node->getUtilities()[updatingPl];
    if (node->type_ == ChanceNode) {
        const auto &children = cache_.getChildrenFor(node);
        double ExpectedValue = 0.0;
        auto chanceProbs = node->chanceProbs();

        for (int i = 0; i != children.size(); i++) {
            array<double, 3> newReachProbs = {reachProbs[0],
                                              reachProbs[1],
                                              reachProbs[CHANCE_PLAYER] * chanceProbs[i]};
            ExpectedValue += chanceProbs[i] * runIteration(children[i], newReachProbs, updatingPl);
        }
        return ExpectedValue;
    }

    assert(node->type_ == PlayerNode);
    const auto actingPl = node->getPlayer();
    const auto oppExploringPl = 1 - updatingPl;
    const auto &children = cache_.getChildrenFor(node);
    const auto &infoSet = cache_.getInfosetFor(node);
    const auto numActions = children.size();
    auto &infosetData = cache_.infosetData.at(infoSet);
    auto &reg = infosetData.regrets;
    auto &acc = infosetData.avgStratAccumulator;
    auto &regUpdates = infosetData.regretUpdates;
    auto rmProbs = calcRMProbs(reg);
    auto ChildrenActionValues = vector<double>(numActions, 0.0);
    double ExpectedValue = 0.0;

    for (int i = 0; i != children.size(); i++) {
        array<double, 3> newReachProbs = {reachProbs[0],
                                          reachProbs[1],
                                          reachProbs[CHANCE_PLAYER]};
        newReachProbs[actingPl] *= rmProbs[i];

        ChildrenActionValues[i] += runIteration(children[i], newReachProbs, updatingPl);
        ExpectedValue += rmProbs[i] * ChildrenActionValues[i];
    }

    if (actingPl == updatingPl) {
        for (int i = 0; i < numActions; i++) {
            if (!infosetData.fixRMStrategy) {
                double cfActionRegret = (ChildrenActionValues[i] - ExpectedValue)
                    * reachProbs[oppExploringPl] * reachProbs[CHANCE_PLAYER];
                ((settings_.cfrUpdating == HistoriesUpdating) ? reg[i] : regUpdates[i]) +=
                    cfActionRegret;
            }

            if (!infosetData.fixAvgStrategy) {
                acc[i] += reachProbs[updatingPl] * rmProbs[i];
            }
        }
    }

    return ExpectedValue;
}

void calcRMProbs(const vector<double> &regrets, ProbDistribution *pProbs, double epsilonUniform) {
    assert(regrets.size() <= pProbs->size());
    assert(!regrets.empty());
    assert(!pProbs->empty());
    assert(epsilonUniform >= 0.);
    assert(epsilonUniform <= 1.);
    const auto n = regrets.size();

    double posRegretSum = 0.0;
    for (double r : regrets) {
        posRegretSum += max(0.0, r);
    }

    if (posRegretSum > 0) {
        for (int i = 0; i < n; i++) {
            (*pProbs)[i] = (1 - epsilonUniform) * max(0.0, regrets[i] / posRegretSum)
                + epsilonUniform / n;
        }
    } else {
        std::fill(pProbs->begin(), pProbs->begin() + n, 1.0 / n);
    }
}

void calcAvgProbs(const vector<double> &acc, ProbDistribution *pProbs) {
    assert(acc.size() <= pProbs->size());
    assert(acc.size() > 0);
    assert(pProbs->size() > 0);
#ifndef NDEBUG
    for (auto prob: acc) assert(prob >= 0.0);
#endif

    double sum = 0.0;
    for (double d : acc) sum += d;

    for (int i = 0; i < acc.size(); ++i) {
        (*pProbs)[i] = sum == 0.0
                       ? 1.0 / acc.size()
                       : acc[i] / sum;
    }
}

ExpectedUtility calcExpectedUtility(CFRData &cache, const shared_ptr<EFGNode> &node, Player pl) {
    switch (node->type_) {
        case ChanceNode: {
            const auto &children = cache.getChildrenFor(node);
            double rmUtility = 0., avgUtility = 0.;
            const auto chanceProbs = node->chanceProbs();
            for (int i = 0; i < children.size(); ++i) {
                auto childUtils = calcExpectedUtility(cache, children[i], pl);
                rmUtility += chanceProbs[i] * childUtils.rmUtility;
                avgUtility += chanceProbs[i] * childUtils.avgUtility;
            }
            return ExpectedUtility(rmUtility, avgUtility);
        }
        case PlayerNode: {
            const auto &children = cache.getChildrenFor(node);
            const auto &infoSet = cache.getInfosetFor(node);
            auto &infosetData = cache.infosetData.at(infoSet);

            double rmUtility = 0., avgUtility = 0.;
            auto rmProbs = calcRMProbs(infosetData.regrets);
            auto avgProbs = calcAvgProbs(infosetData.avgStratAccumulator);

            for (int i = 0; i < children.size(); ++i) {
                auto childUtils = calcExpectedUtility(cache, children[i], pl);
                rmUtility += rmProbs[i] * childUtils.rmUtility;
                avgUtility += avgProbs[i] * childUtils.avgUtility;
            }
            return ExpectedUtility(rmUtility, avgUtility);
        }
        case TerminalNode:
            return ExpectedUtility(node->getUtilities()[pl], node->getUtilities()[pl]);
        default:
            unreachable("unrecognized option!");
    }

}

}  // namespace GTLib2
