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

    virtual ~EFGCache() = default;

    inline EFGCache(const EFGCache &other) :
        rootNode_(other.rootNode_),
        nodesChildren_(other.nodesChildren_),
        domain_(other.domain_),
        builtForest_(other.builtForest_) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createNode(n); });
    }

    inline virtual void reset() {}
    inline virtual void clear() {
        nodesChildren_.clear();
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

    inline explicit InfosetCache(const InfosetCache &other) :
        EFGCache(other) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createAugInfosets(n); });
        node2infosets_ = other.node2infosets_;
        infoset2nodes_ = other.infoset2nodes_;
    }

    inline void clear() override {
        EFGCache::clear();
        node2infosets_.clear();
        infoset2nodes_.clear();
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

    inline PublicStateCache(const PublicStateCache &other) : EFGCache(other) {
        addCallback([&](const shared_ptr<EFGNode> &n) { this->createPublicState(n); });
        node2publicState_ = other.node2publicState_ ;
        publicState2nodes_ = other.publicState2nodes_ ;
        infoset2publicState_ = other.infoset2publicState_ ;
        publicState2infosets_ = other.publicState2infosets_ ;
    }

    inline void clear() override {
        EFGCache::clear();
        node2publicState_.clear();
        publicState2nodes_.clear();
        infoset2publicState_.clear();
        publicState2infosets_.clear();
        this->createPublicState(getRootNode());
    }

    inline bool hasPublicState(const shared_ptr<PublicState> &pubState) const {
        return publicState2nodes_.find(pubState) != publicState2nodes_.end();
    }
    inline bool hasPublicStateFor(const shared_ptr<EFGNode> &node) const {
        return node2publicState_.find(node) != node2publicState_.end();
    }
    inline bool hasPublicStateFor(const shared_ptr<AOH> &infoset) const {
        return infoset2publicState_.find(infoset) != infoset2publicState_.end();
    }

    inline const shared_ptr<PublicState> &getPublicStateFor(const shared_ptr<EFGNode> &node) const {
        return node2publicState_.at(node);
    }

    inline const shared_ptr<PublicState> &getPublicStateFor(const shared_ptr<AOH> &infoset) const {
        return infoset2publicState_.at(infoset);
    }


    inline const unordered_set<shared_ptr<EFGNode>> &
    getNodesForPubState(const shared_ptr<PublicState> &state) const {
        return publicState2nodes_.at(state);
    }

    inline const unordered_set<shared_ptr<AOH>> &getInfosetsFor(
        const shared_ptr<PublicState> &pubState) {
        return publicState2infosets_.at(pubState);
    }

    inline unordered_set<shared_ptr<AOH>>
    getInfosetsForPubStatePlayer(const shared_ptr<PublicState> &pubState, Player pl) {
        const auto &infosets = publicState2infosets_.at(pubState);
        auto filteredSet = unordered_set<shared_ptr<AOH>>(infosets.size() / 2);
        for (const auto &infoset : infosets) {
            if (infoset->getPlayer() == pl) {
                filteredSet.emplace(infoset);
            }
        }
        return filteredSet;
    }

    const shared_ptr<PublicState> &getRootPublicState() const {
        return node2publicState_.at(getRootNode());
    }

    const unordered_map<shared_ptr<PublicState>, unordered_set<shared_ptr<EFGNode>>> &
    getPublicState2nodes() const {
        return publicState2nodes_;
    }

    inline unsigned long countPublicStates() {
        return publicState2nodes_.size();
    }

 private:
    void createPublicState(const shared_ptr<EFGNode> &node);
};

class StrategyCache {
 public:
    virtual optional <ProbDistribution> strategyFor(const shared_ptr<AOH> &currentInfoset) const = 0;
    virtual ~StrategyCache() = default;
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
bool treeWalk(EFGCache &cache, EFGNodeCallback function, unsigned int maxEfgDepth);

inline unsigned int cntPsChildren(const PublicStateCache &cache, const shared_ptr<PublicState> &parent) {
    // todo: inefficient but gets jobs done -- we have fully built caches
    assert(cache.isCompletelyBuilt());

    auto cnt = 0;
    for (const auto &[pubState, _] :  cache.getPublicState2nodes()) {
        if (pubState->getDepth() == parent->getDepth() + 1
            && isCompatible(parent->getHistory(), pubState->getHistory()))
            cnt++;
    }
    return cnt;
}

inline unsigned int psIsTerminal(const PublicStateCache &cache, const shared_ptr<PublicState> &node) {
    // todo: inefficient but gets jobs done -- we have fully built caches
    if(cntPsChildren(cache, node) == 0) {
#ifndef NDEBUG
        auto efgNodes = cache.getNodesForPubState(node);
        for (auto &efgNode: efgNodes ) {
            assert(node->getHistory() == efgNode->getPubObsIds());
//            cout << efgNode->getPubObsIds() << endl;
//            cout << efgNode->getAOids(0) << endl;
//            cout << efgNode->getAOids(1) << endl;
//            assert(efgNode->type_ == TerminalNode);
        }
#endif
        return true;
    }
    return false;
}

inline shared_ptr<PublicState> expandPs(const PublicStateCache &cache,
                                        const shared_ptr<PublicState> &parent, EdgeId index) {
    // todo: inefficient but gets jobs done -- we have fully built caches
    assert(cache.isCompletelyBuilt());

    vector<shared_ptr<PublicState>> children;
    for (const auto &[pubState, _] :  cache.getPublicState2nodes()) {
        if (pubState->getDepth() == parent->getDepth() + 1
            && isCompatible(parent->getHistory(), pubState->getHistory())) {
            children.push_back(pubState);
        }
    }
    std::sort(children.begin(), children.end()); // invokes PublicState::operator<
    return children.at(index);
}

} // namespace GTLib2


#endif //GTLIB2_CACHE_H
