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


#include "strategy.h"

namespace GTLib2 {
namespace algorithms {


StrategyProfile getAverageStrategy(CFRData &data, int maxDepth) {
    auto profile = StrategyProfile(2);
    auto getStrategy = [&profile, &data](shared_ptr<EFGNode> node) {
        if (node->isTerminal()) return;

        auto infoSet = node->getAOHInfSet();
        Player curPlayer = *node->getCurrentPlayer();
        BehavioralStrategy *playerStrategy = &profile[curPlayer];
        if (playerStrategy->find(infoSet) != playerStrategy->end()) return;

        auto acc = data.infosetData.at(infoSet).avgStratAccumulator;
        playerStrategy->emplace(infoSet, calcAvgProbs(acc));
    };
    treeWalkEFG(data, getStrategy, maxDepth);

    return profile;
}


StrategyProfile getUniformStrategy(InfosetCache &data, int maxDepth) {
    auto profile = StrategyProfile(2);
    auto getStrategy = [&profile, &data](shared_ptr<EFGNode> node) {
        if (node->isTerminal()) return;

        auto infoSet = node->getAOHInfSet();
        Player curPlayer = *node->getCurrentPlayer();
        BehavioralStrategy *playerStrategy = &profile[curPlayer];
        if (playerStrategy->find(infoSet) != playerStrategy->end()) return;

        unsigned long numActions = node->countAvailableActions();
        playerStrategy->emplace(infoSet, ProbDistribution(numActions, 1. / numActions));
    };
    treeWalkEFG(data, getStrategy, maxDepth);

    return profile;
}

void playOnlyAction(ProbDistribution &dist, unsigned long actionIdx) {
    assert(actionIdx < dist.size() && actionIdx >= 0);
    for (int i = 0; i < dist.size(); ++i) {
        dist[i] = i == actionIdx ? 1.0 : 0.0;
    }
}

}  // namespace algorithms
}  // namespace GTLib2
