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


#include <array>

#include "algorithms/cfr.h"
#include "algorithms/tree.h"
#include "algorithms/common.h"
#include "algorithms/utility.h"

using std::make_pair;
using std::max;

namespace GTLib2 {
namespace algorithms {


void CFRiterations(CFRData &data, int numIterations) {
    // todo: check that tree is built
    for (int i = 0; i < numIterations; ++i) {
        for (const auto &[node, chanceProb] : data.getRootNodes()) {
            CFRiteration(data, node, std::array<double, 3>{1., 1., chanceProb}, Player(0));
            CFRiteration(data, node, std::array<double, 3>{1., 1., chanceProb}, Player(1));
        }
    }
}

constexpr int CHANCE_PLAYER = 2;

double CFRiteration(CFRData &data,
                    const shared_ptr<EFGNode> &node,
                    const std::array<double, 3> reachProbs,
                    const Player updatingPl) {

    if (reachProbs[0] == 0 && reachProbs[1] == 0) {
        return 0.0;
    }

    if (node->isTerminal()) {
        return node->rewards_[updatingPl];
    }

    const auto actingPl = *node->getCurrentPlayer();
    const auto oppExploringPl = 1 - updatingPl;
    const auto &children = data.getChildrenFor(node);
    const auto &infoSet = data.getInfosetFor(node);
    const auto numActions = children.size();
    auto &infosetData = data.infosetData;

    if (infosetData.find(infoSet) == infosetData.end()) {
        infosetData.emplace(make_pair(infoSet, CFRData::InfosetData(numActions)));
    }
    auto&[reg, acc] = infosetData.at(infoSet);

    double posRegretSum = 0.0;
    for (double r : reg) {
        posRegretSum += max(0.0, r);
    }
    auto rmProbs = vector<double>(numActions);
    if (posRegretSum > 0) {
        for (int i = 0; i < numActions; i++) {
            rmProbs[i] = max(0.0, reg[i] / posRegretSum);
        }
    } else {
        std::fill(rmProbs.begin(), rmProbs.end(), 1.0 / numActions);
    }
    auto cfvAction = vector<double>(numActions);
    double cfvInfoset = 0.0;
    std::fill(cfvAction.begin(), cfvAction.end(), 0.0);

    for (int i = 0; i != children.size(); i++) {
        for (const auto &[nextNode, chanceProb] : *children[i]) {
            std::array<double, 3> newReachProbs = {
                reachProbs[0], reachProbs[1], reachProbs[CHANCE_PLAYER]};
            newReachProbs[CHANCE_PLAYER] *= chanceProb;
            newReachProbs[actingPl] *= rmProbs[i];

            cfvAction[i] += chanceProb * CFRiteration(data, nextNode, newReachProbs, updatingPl);
        }
        cfvInfoset += rmProbs[i] * cfvAction[i];
    }

    if (actingPl == updatingPl) {
        for (int i = 0; i < numActions; i++) {
            reg[i] += (cfvAction[i] - cfvInfoset) * reachProbs[oppExploringPl] * reachProbs[CHANCE_PLAYER];
            acc[i] += reachProbs[updatingPl] * rmProbs[i];
        }
    }

    return cfvInfoset;
}

}  // namespace algorithms
}  // namespace GTLib2
