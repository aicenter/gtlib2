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


void CFRiterations(CFRData *data, int numIterations) {
    // todo: check that tree is built
    for (int i = 0; i < numIterations; ++i) {
        for (const auto &[node, prob] : data->getRootNodes()) {
            CFRiteration(data, node, std::array<double, 2>{1., prob}, Player(0));
            CFRiteration(data, node, std::array<double, 2>{prob, 1.}, Player(1));
        }
    }
}

double CFRiteration(CFRData *data,
                    const shared_ptr<EFGNode> &node,
                    const std::array<double, 2> pi,
                    const Player exploringPl) {

    if (pi[0] == 0 && pi[1] == 0) {
        return 0.0;
    }

    if (node->isTerminal()) {
        return node->rewards_[exploringPl];
    }

    const int actingPl = *node->getCurrentPlayer();
    const int oppExploringPl = 1 - exploringPl;
    const auto &children = data->getChildrenFor(node);
    const auto &infoSet = data->getInfosetFor(node);
    const unsigned long numActions = children.size();
    auto &infosetData = data->infosetData;

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

    for (int ai = 0; ai != children.size(); ai++) {
        for (const auto &[nextNode, prob] : *children[ai]) {
            // let's put chance probs into opponent's reach probs.
            std::array<double, 2> new_pi = {pi[0], pi[1]};
            new_pi[oppExploringPl] *= prob;
            new_pi[actingPl] *= rmProbs[ai];

            double incr = prob * CFRiteration(data, nextNode, new_pi, exploringPl);
            cfvAction[ai] += incr;
        }
        cfvInfoset += rmProbs[ai] * cfvAction[ai];
    }

    if (actingPl == exploringPl) {
        for (int i = 0; i < numActions; i++) {
            reg[i] += (cfvAction[i] - cfvInfoset) * pi[oppExploringPl];
            acc[i] += pi[exploringPl] * rmProbs[i];
        }
    }

    return cfvInfoset;
}

}  // namespace algorithms
}  // namespace GTLib2
