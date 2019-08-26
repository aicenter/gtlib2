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
#include "Exp3LSelector.h"

namespace GTLib2::algorithms {

void Exp3LSelector::updateProb() {
    for (int i=0; i < actionsNumber_; i++) {
        double denom = 1;
        for (int j=0 ; j < actionsNumber_; j++) {
            if (i != j) denom += exp(-sqrt(lnActionsNumber_ / (currentIteration_*actionsNumber_)) * (rewards_[j] - rewards_[i]));
        }
        const double cp = (1 / denom);
        actionProbability_[i] = cp;
        actionMeanProbability_[i]+=cp;
    }
}

ActionId Exp3LSelector::select() {
    if (currentIteration_>0) updateProb();

    double rand = pickRandomDouble(fact_->getRandom());

    for (int i=0; i<actionsNumber_; i++) {
        if (rand > actionProbability_[i]) {
            rand -= actionProbability_[i];
        } else {
            return i;
        }
    }
    unreachable("no action selected!");
}

void Exp3LSelector::update(ActionId ai, double value) {
    rewards_[ai] += (1 - fact_->normalizeValue(value)) / actionProbability_[ai];
    currentIteration_++;
}

ProbDistribution Exp3LSelector::getActionsProbDistribution() {
    return normalizeProbability(actionMeanProbability_);
}
}