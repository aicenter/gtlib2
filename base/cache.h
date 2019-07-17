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
#include "base/fogefg.h"

namespace GTLib2 {

/**
 * Vector of nodes after following a specified action id.
 *
 * Note that action ids are indexed from 0..N-1
 */
typedef vector<shared_ptr<EFGNode>> EFGChildNodes;


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
 private:
    const shared_ptr<EFGNode> rootNode_;

    /**
     * Specify that in a given node, with which action new distribution of nodes can be obtained.
     *
     * Note that if you need to access parent nodes, they are saved in each respective node:
     * EFGNode::getParent
     */
    unordered_map<shared_ptr<EFGNode>, EFGChildNodes> nodesChildren_;

 public:
    inline explicit EFGCache(const Domain &domain)
        : rootNode_(createRootEFGNode(domain)), domain_(domain) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createNode(n); });
        this->createNode(rootNode_);
    }

    /**
     * Check if cache contains all the children for given node (after following any action).
     */
    bool hasAllChildren(const shared_ptr<EFGNode> &node) const;

    /**
     * Check if cache contains at least one child for given node (after following any action).
     */
    bool hasAnyChildren(const shared_ptr<EFGNode> &node) const;

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
    const shared_ptr<EFGNode> &getChildFor(const shared_ptr<EFGNode> &node,
                                           const shared_ptr<Action> &action);

    /**
     * Retrieve children for the node after following any action.
     *
     * The nodes are saved in the cache along their augmented infoset identification.
     */
    const EFGChildNodes &getChildrenFor(const shared_ptr<EFGNode> &node);

    /**
     * Get cached root nodes for the domain
     */
    inline const shared_ptr<EFGNode> &getRootNode() const { return rootNode_; }

    /**
     * Get copy of all the nodes that are present in the cache.
     */
    vector<shared_ptr<EFGNode>> getNodes() const;

    /**
     * Get a reference to the (parent -> children) map.
     */
    inline const unordered_map<shared_ptr<EFGNode>, EFGChildNodes> &getNodesChildren() const {
        return nodesChildren_;
    }

    /**
     * Create complete cache up to specified depth
     */
    void buildTree(int maxEfgDepth);

    /**
     * Create complete cache up to the depth specified by the domain.
     */
    void buildTree();

    /**
     * Check if the command `buildForest` was called on this cache before.
     */
    inline bool isCompletelyBuilt() const {
        return builtForest_;
    }

    inline unsigned int getDomainMaxStateDepth() const {
        return domain_.getMaxStateDepth();
    }

 protected:
    typedef function<void(const shared_ptr<EFGNode> &)> ProcessingCallback;

    inline void addCallback(const ProcessingCallback &cb) {
        callbacks_.emplace_back(cb);
    }

 private:
    void createNode(const shared_ptr<EFGNode> &node);

    EFGChildNodes &getCachedNode(const shared_ptr<EFGNode> &shared_ptr);
    const Domain &domain_;

    bool builtForest_ = false;
    /**
     * Each class should state the set of operations that should be done with specified (new) node.
     */
    vector<ProcessingCallback> callbacks_;

    inline void processNode(const shared_ptr<EFGNode> &node) {
        for (const auto &callback: callbacks_) {
            callback(node);
        }
    }
};

/**
 * Add caching of (augmented) information sets and the nodes within them.
 */
class InfosetCache: public virtual EFGCache {

    /**
     * Many EFGNodes can belong to many (augmented) infosets.
     *
     * These two fields together represent a bipartite graph.
     */
    unordered_map<shared_ptr<EFGNode>, vector<shared_ptr<AOH>>> node2infosets_;
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> infoset2nodes_;

 public:
    inline explicit InfosetCache(const Domain &domain) : EFGCache(domain) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createAugInfosets(n); });
        this->createAugInfosets(getRootNode());
    }

    inline bool hasInfoset(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_.find(augInfoset) != infoset2nodes_.end();
    }

    inline const vector<shared_ptr<EFGNode>> &getNodesFor(const shared_ptr<AOH> &augInfoset) {
        return infoset2nodes_.at(augInfoset);
    }

    inline const vector<shared_ptr<AOH>> &getInfosetsFor(const shared_ptr<EFGNode> &node) {
        return node2infosets_.at(node);
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
        return node2infosets_.at(node).at(node->getPlayer());
    }

    /**
     * Find augmented infoset for the supplied node.
     *
     * This function cannot be called on terminal nodes, as infosets are not defined there.
     * It also crashes if you ask for infoset for a node which is not saved in this cache.
     */
    inline const shared_ptr<AOH> &
    getAugInfosetFor(const shared_ptr<EFGNode> &node, Player player) {
        return node2infosets_.at(node).at(player);
    }

    const unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> &getInfoset2NodeMapping()
    const {
        return infoset2nodes_;
    }

 private:
    void createAugInfosets(const shared_ptr<EFGNode> &node);
};


/**
 * Add caching of public states and their respective augmented information sets and nodes.
 */
class PublicStateCache: public virtual EFGCache {

    /**
      * Many EFGNodes can belong to one public state.
      * Many infosets can belong to one public state.
      */
    unordered_map<shared_ptr<EFGNode>, shared_ptr<PublicState>> node2publicState_;
    unordered_map<shared_ptr<PublicState>, unordered_set<shared_ptr<EFGNode>>>
        publicState2nodes_;
    unordered_map<shared_ptr<AOH>, shared_ptr<PublicState>> infoset2publicState_;
    unordered_map<shared_ptr<PublicState>, unordered_set<shared_ptr<AOH>>> publicState2infosets_;

 public:
    inline explicit PublicStateCache(const Domain &domain) : EFGCache(domain) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createPublicState(n); });
        this->createPublicState(getRootNode());
    }

    inline bool hasPublicState(const shared_ptr<PublicState> &pubState) {
        return publicState2nodes_.find(pubState) != publicState2nodes_.end();
    }
    inline bool hasPublicState(const shared_ptr<EFGNode> &node) {
        return node2publicState_.find(node) != node2publicState_.end();
    }
    inline bool hasPublicState(const shared_ptr<AOH> &infoset) {
        return infoset2publicState_.find(infoset) != infoset2publicState_.end();
    }

    inline const shared_ptr<PublicState> &getPublicStateFor(const shared_ptr<EFGNode> &node) {
        return node2publicState_.at(node);
    }

    inline const shared_ptr<PublicState> &getPublicStateFor(const shared_ptr<AOH> &infoset) {
        return infoset2publicState_.at(infoset);
    }


    inline const unordered_set<shared_ptr<EFGNode>> &
    getNodesFor(const shared_ptr<PublicState> &state) {
        return publicState2nodes_.at(state);
    }

    inline const unordered_set<shared_ptr<AOH>> &getInfosetsFor(
        const shared_ptr<PublicState> &pubState) {
        return publicState2infosets_.at(pubState);
    }

    inline unordered_set<shared_ptr<AOH>> getInfosetsFor(const shared_ptr<PublicState> &pubState,
                                                         Player pl) {
        const auto &infosets = publicState2infosets_.at(pubState);
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

 private:
    void createPublicState(const shared_ptr<EFGNode> &node);
};

class StrategyCache {
 public:
    virtual ProbDistribution strategyFor(const shared_ptr<AOH> &currentInfoset) = 0;
};

/**
 * Call supplied function at each EFGNode of the EFG tree supplied by cache, including leaves.
 * The tree is walked as DFS up to maximum depth.
 */
void treeWalk(EFGCache &cache, EFGNodeCallback function);

/**
 * Call supplied function at each EFGNode of the EFG tree supplied by cache, including leaves.
 * The tree is walked as DFS up to maximum depth.
 */
bool treeWalk(EFGCache &cache, EFGNodeCallback function, int maxEfgDepth);

} // namespace GTLib2


#endif //GTLIB2_CACHE_H
