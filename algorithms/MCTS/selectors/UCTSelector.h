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

#ifndef GTLIB2_UCTSELECTOR_H
#define GTLIB2_UCTSELECTOR_H


#include "UCTSelectorFactory.h"

namespace GTLib2::algorithms {

class UCTSelector: public Selector {
 public:
    UCTSelector(const vector<shared_ptr<Action>> &actions, const UCTSelectorFactory *factory)
        : factory_(factory) {
        values_ = vector<double>(actions.size());
        visits_ = vector<int>(actions.size());
    }
    UCTSelector(int actionsNumber, const UCTSelectorFactory *factory) : factory_(factory) {
        values_ = vector<double>(actionsNumber);
        visits_ = vector<int>(actionsNumber);
    }

    ActionId select() override;
    void update(ActionId ai, double value) override;
    ProbDistribution getActionsProbDistribution() override;
 private:
    const UCTSelectorFactory *factory_;
    vector<double> values_;
    vector<int> visits_;
    unsigned int totalVisits = 0;

    double getBestRateIndex();

    int getBestRateCount(double eps, double bestVal);
};

}

#endif //GTLIB2_UCTSELECTOR_H
