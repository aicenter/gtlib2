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



#include "base/base.h"
#include "utils/range.h"

#include "gtest/gtest.h"

#define EXPECT_VEC_DOUBLE_EQ(vec1, vec2)       \
    for(unsigned int i = 0; i < vec1.size(); i++) {     \
        const auto v1 = vec1[i];               \
        const auto v2 = vec2[i];               \
        EXPECT_DOUBLE_EQ(v1, v2);              \
    }


namespace GTLib2::utils {

TEST(Range, LogRange) {
    EXPECT_VEC_DOUBLE_EQ(logRange(1., 10000., 5, 10.), (vector<double>{1., 10., 100., 1000., 10000.}));
    EXPECT_VEC_DOUBLE_EQ(logRange(10., 10000., 4, 10.), (vector<double>{10., 100., 1000., 10000.}));
    EXPECT_VEC_DOUBLE_EQ(logRange(10., 100., 3, 10.),   (vector<double>{10., 31.622776601683793, 100.}));
}

}
