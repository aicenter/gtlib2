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

#include "DD_ISMCTS.h"
namespace GTLib2::algorithms {
PlayControl DD_ISMCTS::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if  (isCurrentISUndiscovered)
    {
        iteration(rootNode_);
        return ContinueImproving;
    }

    if (currentInfoset == nullopt) {
        iteration(rootNode_);
        return ContinueImproving;
    }

    if (infosetSelectors_.find(*currentInfoset) == infosetSelectors_.end())
    {
        isCurrentISUndiscovered = true;
        iteration(rootNode_);
        return ContinueImproving;
    }

    const auto nodes = nodesMap_.find(*currentInfoset);
    if (nodes == nodesMap_.end())
    {
        isCurrentISUndiscovered = true;
        iteration(rootNode_);
        return ContinueImproving;
    }

    if (currentInfoset_ != *currentInfoset) setCurrentInfoset(*currentInfoset);

    const int nodeIndex = config_.useBelief
                          ? pickRandom(belief_, generator_)
                          : pickRandomInt(0, nodes->second.size() - 1, generator_);
    iteration(nodes->second[nodeIndex]);

    return ContinueImproving;
}
}