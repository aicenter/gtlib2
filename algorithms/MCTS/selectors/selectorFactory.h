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

#ifndef GTLIB2_SELECTORFACTORY_H
#define GTLIB2_SELECTORFACTORY_H
#include "selector.h"
namespace GTLib2::algorithms {
class SelectorFactory {
public:
    virtual unique_ptr<Selector> createSelector(vector<shared_ptr<Action>> actions) const = 0;

    virtual unique_ptr<Selector> createSelector(int actionsNumber) const = 0;

    virtual std::mt19937 getRandom() const = 0;
};
}
#endif //GTLIB2_SELECTORFACTORY_H
