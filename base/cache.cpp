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


#include "cache.h"
#include "algorithms/tree.h"
#include "algorithms/common.h"


namespace GTLib2 {

EFGCache::EFGCache(const GTLib2::EFGNodesDistribution &rootNodesDist) {
    rootNodes_ = rootNodesDist;
    for (auto &[node, _]: rootNodesDist) {
        createNode(node);
        updateAugInfosets(node);
    }
}

EFGCache::EFGCache(const OutcomeDistribution &rootProbDist)
    : EFGCache(algorithms::createRootEFGNodes(rootProbDist)) {
}

bool EFGCache::hasChildren(const shared_ptr<EFGNode> &node) {
    auto it = nodesChildren_.find(node);
    if (it == nodesChildren_.end()) return false;

    auto &distributionEntry = it->second;
    for (auto ptr : distributionEntry) {
        if (ptr != nullptr) return true;
    }
    return false;
}

bool EFGCache::hasChildren(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action) {
    auto it = nodesChildren_.find(node);
    if (it == nodesChildren_.end()) return false;
    auto &distributionEntry = it->second;
    auto id = action->getId();
    return distributionEntry.size() >= id && distributionEntry[id] != nullptr;
}


const EFGNodesDistribution &
EFGCache::getChildrenFor(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action) {
    auto maybeNode = nodesChildren_.find(node);

    // Node not found -- maybe trying to get children
    // for a node gotten outside from cache?
    assert(maybeNode != nodesChildren_.end());

    // fetch from cache if possible
    const auto actionId = action->getId();
    auto &nodeDist = maybeNode->second;
    if (nodeDist.size() >= actionId && nodeDist[actionId] != nullptr) {
        return *nodeDist[action->getId()];
    }

    // create new nodes and save them to cache
    assert(nodeDist.size() > actionId);
    auto newDist = node->performAction(action);
    nodeDist.at(actionId) = make_shared<EFGNodesDistribution>(newDist);

    for (auto &[childNode, _]: newDist) {
        createNode(childNode);
        updateAugInfosets(childNode);
    }

    // retrieve from map directly to return a reference,
    // this is guaranteed to exist there since we've just inserted it
    return *nodesChildren_.find(node)->second[actionId];
}

const EFGActionNodesDistribution &EFGCache::getChildrenFor(const shared_ptr<EFGNode> &node) {
    auto maybeNode = nodesChildren_.find(node);

    // Node not found -- maybe trying to get children
    // for a node gotten outside from cache?
    assert(maybeNode != nodesChildren_.end());

    auto &nodeDist = maybeNode->second;
    if (builtForest_) return nodeDist;

    bool hasMissing = false;
    for (auto &ptr : nodeDist) {
        if (ptr == nullptr) hasMissing = true;
    }
    if (!hasMissing) return nodeDist;

    // add missing actions
    for (auto &action : node->availableActions()) {
        if (nodeDist[action->getId()] != nullptr) {
            continue;
        }

        auto newDist = node->performAction(action);
        nodeDist.at(action->getId()) = make_shared<EFGNodesDistribution>(newDist);

        for (auto &[childNode, _]: newDist) {
            createNode(childNode);
            updateAugInfosets(childNode);
        }
    }

    return nodeDist;
}

void EFGCache::updateAugInfosets(const shared_ptr<EFGNode> &node) {
    vector<shared_ptr<AOH>> infosets;

    for (Player pl = 0; pl < GAME_MAX_PLAYERS; pl++) {
        auto infoset = node->getAOHAugInfSet(pl);
        auto maybeInfoset = infoset2nodes_.find(infoset);

        if (maybeInfoset == infoset2nodes_.end()) {
            // infoset not found yet, initialize it with this node
            infosets.emplace_back(infoset);
            infoset2nodes_.emplace(infoset, vector<shared_ptr<EFGNode>>{node});
        } else {
            // infoset found, append this node
            infoset = maybeInfoset->first;
            infosets.emplace_back(infoset);
            maybeInfoset->second.push_back(node);
        }
    }
    assert (infosets.size() == GAME_MAX_PLAYERS);
    node2infosets_.emplace(node, infosets);
}

void EFGCache::createNode(const shared_ptr<EFGNode> &node) {
    nodesChildren_.emplace(
        node, EFGActionNodesDistribution(node->countAvailableActions(), nullptr));
}

void EFGCache::buildForest(int maxDepth) {
    algorithms::treeWalkEFG(this, [](shared_ptr<EFGNode> _) {}, maxDepth);
}

void EFGCache::buildForest() {
    buildForest(INT_MAX);
    builtForest_ = true;
}

} // namespace GTLib2
