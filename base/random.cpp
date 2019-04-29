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

#include <utility>
#include "base/random.h"
#include "math.h"

namespace GTLib2 {

int pickRandom(const ProbDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator);
    int i = -1;
    while (p > 0) p -= probs[++i];
    assert(i < probs.size());
    return i;
}

int pickRandom(const Distribution &probs, double probSum,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator)*probSum;
    int i = -1;
    while (p > 0) p -= probs[++i];
    assert(i < probs.size());
    return i;
}

int pickRandom(const EFGNodesDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator);
    int i = -1;
    while (p > 0) p -= probs[++i].second;
    assert(i < probs.size());
    return i;
}

int pickUniform(unsigned long numOutcomes,
                std::uniform_real_distribution<double> &uniformDist,
                std::mt19937 &generator) {
    double p = uniformDist(generator);
    int idxOutcome = floor(p * numOutcomes);
    if (idxOutcome == numOutcomes) idxOutcome--; // if p == 1.0
    assert(idxOutcome < numOutcomes);
    assert(idxOutcome >= 0);
    return idxOutcome;
}


RandomLeafOutcome pickRandomLeaf(const std::shared_ptr<EFGNode> &start,
                                 std::uniform_real_distribution<double> &uniformDist,
                                 std::mt19937 &generator) {
    RandomLeafOutcome out = {
        .utilities = vector<double>(0.),
        .playerReachProbs = vector<double>{1., 1.},
        .chanceReachProb = 1.
    };

    const EFGNode *h = &(*start);

    while (!h->isTerminal()) {
//        if (h->getStateDepth() == simStateDepth_) break;
        const auto &actions = h->availableActions();
        auto numActions = actions.size();
        int ai = GTLib2::pickUniform(numActions, uniformDist, generator);
        out.playerReachProbs[*h->getCurrentPlayer()] *= 1.0 / numActions;

        const GTLib2::EFGNodesDistribution children = h->performAction(actions[ai]);
        ai = pickRandom(children, uniformDist, generator);
        h = &(*children[ai].first);
        out.chanceReachProb *= children[ai].second;
    }

    out.utilities = h->rewards_;
    return out;
}

RandomLeafOutcome pickRandomLeaf(const std::shared_ptr<EFGNode> &start,
                                 const std::shared_ptr<GTLib2::Action> &firstAction,
                                 std::uniform_real_distribution<double> &uniformDist,
                                 std::mt19937 &generator) {

    const GTLib2::EFGNodesDistribution children = start->performAction(firstAction);
    int ai = pickRandom(children, uniformDist, generator);
    auto out = pickRandomLeaf(children[ai].first, uniformDist, generator);
    out.chanceReachProb *= children[ai].second;

    return out;
}

}  // namespace std
