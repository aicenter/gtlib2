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
#include "Exp3Selector.h"

namespace GTLib2::algorithms {

    void Exp3Selector::updateProb() {
        const int K = rewards_.size();
        for (int i=0; i < K; i++) {
            double denom = 1;
            for (int j=0 ; j < K; j++) {
                if (i != j) denom += exp((gamma_ / K) * (rewards_[j] - rewards_[i]));
            }
            __const double cp = (1 / denom);
            actionProbability_[i] = (1 - gamma_) * cp + gamma_ / K;
            if (fact_->storeExploration) actionMeanProbability_[i]+=actionProbability_[i];
            else actionMeanProbability_[i] += cp;
        }
    }

    int Exp3Selector::select() {
        updateProb();
        double rand = pickRandomDouble(fact_->getRandom());
        for (int i=0; i<actionProbability_.size(); i++) {
            if (rand > actionProbability_[i]) {
                rand -= actionProbability_[i];
            } else {
                return i;
            }
        }

        assert (false);
        return -1;
    }

    void Exp3Selector::update(int ai, double value) {
        rewards_[ai] += fact_->normalizeValue(value) / actionProbability_[ai];
    }

    ProbDistribution Exp3Selector::getActionsProbDistribution() {
        return actionMeanProbability_;
    }
}