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

#ifndef GTLIB2_RMSELECTORFACTORY_H
#define GTLIB2_RMSELECTORFACTORY_H
#include "selectorFactory.h"

namespace GTLib2::algorithms {
class RMSelectorFactory : public SelectorFactory {
public:
    const double gamma = 0.2;
    explicit RMSelectorFactory(double gamma, double minUtility, double maxUtility, std::mt19937 random): gamma(gamma), minUtility_(minUtility), maxUtility_(maxUtility), generator_(random) {};
    explicit RMSelectorFactory(double gamma, double minUtility, double maxUtility, int seed): gamma(gamma), minUtility_(minUtility), maxUtility_(maxUtility) { generator_ = std::mt19937(seed);};
    explicit RMSelectorFactory(double gamma, double minUtility, double maxUtility): gamma(gamma), minUtility_(minUtility), maxUtility_(maxUtility) { generator_ = std::mt19937();};
    unique_ptr<Selector> createSelector(int actionsNumber) const override;
    unique_ptr<Selector> createSelector(vector<shared_ptr<Action>> actions) const override;
    std::mt19937 getRandom() const override;

    double normalizeValue(double value) const;

private:
    std::mt19937 generator_;
    const double minUtility_=-1;
    const double maxUtility_=1;
};
}

#endif //GTLIB2_RMSELECTORFACTORY_H
