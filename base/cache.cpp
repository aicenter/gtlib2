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

#include "base/base.h"
#include "cache.h"

#include "algorithms/tree.h"


namespace GTLib2 {

bool EFGCache::hasAllChildren(const shared_ptr<EFGNode> &node) const {
    auto it = nodesChildren_.find(node);
    if (it == nodesChildren_.end()) return false;

    for (auto &childNode : it->second) {
        if (!childNode) return false;
    }
    return true;
}

bool EFGCache::hasAnyChildren(const shared_ptr<EFGNode> &node) const {
    auto it = nodesChildren_.find(node);
    if (it == nodesChildren_.end()) return false;

    for (auto &childNode : it->second) {
        if (childNode) return true;
    }

    return false;
}

bool EFGCache::hasChildren(const shared_ptr<EFGNode> &node,
                           const shared_ptr<Action> &action) const {
    auto it = nodesChildren_.find(node);
    if (it == nodesChildren_.end()) return false;
    const EFGChildNodes &actionNodes = it->second;
    auto id = action->getId();
    return actionNodes.size() >= id && actionNodes[id];
}

EFGChildNodes &EFGCache::getCachedNode(const shared_ptr<EFGNode> &node) {
    // Node not found -- maybe trying to get children
    // for a node gotten outside from cache?
    if (nodesChildren_.find(node) == nodesChildren_.end()) {
        if (*getRootNode() == *node) {
            processNode(getRootNode());

            // createNode must append to nodesChildren
            return nodesChildren_[node];
        }

        assert(false); // not found even in root nodes :/
    }

    return nodesChildren_[node];
}


const shared_ptr<EFGNode> &
EFGCache::getChildFor(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action) {
    EFGChildNodes &nodes = getCachedNode(node);

    // fetch from cache if possible
    const auto actionId = action->getId();
    if (nodes.size() >= actionId && nodes[actionId]) return nodes[actionId];

    // create new nodes and save them to cache
    assert(nodes.size() > actionId);
    auto childNode = node->performAction(action);
    nodes[actionId] = childNode;
    this->processNode(childNode);

    return nodes[actionId];
}

const EFGChildNodes &EFGCache::getChildrenFor(const shared_ptr<EFGNode> &node) {
    EFGChildNodes &nodes = getCachedNode(node);

    // Check that we have built all of the actions
    if (builtForest_) return nodes;

    int missingIdx = -1;
    for (int i = 0; i < nodes.size(); i++) {
        if (!nodes[i]) {
            missingIdx = i;
            break;
        }
    }
    if (missingIdx == -1) return nodes;

    // Add missing actions
    auto actions = node->availableActions();
    for (int i = missingIdx; i < actions.size(); ++i) {
        if (nodes[i]) continue;

        auto childNode = node->performAction(actions[i]);
        nodes[i] = childNode;
        this->processNode(childNode);
    }

    return nodes;
}

vector<shared_ptr<EFGNode>> EFGCache::getNodes() const {
    vector<shared_ptr<EFGNode>> keys(nodesChildren_.size());
    transform(nodesChildren_.begin(), nodesChildren_.end(),
              keys.begin(), [](auto pair) { return pair.first; });
    return keys;
}

void EFGCache::createNode(const shared_ptr<EFGNode> &node) {
    nodesChildren_.emplace(node,
                           EFGChildNodes(node->type_ == TerminalNode
                                         ? 0 : node->countAvailableActions(), nullptr));
}

void EFGCache::buildForest(int maxStateDepth) {
    // Root node can have state depth of either
    // 0 - it's a chance node which decides about the root distribution of outcomes
    // 1 - it's another node that had previously only one outcome
    if (rootNode_->stateDepth_ <= maxStateDepth
        && nodesChildren_.find(rootNode_) == nodesChildren_.end()) {
        processNode(rootNode_);
    }
}


void EFGCache::buildForest() {
    buildForest(domain_.getMaxStateDepth());
    builtForest_ = true;
}

//void InfosetCache::processNode(const shared_ptr<GTLib2::EFGNode> &node) {
//    EFGCache::createNode(node);
//    createAugInfosets(node);
//}

void InfosetCache::createAugInfosets(const shared_ptr<EFGNode> &node) {
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


void PublicStateCache::createPublicState(const shared_ptr<EFGNode> &node) {
    auto pubState = node->getPublicState();
    auto infoset0 = node->getAOHAugInfSet(Player(0));
    auto infoset1 = node->getAOHAugInfSet(Player(1));

    node2publicState_.emplace(node, pubState);
    infoset2publicState_.emplace(infoset0, pubState);
    infoset2publicState_.emplace(infoset1, pubState);

    auto maybePubStateNode = publicState2nodes_.find(pubState);
    if (maybePubStateNode == publicState2nodes_.end()) {
        publicState2nodes_.emplace(pubState, unordered_set<shared_ptr<EFGNode>>{node});
    } else {
        maybePubStateNode->second.emplace(node);
    }

    auto maybePubStateInfo = publicState2infosets_.find(pubState);
    if (maybePubStateInfo == publicState2infosets_.end()) {
        publicState2infosets_.emplace(pubState, unordered_set<shared_ptr<AOH>>{infoset0, infoset1});
    } else {
        maybePubStateInfo->second.emplace(infoset0);
        maybePubStateInfo->second.emplace(infoset1);
    }
}

} // namespace GTLib2
