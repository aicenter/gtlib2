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


#include "RMSelector.h"
#include "RMSelectorFactory.h"

namespace GTLib2::algorithms {
    unique_ptr<Selector> RMSelectorFactory::createSelector(int actionsNumber) {
        generator_ = std::mt19937();
        return make_unique<RMSelector>(actionsNumber, this);
    }

    unique_ptr<Selector> RMSelectorFactory::createSelector(vector<shared_ptr<Action>> actions) {
        return make_unique<RMSelector>(actions, this);
    }

    std::mt19937 RMSelectorFactory::getRandom() {
        return generator_;
    }

    double RMSelectorFactory::normalizeValue(double value) {
        assert (minUtility_ <= value + 1e-5 && value <= maxUtility_ + 1e-5);
        return (value - minUtility_) / (maxUtility_ - minUtility_);
    }

}