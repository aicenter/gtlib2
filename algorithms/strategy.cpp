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
        auto acc = data->infosetData.at(infoSet).avgStratAccumulator;

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

}  // namespace algorithms
}  // namespace GTLib2
