//
// Created by Pavel Rytir on 1/13/18.
//

#ifndef PURSUIT_TREEWALK_H
#define PURSUIT_TREEWALK_H


#include "../base/base.h"
#include "../base/efg.h"

namespace GTLib2 {
    namespace algorithms {

        void
        pavelEFGTreeWalk(const Domain &domain, std::function<void(shared_ptr<EFGNode>, double)> function, int maxDepth);


        // Counts the number of nodes of the game given by the domain parameter.
        // Does not include the initial dummy chance node above the root states.
        int countNodes(const Domain &domain);
    }
}
#endif //PURSUIT_TREEWALK_H
