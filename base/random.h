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


#include "base/efg.h"

namespace GTLib2 {

extern std::uniform_real_distribution<double> uniformDist;

static int pickRandomNumber(int min, int max, std::mt19937 rand)
{
    int res = rand() % (max + 1 - min) + min;
    return res;
}

static double pickRandomDouble(std::mt19937 rand)
{
    double d =  uniformDist(rand);
    return d;
}

/**
 * Return index of which event should be picked according to given probability distribution
 */
int pickRandom(const ProbDistribution &probs, std::mt19937 &generator);

int pickRandom(const Distribution &probs, double probSum, std::mt19937 &generator);

int pickRandom(const EFGNode &node, std::mt19937 &generator);

int pickUniform(unsigned long numOutcomes, std::mt19937 &generator);


struct RandomLeafOutcome {
    vector<double> utilities;
    vector<double> playerReachProbs;
    double chanceReachProb;

    inline double reachProb() const { return chanceReachProb * playersReachProb(); }
    inline double playersReachProb() const {
        return std::accumulate(playerReachProbs.begin(), playerReachProbs.begin(),
                               1, std::multiplies<>());
    }
};

RandomLeafOutcome pickRandomLeaf(const std::shared_ptr<EFGNode> &start, std::mt19937 &generator);

RandomLeafOutcome pickRandomLeaf(const std::shared_ptr<EFGNode> &start,
                                 const std::shared_ptr<GTLib2::Action> &firstAction,
                                 std::mt19937 &generator);

}  // namespace std


#endif //GTLIB2_RANDOM_H
