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


#include <utility>

#include "UCTSelectorFactory.h"

namespace GTLib2 {
    class UCTSelector : public Selector {
public:
    UCTSelector(vector<shared_ptr<Action>> actions, UCTSelectorFactory * fact) : fact_(fact) {
        actions_ = std::move(actions); values_ = vector<double>(actions_.size()); visits_ = vector<int>(actions_.size());}
    UCTSelector(int N, UCTSelectorFactory * fact) : fact_(fact) {
        values_ = vector<double>(N); visits_ = vector<int>(N);}

    int select() override;
    void update(int ai, double value) override;
    ProbDistribution getActionsProbDistribution() override ;
private:
    UCTSelectorFactory * fact_;
    vector<double > values_;
    vector<int> visits_;
    int n = 0;

    double getBestRateIndex();

    int getBestRateCount(double eps, double bestVal);
};

}

#endif //GTLIB2_UCTSELECTOR_H
