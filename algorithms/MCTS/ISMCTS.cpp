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

#include "ISMCTS.h"

namespace GTLib2::algorithms {

PlayControl ISMCTS::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if ((currentInfoset != PLAY_FROM_ROOT)
        && (infosetSelectors_.find(*currentInfoset) == infosetSelectors_.end()))
        return GiveUp; // unexplored IS reached

    iteration(rootNode_);
    return ContinueImproving;
}

double ISMCTS::iteration(const shared_ptr<EFGNode> &h) {
    switch (h->type_) {
        case TerminalNode:
            return handleTerminalNode(h);
        case ChanceNode:
            return handleChanceNode(h);
        case PlayerNode:
            return handlePlayerNode(h);
        default:
            unreachable("unrecognized option!");
    }
}

double ISMCTS::handleTerminalNode(const shared_ptr<EFGNode> &h) {
    return h->getUtilities()[playingPlayer_];
}

double ISMCTS::handleChanceNode(const shared_ptr<EFGNode> &h) {
    const int selectedIndex = pickRandom(*h, generator_);
    const shared_ptr<Action> selectedAction = h->getActionByID(selectedIndex);
    const auto child = h->performAction(selectedAction);
    return iteration(child);
}

double ISMCTS::handlePlayerNode(const shared_ptr<EFGNode> &h) {
    const auto infoset = h->getAOHInfSet();
    const auto selectorPtr = infosetSelectors_.find(infoset);
    Selector *selector;
    int actionIndex;
    double simulationResult;
    if (selectorPtr == infosetSelectors_.end()) {
        infosetSelectors_.emplace(infoset, factory_->createSelector(h->availableActions()));
        selector = infosetSelectors_[infoset].get();
        actionIndex = selector->select();
        simulationResult = simulate(h);
    } else {
        selector = selectorPtr->second.get();
        actionIndex = selector->select();
        shared_ptr<Action> selectedAction = h->getActionByID(actionIndex);
        const auto child = h->performAction(selectedAction);
        simulationResult = iteration(child);
    }
    const int sign = h->getPlayer() == playingPlayer_ ? 1 : -1;
    selector->update(actionIndex, sign * simulationResult);
    return simulationResult;
}

double ISMCTS::simulate(const shared_ptr<EFGNode> &h) {
    shared_ptr<EFGNode> currentNode = h;
    while (currentNode->type_ != TerminalNode) {
        const int selectedIndex = pickRandom((*currentNode), generator_);
        shared_ptr<Action> selectedAction = currentNode->getActionByID(selectedIndex);
        currentNode = currentNode->performAction(selectedAction);
    }
    return currentNode->getUtilities()[playingPlayer_];
}

optional<ProbDistribution> ISMCTS::getPlayDistribution(const shared_ptr<AOH> &currentInfoset) {
    const auto selectorPtr = infosetSelectors_.find(currentInfoset);
    if (selectorPtr != infosetSelectors_.end()) {
        return selectorPtr->second->getActionsProbDistribution();
    }
    return nullopt;
}

}
