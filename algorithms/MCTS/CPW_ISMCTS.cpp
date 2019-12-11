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

#include "CPW_ISMCTS.h"

namespace GTLib2::algorithms {

PlayControl CPW_ISMCTS::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
    if (currentInfoset == PLAY_FROM_ROOT) {
        iteration(rootNode_);
        return ContinueImproving;
    }

    if (currentInfoset_ != *currentInfoset) setCurrentInfoset(*currentInfoset);

    if (config_.enableHistoryGeneration && !currentISChecked_) {
        hgNodeGenerator_(dynamic_cast<const ConstrainingDomain &>(domain_), currentInfoset_, config_.hgBudgetType, config_.hgBudget,
                         [this](const shared_ptr<EFGNode> &node) -> double {
                             return this->iteration(node);
                         });
        currentISChecked_ = true;
    }

    const auto nodes = nodesMap_.find(*currentInfoset);
    if (infosetSelectors_.find(*currentInfoset) == infosetSelectors_.end() || nodes == nodesMap_.end()) {
        if (config_.iterateRoot) iteration(rootNode_);
        else return GiveUp;
    }

    const int nodeIndex = config_.useBelief
                          ? pickRandom(belief_, generator_)
                          : pickRandomInt(0, nodes->second.size() - 1, generator_);
    iteration(nodes->second[nodeIndex]);

    return ContinueImproving;
}

double CPW_ISMCTS::handlePlayerNode(const shared_ptr<EFGNode> &h) {
    const auto it = nodesMap_.find(h->getAOHInfSet());
    if (it == nodesMap_.end()) nodesMap_[h->getAOHInfSet()] = {h};
    else {
        const auto nodes = it->second;
        bool isNodeInMap = false;
        for (auto const &node: nodes) {
            if (*node == *h) {
                isNodeInMap = true;
                break;
            }
        }
        if (!isNodeInMap) nodesMap_[h->getAOHInfSet()].push_back(h);
    }
    return ISMCTS::handlePlayerNode(h);
}

void CPW_ISMCTS::setCurrentInfoset(const shared_ptr<AOH> &newInfoset) {
    if (config_.useBelief) {
        const auto newNodesIt = nodesMap_.find(newInfoset);
        vector<shared_ptr<EFGNode>> oldNodes;
        if (currentInfoset_ == nullptr) {
            oldNodes = {rootNode_};
        } else {
            oldNodes = nodesMap_.find(currentInfoset_)->second;
        }
        const auto oldBelief = belief_;
        const vector<shared_ptr<EFGNode>> newNodes = newNodesIt->second;

        belief_ = vector<double>(newNodes.size());
        for (unsigned long i = 0; i < oldNodes.size(); i++) {
            fillBelief(oldNodes[i], newInfoset, oldBelief[i], newNodes);
        }
        //normalize belief
        double sum = 0;
        for (double d : belief_) sum += d;
        assert(sum > 0);
        for (double & i : belief_) i /= sum;
    }
    currentInfoset_ = newInfoset;
}

void CPW_ISMCTS::fillBelief(const shared_ptr<EFGNode> &currentNode,
                            const shared_ptr<AOH> &newInfoset,
                            const double reachProbability,
                            const vector<shared_ptr<EFGNode>> &newNodes) {

    if (currentNode->type_ == PlayerNode) {
        const auto currentInfoset = currentNode->getAOHInfSet();
        if (*currentInfoset == *newInfoset) {
            for (int i = 0; i < newNodes.size(); i++) {
                auto n = newNodes.at(i);
                if (*n == *currentNode) {
                    belief_[i] = reachProbability;
                    return;
                }
            }

            return; // reached undiscovered node in newInfoset
        }
        if (!algorithms::isAOCompatible(currentNode->getAOids(newInfoset->getPlayer()),
                                        newInfoset->getAOids()))
            return;
        const auto it = infosetSelectors_.find(currentInfoset);
        if (it == infosetSelectors_.end())
            return;
        const auto distribution = it->second->getActionsProbDistribution();
        for (int i = 0; i < currentNode->countAvailableActions(); i++) {
            const auto action = currentNode->getActionByID(i);
            fillBelief(currentNode->performAction(action),
                       newInfoset,
                       reachProbability * distribution[i],
                       newNodes);
        }
    }

    if (currentNode->type_ == ChanceNode) {
        for (int i = 0; i < currentNode->countAvailableActions(); i++) {
            const auto action = currentNode->getActionByID(i);
            fillBelief(currentNode->performAction(action),
                       newInfoset,
                       reachProbability * currentNode->chanceProbForAction(action),
                       newNodes);
        }
    }
}
}
