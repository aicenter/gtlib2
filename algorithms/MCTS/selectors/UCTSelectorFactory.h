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

#ifndef GTLIB2_UCTSELECTORFACTORY_H
#define GTLIB2_UCTSELECTORFACTORY_H
#include "selectorFactory.h"


namespace GTLib2::algorithms {
class UCTSelectorFactory : public SelectorFactory {
public:
    const double c;
    explicit UCTSelectorFactory(double c, std::mt19937 random): c(c), generator_(random) {};
    explicit UCTSelectorFactory(double c, int seed): c(c) {generator_ = std::mt19937(seed);};
    explicit UCTSelectorFactory(double c): c(c) {generator_ = std::mt19937();};
    unique_ptr<Selector> createSelector(int actionsNumber) override;
    unique_ptr<Selector> createSelector(vector<shared_ptr<Action>> actions) override;
    std::mt19937 getRandom() override;
private:
    std::mt19937 generator_;
};

}
#endif //GTLIB2_UCTSELECTORFACTORY_H
