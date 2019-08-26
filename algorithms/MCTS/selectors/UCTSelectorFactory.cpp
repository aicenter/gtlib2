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

#include "UCTSelector.h"
#include "UCTSelectorFactory.h"
namespace GTLib2::algorithms {

    unique_ptr<Selector> UCTSelectorFactory::createSelector(int actionsNumber) const {
        return make_unique<UCTSelector>(actionsNumber, this);
    }

    unique_ptr<Selector> UCTSelectorFactory::createSelector(vector<shared_ptr<Action>> actions) const {
        return make_unique<UCTSelector>(actions, this);
    }

    std::mt19937 UCTSelectorFactory::getRandom() const {
        return generator_;
    }
}