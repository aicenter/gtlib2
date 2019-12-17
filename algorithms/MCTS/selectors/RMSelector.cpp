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


#include <base/random.h>
#include <algorithms/strategy.h>
#include "RMSelector.h"

namespace GTLib2::algorithms {

ActionId RMSelector::select() {
    updateProb();
    const auto numActions = actionProbability.size();
    double rand = pickRandomDouble(factory_->getRandom());
    for (unsigned int i = 0; i < numActions; i++) {
        const double pa = (1 - factory_->cfg_.gamma) * actionProbability[i] + factory_->cfg_.gamma / numActions;

        if (rand > pa) {
            rand -= pa;
        } else {
            return i;
        }
    }
    unreachable("no action selected!");
}

void RMSelector::update(ActionId ai, double value) {
    const double v = factory_->normalizeValue(value);
    const double pa = (1 - factory_->cfg_.gamma) * actionProbability[ai]
        + factory_->cfg_.gamma / actionProbability.size();
    regretEstimate[ai] += v / pa;
    for (unsigned int i = 0; i < regretEstimate.size(); i++) {
        regretEstimate[i] -= v;
    }
}

ProbDistribution RMSelector::getActionsProbDistribution() {
    return normalizeProbability(actionMeanProbability);
}

void RMSelector::updateProb() {
    const int K = regretEstimate.size();
    double R = 0;
    for (double ri : regretEstimate) R += (ri > 0 ? ri : 0);

    if (R <= 0) {
        std::fill(actionProbability.begin(), actionProbability.end(), 1.0 / K);
    } else {
        for (unsigned int i = 0; i < actionProbability.size(); i++)
            actionProbability[i] = (regretEstimate[i] > 0 ? regretEstimate[i] / R : 0);
    }

    for (unsigned int i = 0; i < actionProbability.size(); i++)
        actionMeanProbability[i] += actionProbability[i];
}
}
