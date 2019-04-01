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

#include "base/efg.h"

namespace GTLib2 {


/**
 * Save EFG tree structure in a cache.
 *
 * Calls from EFGNode::performAction() are not cached, so at each call a copy is created.
 * This structure is a caching wrapper built on top of EFGNode.
 * It also caches the retrieval of (augmented) information sets and the nodes within them.
 *
 * You can extend this cache to save more information needed by your algoritm.
 */
class EFGCache {

    /**
     * Root distribution of the nodes
     */
    EFGNodesDistribution rootNodes_;

    /**
     * Many EFGNodes can belong to many (augmented) infosets.
     *
     * These two fields together represent a bipartite graph.
     */
    unordered_map<shared_ptr<EFGNode>, vector<shared_ptr<AOH>>> node2infosets_;
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> infoset2nodes_;

    /**
     * Specify that in a given node, with which action new distribution of nodes can be obtained.
     *
     * Note that if you need to access parent nodes, they are saved in each respective node:
     * EFGNode::getParent
     */
    unordered_map<shared_ptr<EFGNode>, EFGActionNodesDistribution> nodesChildren_;

 public:
    explicit EFGCache(const OutcomeDistribution &rootProbDist);
    explicit EFGCache(const EFGNodesDistribution &rootNodes);

    bool hasChildren(const shared_ptr<EFGNode> &node);

    bool hasChildren(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action);

    inline bool hasNode(const shared_ptr<EFGNode> &node) {
        return node2infosets_.find(node) != node2infosets_.end();
    }

    inline bool hasInfoset(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_.find(augInfoset) != infoset2nodes_.end();
    }

    inline const vector<shared_ptr<EFGNode>> &getNodesFor(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_[augInfoset];
    }

    inline const vector<shared_ptr<AOH>> &getInfosetsFor(const shared_ptr<EFGNode> &node) {
        return node2infosets_[node];
    }

    long countAugInfosets() {
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

    /**
     * Retrieve children for the node after following some action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGNodesDistribution &
    getChildrenFor(const shared_ptr<EFGNode> &node, const shared_ptr<Action> &action);

    /**
     * Retrieve children for the node after following any action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGActionNodesDistribution &getChildrenFor(const shared_ptr<EFGNode> &node);

    inline const EFGNodesDistribution &getRootNodes() {
        return rootNodes_;
    }

    /**
     * Create complete cache up to specified depth
     */
    void buildForest(int maxDepth);

    /**
     * Create complete cache
     */
    void buildForest();

 protected:
    void updateAugInfosets(const shared_ptr<EFGNode> &node);
    void createNode(const shared_ptr<EFGNode> &node);

 private:
    bool builtForest_ = false;
};

} // namespace GTLib2


#endif //GTLIB2_CACHE_H
