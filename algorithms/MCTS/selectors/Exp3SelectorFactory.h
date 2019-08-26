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

#ifndef GTLIB2_EXP3SELECTORFACTORY_H
#define GTLIB2_EXP3SELECTORFACTORY_H

#include "selectorFactory.h"

namespace GTLib2::algorithms {
class Exp3SelectorFactory: public SelectorFactory {
 public:
    const double gamma = 0.05;
    const bool storeExploration = false; //not used in Exp3L
    const bool useExp3L = false;

    Exp3SelectorFactory(double minUtility, double maxUtility, double gamma,
                        bool storeExploration, std::mt19937 random) :
        minUtility_(minUtility), maxUtility_(maxUtility), gamma(gamma),
        storeExploration(storeExploration), generator_(random) {}
    Exp3SelectorFactory(double minUtility, double maxUtility, double gamma,
                        bool storeExploration, int seed) :
        minUtility_(minUtility), maxUtility_(maxUtility), gamma(gamma),
        storeExploration(storeExploration) { generator_ = std::mt19937(seed); }
    Exp3SelectorFactory(double minUtility, double maxUtility, double gamma, bool storeExploration) :
        minUtility_(minUtility), maxUtility_(maxUtility), gamma(gamma),
        storeExploration(storeExploration) { generator_ = std::mt19937(0); }
    Exp3SelectorFactory(bool useExp3L, double minUtility, double maxUtility, double gamma,
                        std::mt19937 random) :
        minUtility_(minUtility), maxUtility_(maxUtility), gamma(gamma), useExp3L(useExp3L),
        generator_(random) {}
    Exp3SelectorFactory(bool useExp3L, double minUtility, double maxUtility, double gamma, int seed)
        : minUtility_(minUtility), maxUtility_(maxUtility), gamma(gamma), useExp3L(useExp3L) {
        generator_ = std::mt19937(seed);
    }
    Exp3SelectorFactory(bool useExp3L, double minUtility, double maxUtility, double gamma) :
        minUtility_(minUtility), maxUtility_(maxUtility), gamma(gamma), useExp3L(useExp3L) {
        generator_ = std::mt19937(0);
    }

    unique_ptr<Selector> createSelector(int actionsNumber) const override;
    unique_ptr<Selector> createSelector(vector<shared_ptr<Action>> actions) const override;
    std::mt19937 getRandom() const override;

    double normalizeValue(double value) const;
 private:
    std::mt19937 generator_;
    const double minUtility_;
    const double maxUtility_;
};
}

#endif //GTLIB2_EXP3SELECTORFACTORY_H
