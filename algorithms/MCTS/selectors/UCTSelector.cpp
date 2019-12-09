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

#include "base/random.h"
#include "UCTSelector.h"

namespace GTLib2::algorithms {
void UCTSelector::update(ActionId ai, double value) {
    totalVisits++;
    visits_[ai]++;
    if (visits_[ai] == 1) {
        values_[ai] = value;
    } else {
        values_[ai] += (value - values_[ai]) / visits_[ai];
    }
}

double getUCBRate(double v, int ni, int n, double c) {
    return v + c * sqrt(log(n) / ni);
}

double UCTSelector::getBestRateIndex() {
    double bestVal = getUCBRate(values_[0], visits_[0], totalVisits, factory_->cfg_.c);
    int bestIdx = 0;

    for (int i = 1; i < values_.size(); i++) {
        double curVal = getUCBRate(values_[i], visits_[i], totalVisits, factory_->cfg_.c);

        if (curVal > bestVal) {
            bestVal = curVal;
            bestIdx = i;
        }
    }
    return bestIdx;
}

int UCTSelector::getBestRateCount(double eps, double bestVal) {
    int count = 0;
    for (int i = 0; i < values_.size(); i++) {
        double curVal = getUCBRate(values_[i], visits_[i], totalVisits, factory_->cfg_.c);
        if (curVal > bestVal - eps) count++;
    }
    return count;
}

ActionId UCTSelector::select() {
    if (totalVisits < values_.size()) {
        int j = pickRandomInt(1, values_.size() - totalVisits, factory_->getRandom());
        int i = -1;

        while (j > 0) {
            i++;
            if (visits_[i] == 0)
                j--;
        }
        return i;
    }
    const double epsilon = 0.01;
    const double bestIndex = getBestRateIndex();
    const double bestVal = getUCBRate(values_[bestIndex], visits_[bestIndex],
                                      totalVisits, factory_->cfg_.c);
    const int bestCount = getBestRateCount(epsilon, bestVal);
    int index = pickRandomInt(0, bestCount - 1, factory_->getRandom());
    // if there is a number of actions with the same value equal to the best, choose a random one
    for (int i = 0; i < values_.size(); i++) {
        const double curVal = getUCBRate(values_[i], visits_[i], totalVisits, factory_->cfg_.c);

        if (curVal >= bestVal - epsilon) {
            if (index == 0)
                return i;
            index--;
        }
    }
    unreachable("no action selected!");
}

ProbDistribution UCTSelector::getActionsProbDistribution() {
    ProbDistribution probs = vector<double>(visits_.size());
    for (int i = 0; i < visits_.size(); i++) {
        probs[i] = visits_[i] * 1.0 / totalVisits;
    }
    return probs;
}
}
