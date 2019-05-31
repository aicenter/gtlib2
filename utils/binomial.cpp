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

    You should have received a copy of the GNU Lesser General Public
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#include "binomial.h"

namespace GTLib2::utils {
    unsigned  int binomial_coefficient(unsigned int n, unsigned int k){
        int res = 1;

        for (unsigned int i = n; i > k; i--) {
            res *= i;
        }

        for (unsigned int i = 2; i <= n - k; i++) {
            res /= i;
        }
        return res;
    }
}
