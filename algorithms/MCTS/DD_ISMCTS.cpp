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
    if (currentInfoset == PLAY_FROM_ROOT)  {
        iteration(rootNode_);
        return ContinueImproving;
    }

    if (currentInfoset_ != *currentInfoset) {
        setCurrentInfoset(*currentInfoset);
    }

    if (!currentISChecked_) {
        bool newFigureRevealed = dynamic_cast<const ConstrainingDomain &>(domain_).updateConstraints(
            currentInfoset_,
            lastRevealAoid_,
            revealed_);
        if (newFigureRevealed)
            dynamic_cast<const ConstrainingDomain&>(domain_).generateNodes(currentInfoset_, revealed_, generateIters_,
                                                                        [this](const shared_ptr<EFGNode> & node) -> double{ return this->iteration(node);});

        currentISChecked_ = true;
    }

    const auto nodes = nodesMap_.find(currentInfoset_);
    if (infosetSelectors_.find(currentInfoset_) == infosetSelectors_.end()
            || nodes == nodesMap_.end() || nodes->second.size() == 0) {
        isCurrentISUndiscovered_ = true;
        if (iterateRoot_) iteration(rootNode_);
        return ContinueImproving;
    }

    isCurrentISUndiscovered_ = false;
    const int nodeIndex = pickRandomInt(0, nodes->second.size() - 1, generator_);
    iteration(nodes->second[nodeIndex]);
    return ContinueImproving;
}

optional<ProbDistribution> DD_ISMCTS::getPlayDistribution(const shared_ptr<AOH> &currentInfoset,
                                                            const long actionsNum) {
    if (isCurrentISUndiscovered_) {
        auto v = vector<double>(actionsNum);
        std::fill(v.begin(), v.end(), 1.0/actionsNum);
        return v;
    }
    return ISMCTS::getPlayDistribution(currentInfoset, actionsNum);
}

void DD_ISMCTS::setCurrentInfoset(const shared_ptr<AOH> &newInfoset) {
    currentInfoset_ = newInfoset;
    currentISChecked_ = false;
}
}
