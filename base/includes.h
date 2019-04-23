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


#ifndef GTLIB2_INCLUDES_H
#define GTLIB2_INCLUDES_H

#include <iostream>
#include <array>
#include <boost/functional/hash.hpp>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <limits>
#include <vector>
#include <iterator>
#include <random>
#include <stdlib.h>
#include <stdint.h>


namespace GTLib2 {

using std::array;
using std::cerr;
using std::cout;
using std::dynamic_pointer_cast;
using std::endl;
using std::function;
using std::make_pair;
using std::make_shared;
using std::make_unique;
using std::max;
using std::min;
using std::move;
using std::nullopt;
using std::optional;
using std::pair;
using std::reference_wrapper;
#define ref_wrap reference_wrapper
using std::shared_ptr;
using std::string;
using std::tie;
using std::to_string;
using std::tuple;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

}

#endif //GTLIB2_INCLUDES_H
