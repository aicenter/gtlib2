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

#include "CPWISMCTS.h"

namespace GTLib2 {
    PlayControl CPWISMCTS::runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) {
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
        auto nodes = nodesMap_.find(*currentInfoset);
        if (nodes == nodesMap_.end())
            return GiveUp;
        if (currentInfoset_ != *currentInfoset) setCurrentInfoset(*currentInfoset);
        int nodeIndex = Selector::getRandomNumber(0, nodes->second.size() - 1, generator_);
        if (config_.useBelief) {
            for (int i=0; i<1+100*belief_[nodeIndex]; i++){
                auto n = nodes->second[nodeIndex];
                iteration(n);
            }
        } else {
            iteration(nodes->second[nodeIndex]);
        }
        return ContinueImproving;
    }

    double CPWISMCTS::handlePlayerNode(const shared_ptr<EFGNode> &h){
        auto it = nodesMap_.find(h->getAOHInfSet());
        if (it == nodesMap_.end()) nodesMap_[h->getAOHInfSet()] = {h};
        else {
            auto v = nodesMap_[h->getAOHInfSet()];
            bool f = false;
            for (auto const &vi: v)
            {
                if (*vi == *h) {
                    f = true;
                    break;
                }
            }
            if (!f) nodesMap_[h->getAOHInfSet()].push_back(h);
        }
        return ISMCTS::handlePlayerNode(h);
    }

    void CPWISMCTS::setCurrentInfoset(const shared_ptr<AOH> &newInfoset) {
        if (config_.useBelief)
        {
            if (currentInfoset_ == nullptr)
            {
                currentInfoset_ = newInfoset;
                auto newNodesIt = nodesMap_.find(newInfoset);
                if (newNodesIt == nodesMap_.end())
                {
                    // unexplored IS reached
                    currentInfoset_ = newInfoset;
                    giveUp_ = true;
                    return;
                }
                auto newNodes = newNodesIt->second;
                belief_ = vector<double>(newNodes.size());
                fillBelief(rootNode_, newInfoset, 1, newNodes);
                double sum=0;
                for (double d : belief_) sum +=d;
                for (int i=0;i<belief_.size();i++) belief_[i] /= sum;
                currentInfoset_ = newInfoset;
                return;
            }
            auto oldBelief = belief_;
            auto oldNodesIt = nodesMap_.find(currentInfoset_);
            auto newNodesIt = nodesMap_.find(newInfoset);
            if (oldNodesIt == nodesMap_.end() || newNodesIt == nodesMap_.end())
            {
                // unexplored IS reached
                giveUp_ = true;
                return;
            }
            auto oldNodes = oldNodesIt->second;
            auto newNodes = newNodesIt->second;
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

    void CPWISMCTS::fillBelief(const shared_ptr <EFGNode> &currentNode, const shared_ptr <AOH> &newInfoset,
                               const double prob, vector<shared_ptr<EFGNode>> newNodes)
    {
        if (currentNode->type_ == PlayerNode)
        {
            auto currentInfoset = currentNode->getAOHInfSet();
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
                //assert (false);
                return;
            }
            if (!algorithms::isAOCompatible(currentNode->getAOids(newInfoset->getPlayer()), newInfoset->getAOids()))
                return;
            auto it = infosetSelectors_.find(currentInfoset);
            if (it == infosetSelectors_.end())
                return;
            auto distribution = it->second->getActionsProbDistribution();
            for (int i = 0; i < currentNode->availableActions().size(); i++)
            {
                auto action = currentNode->availableActions()[i];
                fillBelief(currentNode->performAction(action), newInfoset, prob * distribution[i], newNodes);
            }
        }
        if (currentNode->type_ == ChanceNode)
        {
            for (int i = 0; i < currentNode->availableActions().size(); i++)
            {
                auto action = currentNode->availableActions()[i];
                fillBelief(currentNode->performAction(action), newInfoset,prob * currentNode->chanceProbForAction(action), newNodes);
            }
        }
    }

//    int fillBelief(shared_ptr<EFGNode> &oldNode, const shared_ptr<AOH> &newInfoset, double oldNodeBelief, int nextPos) {
//
////        return isAOCompatible(currentInfoset_->getAOids(),
////                              h->performAction(action)->getAOids(currentInfoset_->getPlayer()));
//
//        if (newInfoset == oldNode->getAOHInfSet()) {
//            belief_[nextPos]=oldNodeBelief;
//            return nextPos+1;
//        }
//
//        if (oldNode->getPlayer() == newInfoset->getPlayer()){//searching player's node
//            if (oldNode->getAOHInfSet()->getAOids().size() != newInfoset->getAOids().size() - 1) return nextPos; //this is after opponent's move out of current IS
//            auto child = oldNode->performAction(oldNode->availableActions()[newInfoset->getAOids().end()->action]);
//            if (child == nullptr) return nextPos;//running out of the tree in memory, alternativly, we could add the nodes to the tree
//            if (child->type_ == TerminalNode) return nextPos;//if based on unknown information the last players action leads to a leaf
//            return fillBelief(child, newInfoset, oldNodeBelief, nextPos);
//        } else if (oldNode->type_ == ChanceNode){ //is current player is the environment????
//            int i=nextPos;
//            for (int j = 0; j < oldNode->availableActions().size(); j++){
//                auto child = oldNode->performAction(oldNode->availableActions()[j]);
//                if (child->type_ != TerminalNode)
//                    i = fillBelief(child, newInfoset, oldNodeBelief * oldNode->chanceProbForAction(oldNode->availableActions()[j]), i);
//            }
//            return i;
//        } else {//opponent's move
//            auto distribution = infosetSelectors_.find(oldNode->getAOHInfSet())->second->getActionsProbDistribution();
//
//            int i=nextPos;
//            for (int j = 0; j < oldNode->availableActions().size(); j++){
//                auto child = oldNode->performAction(oldNode->availableActions()[j]);
//                if (child != nullptr && child->type_ != TerminalNode)
//                    i = fillBelief(child, newInfoset, oldNodeBelief * distribution[j], i);
//            }
//            return i;
//        }
//    }
}