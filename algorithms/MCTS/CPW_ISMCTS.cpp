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
        if (giveUp_)
            return GiveUp;
        if (currentInfoset == nullopt)
        {
            //setCurrentInfoset(*currentInfoset);
            iteration(rootNode_);
            return ContinueImproving;
        }
        if (infosetSelectors_.find(*currentInfoset)  == infosetSelectors_.end())
            return GiveUp;
        const auto nodes = nodesMap_.find(*currentInfoset);
        if (nodes == nodesMap_.end())
            return GiveUp;
        if (currentInfoset_ != *currentInfoset) setCurrentInfoset(*currentInfoset);
        const int nodeIndex = pickRandomNumber(0, nodes->second.size() - 1, generator_);
        if (config_.useBelief) {
            for (int i=0; i<1+100*belief_[nodeIndex]; i++){
                const auto n = nodes->second[nodeIndex];
                iteration(n);
            }
        } else {
            iteration(nodes->second[nodeIndex]);
        }
        return ContinueImproving;
    }

    double CPW_ISMCTS::handlePlayerNode(const shared_ptr<EFGNode> &h){
        const auto it = nodesMap_.find(h->getAOHInfSet());
        if (it == nodesMap_.end()) nodesMap_[h->getAOHInfSet()] = {h};
        else {
            const auto nodes = nodesMap_[h->getAOHInfSet()];
            bool flag = false;
            for (auto const &node: nodes)
            {
                if (*node == *h) {
                    flag = true;
                    break;
                }
            }
            if (!flag) nodesMap_[h->getAOHInfSet()].push_back(h);
        }
        return ISMCTS::handlePlayerNode(h);
    }

    void CPW_ISMCTS::setCurrentInfoset(const shared_ptr<AOH> &newInfoset) {
        if (config_.useBelief)
        {
            const auto newNodesIt = nodesMap_.find(newInfoset);
            if (currentInfoset_ == nullptr)
            {
                if (newNodesIt == nodesMap_.end())
                {
                    // unexplored IS reached
                    giveUp_ = true;
                    return;
                }
                const auto newNodes = newNodesIt->second;
                belief_ = vector<double>(newNodes.size());
                fillBelief(rootNode_, newInfoset, 1, newNodes);
                double sum=0;
                for (double d : belief_) sum +=d;
                for (int i=0;i<belief_.size();i++) belief_[i] /= sum;
                currentInfoset_ = newInfoset;
                return;
            }
            const auto oldBelief = belief_;
            const auto oldNodesIt = nodesMap_.find(currentInfoset_);
            if (oldNodesIt == nodesMap_.end() || newNodesIt == nodesMap_.end())
            {
                // unexplored IS reached
                giveUp_ = true;
                return;
            }
            const auto oldNodes = oldNodesIt->second;
            const auto newNodes = newNodesIt->second;
            belief_ = vector<double>(newNodes.size());
            for (int i=0; i<oldNodes.size(); i++){
                fillBelief(oldNodes[i], newInfoset, oldBelief[i], newNodes);
            }
            //normalize belief
            double sum=0;
            for (double d : belief_) sum +=d;
            assert(sum > 0);
            for (int i=0;i<belief_.size();i++) belief_[i] /= sum;
        }
        currentInfoset_ = newInfoset;
    }

    void CPW_ISMCTS::fillBelief(const shared_ptr <EFGNode> &currentNode, const shared_ptr <AOH> &newInfoset,
                                const double prob, vector<shared_ptr<EFGNode>> newNodes)
    {
        if (currentNode->type_ == PlayerNode)
        {
            const auto currentInfoset = currentNode->getAOHInfSet();
            if (*currentInfoset == *newInfoset)
            {
                int i = 0;
                for (auto n : newNodes)
                {
                    if (*n == *currentNode)
                    {
                        belief_[i] += prob;
                        return;
                    }
                    i++;
                }
                return; // reached undiscovered node in newInfoset
            }
            if (!algorithms::isAOCompatible(currentNode->getAOids(newInfoset->getPlayer()), newInfoset->getAOids()))
                return;
            const auto it = infosetSelectors_.find(currentInfoset);
            if (it == infosetSelectors_.end())
                return;
            const auto distribution = it->second->getActionsProbDistribution();
            for (int i = 0; i < currentNode->availableActions().size(); i++)
            {
                const auto action = currentNode->availableActions()[i];
                fillBelief(currentNode->performAction(action), newInfoset, prob * distribution[i], newNodes);
            }
        }
        if (currentNode->type_ == ChanceNode)
        {
            for (int i = 0; i < currentNode->availableActions().size(); i++)
            {
                const auto action = currentNode->availableActions()[i];
                fillBelief(currentNode->performAction(action), newInfoset,prob * currentNode->chanceProbForAction(action), newNodes);
            }
        }
    }
}