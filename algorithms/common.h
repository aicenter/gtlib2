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


#ifndef ALGORITHMS_COMMON_H_
#define ALGORITHMS_COMMON_H_

#include "base/efg.h"

namespace GTLib2::algorithms {

BehavioralStrategy mixedToBehavioralStrategy(const Domain &domain,
                                             const vector<BehavioralStrategy> &pureStrats,
                                             const ProbDistribution &distribution,
                                             Player player);

bool isAOCompatible(const vector<ActionObservationIds> &aoTarget,
                    const vector<ActionObservationIds> &aoCmp);

template<typename T>
bool isCompatible(const vector<T> &target, const vector<T> &cmp) {
    auto sizeTarget = target.size();
    auto sizeCmp = cmp.size();
    if (min(sizeTarget, sizeCmp) == 0) return true;

    size_t cmpBytes = min(sizeTarget, sizeCmp) * sizeof(T);
    return !memcmp(target.data(), cmp.data(), cmpBytes);
}

vector<shared_ptr<EFGNode>> getAllNodesInInfoset(const shared_ptr<AOH> &infoset,
                                                 const Domain &domain);

}  // namespace GTLib2

#endif  // ALGORITHMS_COMMON_H_

