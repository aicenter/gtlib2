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


#ifndef GTLIB2_CFR_CR_H
#define GTLIB2_CFR_CR_H

#include "algorithms/continualResolving.h"
#include "algorithms/oos.h"


namespace GTLib2::algorithms {

/**
 * Use CFR for continual resolving
 */
class CFR_CR: public ContinualResolving, public CFRAlgorithm {
 public:
    inline CFR_CR(const Domain &domain,
                  Player playingPlayer,
                  OOSData &cache,
                  CFRSettings settings)
        : ContinualResolving(domain, playingPlayer, cache),
          CFRAlgorithm(domain, playingPlayer, cache, settings) {}

 protected:
    PlayControl preplayIteration(const shared_ptr<EFGNode> &rootNode) override;
    PlayControl resolveIteration(const shared_ptr<GadgetRootNode> &rootNode,
                                 const shared_ptr<AOH> &currentInfoset) override;
};

};

#endif //GTLIB2_CFR_CR_H
