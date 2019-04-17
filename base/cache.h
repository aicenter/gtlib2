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


#ifndef GTLIB2_CACHE_H
#define GTLIB2_CACHE_H

#include "base/base.h"
#include "base/efg.h"
#include "algorithms/common.h"

using GTLib2::algorithms::createRootEFGNodes;

namespace GTLib2 {


/**
 * Save EFG tree structure in a cache.
 *
 * Calls from EFGNode::performAction() are not cached, so at each call a copy is created.
 * This structure is a caching wrapper built on top of EFGNode to prevent duplicate calls
 * for creation of children EFGNodes.
 *
 * You can extend this cache to save more information needed by your algorithm.
 */
class EFGCache {

    /**
     * Root distribution of the nodes
     */
    EFGNodesDistribution rootNodes_;

    /**
     * Specify that in a given node, with which action new distribution of nodes can be obtained.
     *
     * Note that if you need to access parent nodes, they are saved in each respective node:
     * EFGNode::getParent
     */
    unordered_map<shared_ptr<EFGNode>, EFGActionNodesDistribution> nodesChildren_;

 public:
    inline explicit EFGCache(const Domain &domain) : domain_(domain) {}

    /**
     * Check if cache contains all the children for given node (after following any action).
     */
    bool hasChildren(const shared_ptr<EFGNode> &node) const;

    /**
     * Check if cache contains children of for a given (node, action)
     */
    bool hasChildren(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action) const;

    /**
     * Check if cache contains information about children of specified node.
     */
    inline bool hasNode(const shared_ptr<EFGNode> &node) const {
        return nodesChildren_.find(node) != nodesChildren_.end();
    }

    /**
     * Retrieve children for the node after following some action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGNodesDistribution &getChildrenFor(const shared_ptr<EFGNode> &node,
                                               const shared_ptr<Action> &action);

    /**
     * Retrieve children for the node after following any action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGActionNodesDistribution &getChildrenFor(const shared_ptr<EFGNode> &node);

    /**
     * Get cached root nodes for the domain
     */
    inline const EFGNodesDistribution &getRootNodes() {
        // rootNodes cannot be initialized in constructor,
        // because createNode is a virtual function that can be overriden in child classes
        // https://www.artima.com/cppsource/nevercall.html
        if (rootNodes_.empty()) {
            rootNodes_ = createRootEFGNodes(domain_);
        }
        return rootNodes_;
    }

    /**
     * Get copy of all the nodes that are present in the cache.
     */
    vector<shared_ptr<EFGNode>> getNodes() const;

    /**
     * Get a reference to the (parent -> children) map.
     */
    inline const unordered_map<shared_ptr<EFGNode>, EFGActionNodesDistribution> &getNodesChildren()
    const {
        return nodesChildren_;
    }

    /**
     * Create complete cache up to specified depth
     */
    void buildForest(int maxDepth);

    /**
     * Create complete cache up to the depth specified by the domain.
     */
    void buildForest();

    /**
     * Check if the command `buildForest` was called on this cache before.
     */
    inline bool isCompletelyBuilt() const {
        return builtForest_;
    }

 protected:

    /**
     * To be overridden in child classes:
     * each class should state the set of operations that should be done with specified (new) node.
     */
    virtual void processNode(const shared_ptr<EFGNode> &node);

    void createNode(const shared_ptr<EFGNode> &node);

    EFGActionNodesDistribution &getCachedNode(const shared_ptr<EFGNode> &shared_ptr);
    const Domain &domain_;

 private:
    bool builtForest_ = false;
};

/**
 * Add caching of (augmented) information sets and the nodes within them.
 */
class InfosetCache: public EFGCache {

    /**
     * Many EFGNodes can belong to many (augmented) infosets.
     *
     * These two fields together represent a bipartite graph.
     */
    unordered_map<shared_ptr<EFGNode>, vector<shared_ptr<AOH>>> node2infosets_;
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> infoset2nodes_;

 public:
    inline explicit InfosetCache(const Domain &domain) : EFGCache(domain) {}

    inline bool hasInfoset(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_.find(augInfoset) != infoset2nodes_.end();
    }

    inline const vector<shared_ptr<EFGNode>> &getNodesFor(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_[augInfoset];
    }

    inline const vector<shared_ptr<AOH>> &getInfosetsFor(const shared_ptr<EFGNode> &node) {
        return node2infosets_[node];
    }

    inline long countAugInfosets() {
        return infoset2nodes_.size();
    }

    /**
     * Find infoset for the supplied node.
     *
     * This is equivalent to searching for augmented infoset with the same player
     * as is the acting player in the specified node.
     *
     * This function cannot be called on terminal nodes, as infosets are not defined there.
     * It also crashes if you ask for infoset for a node which is not saved in this cache.
     */
    inline const shared_ptr<AOH> &getInfosetFor(const shared_ptr<EFGNode> &node) {
        return node2infosets_[node][*node->getCurrentPlayer()];
    }

    /**
     * Find augmented infoset for the supplied node.
     *
     * This function cannot be called on terminal nodes, as infosets are not defined there.
     * It also crashes if you ask for infoset for a node which is not saved in this cache.
     */
    inline const shared_ptr<AOH> &
    getAugInfosetFor(const shared_ptr<EFGNode> &node, Player player) {
        return node2infosets_[node][player];
    }

 protected:
    void processNode(const shared_ptr<EFGNode> &node) override;
    void createAugInfosets(const shared_ptr<EFGNode> &node);
};


/**
 * Add caching of public states and their respective augmented information sets and nodes.
 */
class PublicStateCache: public EFGCache {

    /**
      * Many EFGNodes can belong to one public state.
      * Many infosets can belong to one public state.
      */
    unordered_map<shared_ptr<EFGNode>, shared_ptr<EFGPublicState>> node2publicState_;
    unordered_map<shared_ptr<EFGPublicState>, unordered_set<shared_ptr<EFGNode>>> publicState2nodes_;
    unordered_map<shared_ptr<AOH>, shared_ptr<EFGPublicState>> infoset2publicState_;
    unordered_map<shared_ptr<EFGPublicState>, unordered_set<shared_ptr<AOH>>> publicState2infosets_;

 public:
    inline explicit PublicStateCache(const Domain &domain) : EFGCache(domain) {}

    inline bool hasPublicState(const shared_ptr<EFGPublicState> &pubState) {
        return publicState2nodes_.find(pubState) != publicState2nodes_.end();
    }

    inline const shared_ptr<EFGPublicState> &getPublicStateFor(const shared_ptr<EFGNode> &node) {
        return node2publicState_[node];
    }

    inline const shared_ptr<EFGPublicState> &getPublicStateFor(const shared_ptr<AOH> &infoset) {
        return infoset2publicState_[infoset];
    }


    inline const unordered_set<shared_ptr<EFGNode>> &getNodesFor(const shared_ptr<EFGPublicState> &state) {
        return publicState2nodes_[state];
    }

    inline const unordered_set<shared_ptr<AOH>> &getInfosetsFor(
        const shared_ptr<EFGPublicState> &pubState) {
        return publicState2infosets_[pubState];
    }

    inline unordered_set<shared_ptr<AOH>> getInfosetsFor(const shared_ptr<EFGPublicState> &pubState,
                                                         Player pl) {
        const auto &infosets = publicState2infosets_[pubState];
        auto filteredSet = unordered_set<shared_ptr<AOH>>(infosets.size() / 2);
        for (const auto &infoset : infosets) {
            if (infoset->getPlayer() == pl) {
                filteredSet.emplace(infoset);
            }
        }
        return filteredSet;
    }

    inline unsigned long countPublicStates() {
        return publicState2nodes_.size();
    }

 protected:
    void processNode(const shared_ptr<EFGNode> &node) override;
    void createPublicState(const shared_ptr<EFGNode> &node);
};

} // namespace GTLib2


#endif //GTLIB2_CACHE_H
