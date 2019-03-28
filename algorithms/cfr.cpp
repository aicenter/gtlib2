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
#include "algorithms/tree.h"
#include "algorithms/common.h"
#include "algorithms/utility.h"

using std::make_pair;
using std::max;

namespace GTLib2 {
namespace algorithms {


StrategyProfile getAverageStrategy(CFRData *data) {
    auto profile = StrategyProfile(2);
    auto numInfosets = static_cast<unsigned long>(data->countAugInfosets() / 2);
    data->infosetData.reserve(numInfosets);

    auto getStrategy = [&profile, &data](shared_ptr<EFGNode> node) {
        if (node->isTerminal()) return;

        auto infoSet = node->getAOHInfSet();
        Player curPlayer = *node->getCurrentPlayer();
        BehavioralStrategy *playerStrategy = &profile[curPlayer];
        if (playerStrategy->find(infoSet) != playerStrategy->end()) return;

        auto actionDistribution = unordered_map<shared_ptr<Action>, double>();
        auto acc = std::get<1>(data->infosetData.at(infoSet));

        double sum = 0.0;
        for (double d : acc) sum += d;

        for (auto &action : node->availableActions()) {
            actionDistribution.emplace(make_pair(action, sum == 0.0
                                                         ? 1.0 / acc.size()
                                                         : acc[action->getId()] / sum));
        }
        playerStrategy->emplace(infoSet, actionDistribution);
    };
    treeWalkEFG(data, getStrategy, INT_MAX);

    return profile;
}

void CFRiterations(CFRData *data, int numIterations) {
    // todo: check that tree is built
    for (int i = 0; i < numIterations; ++i) {
        for (const auto &[node, prob] : data->getRootNodes()) {
            CFRiteration(data, node, new double[2]{1., prob}, Player(0));
            CFRiteration(data, node, new double[2]{prob, 1.}, Player(1));
        }
    }
}

double CFRiteration(CFRData *data,
                    const shared_ptr<EFGNode> &node,
                    const double pi[2],
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
    const unsigned long K = children.size();
    auto &infosetData = data->infosetData;

    if (infosetData.find(infoSet) == infosetData.end()) {
        infosetData.emplace(make_pair(infoSet, make_pair(
            CFRData::Regrets(K, 0.0), CFRData::AvgStratAccumulator(K, 0.0))));
    }
    auto&[reg, acc] = infosetData.at(infoSet);

    double R = 0.0;
    for (double ri : reg) {
        R += max(0.0, ri);
    }
    auto rmProbs = new double[K];
    if (R > 0) {
        for (int i = 0; i < K; i++) {
            rmProbs[i] = max(0.0, reg[i] / R);
        }
    } else {
        std::fill_n(rmProbs, K, 1.0 / K);
    }
    auto cfva = new double[K];
    double cfvInfoset = 0.0;
    std::fill_n(cfva, K, 0.0);

    for (int ai = 0; ai != children.size(); ai++) {
        for (const auto &[nextNode, prob] : *children[ai]) {
            // let's put chance probs into opponent's reach probs.
            double new_pi[2] = {pi[0], pi[1]};
            new_pi[oppExploringPl] *= prob;
            new_pi[actingPl] *= rmProbs[ai];

            double incr = prob * CFRiteration(data, nextNode, new_pi, exploringPl);
            cfva[ai] += incr;
        }
        cfvInfoset += rmProbs[ai] * cfva[ai];
    }

    if (actingPl == exploringPl) {
        for (int i = 0; i < K; i++) {
            reg[i] += (cfva[i] - cfvInfoset) * pi[oppExploringPl];
            acc[i] += pi[exploringPl] * rmProbs[i];
        }
    }

    return cfvInfoset;
}

}  // namespace algorithms
}  // namespace GTLib2
