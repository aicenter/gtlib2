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

#include "base/random.h"


namespace GTLib2 {

int pickRandom(const ProbDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator);
    int i = -1;
    while (p > 0) p -= probs[++i];
    assert(i < probs.size());
    return i;
}

int pickRandom(const EFGNodesDistribution &probs,
               std::uniform_real_distribution<double> &uniformDist,
               std::mt19937 &generator) {
    double p = uniformDist(generator);
    int i = -1;
    while (p > 0) p -= probs[++i].second;
    assert(i < probs.size());
    return i;
}

}  // namespace std
