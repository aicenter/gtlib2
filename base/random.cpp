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

#include "base/random.h"

#include <math.h>

namespace GTLib2 {

std::uniform_real_distribution<double> uniformDist = std::uniform_real_distribution<double>(0.0, 1.0);

int pickRandom(const ProbDistribution &probs, std::mt19937 &generator) {
    if(probs[0] == 1.0) return 0; // do not use generator unnecessarily

    double p = uniformDist(generator);
    unsigned int i = 0;
    while (true) {
        p -= probs[i];
        if(p < 0) break;
        else ++i;
    }
    assert(i < probs.size());
    return i;
}

int pickRandom(const Distribution &probs, double probSum, std::mt19937 &generator) {
    if(probs[0] == probSum) return 0; // do not use generator unnecessarily

    double p = uniformDist(generator)*probSum;
    unsigned int i = 0;
    while (true) {
        p -= probs[i];
        if(p < 0) break;
        else ++i;
    }
    assert(i < probs.size());
    return i;
}

int pickUniform(unsigned long numOutcomes, std::mt19937 &generator) {
    if(numOutcomes == 1) return 0; // do not use generator unnecessarily

    double p = uniformDist(generator);
    unsigned int idxOutcome = floor(p * numOutcomes);
    if (idxOutcome == numOutcomes) idxOutcome--; // if p == 1.0
    assert(idxOutcome < numOutcomes);
    return idxOutcome;
}

int pickRandom(const EFGNode &node, std::mt19937 &generator) {
    switch (node.type_) {
        case ChanceNode:
            return pickRandom(node.chanceProbs(), generator);
        case PlayerNode:
            return pickUniform(node.countAvailableActions(), generator);
        case TerminalNode:
            unreachable("No actions!");
        default:
            unreachable("unrecognized option!");
    }
}


RandomLeafOutcome pickRandomLeaf(const std::shared_ptr<EFGNode> &start, std::mt19937 &generator) {
    RandomLeafOutcome out = {
        /*.utilities=*/vector<double>(),
        /*.playerReachProbs=*/vector<double>{1., 1.},
        /*.chanceReachProb=*/1.
    };

    std::shared_ptr<EFGNode> h = start;
    while (h->type_ != TerminalNode) {
        const auto &actions = h->availableActions();
        int ai = pickRandom(*h, generator);

        switch (h->type_) {
            case ChanceNode:
                out.chanceReachProb *= h->chanceProbForAction(actions[ai]);
                break;
            case PlayerNode:
                out.playerReachProbs[h->getPlayer()] *= 1.0 / actions.size();
                break;
            case TerminalNode:
                unreachable("terminal node!");
            default:
                unreachable("unrecognized option!");
        }

        h = h->performAction(actions[ai]);
    }

    out.utilities = h->getUtilities();
    return out;
}

RandomLeafOutcome pickRandomLeaf(const std::shared_ptr<EFGNode> &start,
                                 const std::shared_ptr<GTLib2::Action> &firstAction,
                                 std::mt19937 &generator) {
    auto out = pickRandomLeaf(start->performAction(firstAction), generator);

    switch (start->type_) {
        case ChanceNode:
            out.chanceReachProb *= start->chanceProbForAction(firstAction);
            break;
        case PlayerNode:
            out.playerReachProbs[start->getPlayer()] *= 1.0 / start->countAvailableActions();
            break;
        case TerminalNode:
            unreachable("terminal node!");
        default:
            unreachable("unrecognized option!");
    }

    return out;
}

}  // namespace std
