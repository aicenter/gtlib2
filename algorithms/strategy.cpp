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

namespace GTLib2::algorithms {


StrategyProfile getAverageStrategy(CFRData &data, int maxDepth) {
    auto profile = StrategyProfile(2);
    auto getStrategy = [&profile, &data](shared_ptr<EFGNode> node) {
        if (node->type_ != PlayerNode) return;

        auto infoSet = node->getAOHInfSet();
        Player curPlayer = node->getPlayer();
        BehavioralStrategy *playerStrategy = &profile[curPlayer];
        if (playerStrategy->find(infoSet) != playerStrategy->end()) return;

        auto acc = data.infosetData.at(infoSet).avgStratAccumulator;
        playerStrategy->emplace(
            infoSet, mapDistribution(calcAvgProbs(acc), node->availableActions()));
    };
    treeWalkEFG(data, getStrategy, maxDepth);

    return profile;
}


StrategyProfile getUniformStrategy(InfosetCache &data, int maxDepth) {
    auto profile = StrategyProfile(2);
    auto getStrategy = [&profile, &data](shared_ptr<EFGNode> node) {
        if (node->type_ != PlayerNode) return;

        auto infoSet = node->getAOHInfSet();
        Player curPlayer = node->getPlayer();
        BehavioralStrategy *playerStrategy = &profile[curPlayer];
        if (playerStrategy->find(infoSet) != playerStrategy->end()) return;

        auto actions = node->availableActions();
        auto numActions = actions.size();
        playerStrategy->emplace(
            infoSet, mapDistribution(
                ProbDistribution(numActions, 1. / numActions), actions));
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

void playOnlyAction(ActionProbDistribution &dist, const shared_ptr<Action> &action) {
    dist.clear();
    dist[action] = 1.0;
}

ActionProbDistribution mapDistribution(const ProbDistribution &dist,
                                       const vector<shared_ptr<Action>> &actions) {
    assert(dist.size() == actions.size());
    ActionProbDistribution actionDist;
    for (int i = 0; i < dist.size(); ++i) {
        actionDist[actions[i]] = dist[i];
    }
    return actionDist;
}

}  // namespace GTLib2
