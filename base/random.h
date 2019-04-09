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


#ifndef GTLIB2_RANDOM_H
#define GTLIB2_RANDOM_H


#include "base/base.h"
#include "base/efg.h"
#include <random>

namespace GTLib2 {

/**
 * Return index of which event should be picked according to given probability distribution
 */
int pickRandom(const ProbDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator);


int pickRandom(const EFGNodesDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator);

inline int pickRandom(const ProbDistribution &probs, std::mt19937 &generator) {
    auto uniformDist = std::uniform_real_distribution<double>(0.0, 1.0);
    return pickRandom(probs, uniformDist, generator);
}

inline int pickRandom(const EFGNodesDistribution &probs, std::mt19937 &generator) {
    auto uniformDist = std::uniform_real_distribution<double>(0.0, 1.0);
    return pickRandom(probs, uniformDist, generator);
}

}  // namespace std


#endif //GTLIB2_RANDOM_H
