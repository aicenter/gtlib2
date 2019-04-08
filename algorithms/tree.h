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

#ifndef ALGORITHMS_TREEWALK_H_
#define ALGORITHMS_TREEWALK_H_

#include <memory>
#include "base/base.h"
#include "base/efg.h"
#include "base/cache.h"

namespace GTLib2 {
namespace algorithms {

typedef std::function<void(shared_ptr<EFGNode>)> EFGNodeCallback;

/**
 * Call supplied function at each EFGNode of the EFG tree, including leaves.
 * The tree is walked as DFS up to maximum specified depth.
 */
void treeWalkEFG(const Domain &domain, EFGNodeCallback function, int maxDepth);

/**
 * Call supplied function at each EFGNode of the EFG tree supplied by cache, including leaves.
 * The tree is walked as DFS up to maximum specified depth.
 */
void treeWalkEFG(EFGCache *cache, EFGNodeCallback function, int maxDepth);

/**
 * Call supplied function at each EFGNode of the EFG tree supplied by cache, including leaves.
 * The tree is walked as DFS up to maximum depth as the max of int values.
 */
inline void treeWalkEFG(EFGCache *cache, EFGNodeCallback function) {
    treeWalkEFG(cache, move(function), INT_MAX);
}

/**
 * Call supplied function at each EFGNode of the EFG tree, including leaves.
 * The tree is walked as DFS up to maximum depth allowed by the domain.
 */
inline void treeWalkEFG(const Domain &domain, EFGNodeCallback function) {
    treeWalkEFG(domain, move(function), domain.getMaxDepth());
}

}  // namespace algorithms
}  // namespace GTLib2
#endif  // ALGORITHMS_TREEWALK_H_
