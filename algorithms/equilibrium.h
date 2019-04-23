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


#ifndef ALGORITHMS_EQUILIBRIUM_H_
#define ALGORITHMS_EQUILIBRIUM_H_

#include "base/base.h"
#include <tuple>

namespace GTLib2::algorithms {

tuple<double, BehavioralStrategy> findEquilibriumTwoPlayersZeroSum(const Domain &domain);

}  // namespace GTLib2

#endif  // ALGORITHMS_EQUILIBRIUM_H_
