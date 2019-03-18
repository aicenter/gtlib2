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

#include "base/base.h"
#include "base/efg.h"

namespace GTLib2 {
namespace algorithms {

void treeWalkEFG(const Domain &domain, std::function<void(shared_ptr<EFGNode>)> function,
                 int maxDepth);

void treeWalkEFG(const Domain &domain, std::function<void(shared_ptr<EFGNode>)> function);

// Counts the number of nodes of the game given by the domain parameter.
// Does not include the initial dummy chance node above the root states.
int countNodes(const Domain &domain);

int countNodesInfSetsSequencesStates(const Domain &domain);
}  // namespace algorithms
}  // namespace GTLib2
#endif  // ALGORITHMS_TREEWALK_H_
