//
// Created by Pavel Rytir on 1/13/18.
//

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
