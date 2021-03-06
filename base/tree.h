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


#ifndef GTLIB2_TREE_H
#define GTLIB2_TREE_H

#include "base/includes.h"
#include "base/hashing.h"
#include "utils/utils.h"

namespace GTLib2 {

typedef uint32_t EdgeId;

template<class Parent>
class Node {
 public:
    explicit Node(shared_ptr<Parent const> parent, optional <EdgeId> incomingEdge) :
        parent_(move(parent)),
        history_(parent_ == nullptr
                 ? vector<EdgeId>()
                 : extend(parent_->history_, *incomingEdge)),
        hashNode_(hashWithSeed(history_.data(), history_.size() * sizeof(EdgeId), 1412914847)) {
        assert((parent_ == nullptr && incomingEdge == nullopt)
                   || (parent_ != nullptr && incomingEdge != nullopt));
    }

    explicit Node(shared_ptr<Parent const> parent, vector<EdgeId> history) :
        parent_(move(parent)),
        history_(move(history)),
        hashNode_(hashWithSeed(history_.data(), history_.size() * sizeof(EdgeId), 1412914847)) {
        assert(!parent_ || isExtension(parent_->getHistory(), history_));
    }


    // Root node constructor
    inline Node() : Node(nullptr, nullopt) {}

    inline Node(const Node &other) :
        parent_(other.parent_),
        history_(other.history_),
        hashNode_(other.hashNode_) {}

    virtual ~Node() = default;

    inline unsigned long getDepth() const { return history_.size(); }
    inline HashType getHash() const { return hashNode_; };
    inline bool operator==(const Node &rhs) const {
        if (hashNode_ != rhs.hashNode_) return false;
        if (history_.size() != rhs.history_.size()) return false;
        return !memcmp(history_.data(), rhs.history_.data(), history_.size());
    }

    string toString() const {
        std::stringstream ss;
        ss << "∅";
        for (unsigned int edgeId : history_) ss << "," << edgeId;
        return ss.str();
    }

    bool isRoot() const { return parent_ == nullptr; }

    const shared_ptr<Parent const> parent_;
    const vector <EdgeId> history_;
 private:
    const HashType hashNode_;
};

template<class Node>
using NodeCallback = function<void(const shared_ptr < Node > &)>;

template<class Node>
using NodeChildCnt = function<unsigned int(const shared_ptr < Node > &)>;
template<class Node>
unsigned int nodeChildCnt(const shared_ptr < Node > &);

template<class Node>
using NodeChildExpander = function<shared_ptr<Node>(const shared_ptr < Node > &, EdgeId)>;
template<class Node>
shared_ptr <Node> nodeChildExpander(const shared_ptr <Node> &, EdgeId);

template<class Node>
bool nodeCompare(const shared_ptr <Node> &a, const shared_ptr <Node> &b) {
    if(a->getHistory().size() < b->getHistory().size()) return true;
    return a->getHistory() < b->getHistory();
}

/**
 * Call supplied function at each node of the tree, including leaves,
 * with no restriction to depth of the tree walk.
 */
template<class Node>
void treeWalk(const shared_ptr <Node> &node, const NodeCallback<Node> &callback) {
    callback(node);
    for (EdgeId i = 0; i < nodeChildCnt<Node>(node); ++i) {
        auto nextNode = nodeChildExpander<Node>(node, i);
        treeWalk(nextNode, callback);
    }
}

/**
 * Call supplied function at each node of the tree, including leaves.
 * The tree is walked as DFS up to maximum specified depth.
 *
 * @return whether the entire tree has been walked
 */
template<class Node>
bool treeWalk(const shared_ptr <Node> &node,
              const NodeCallback<Node> &callback,
              const NodeChildCnt<Node> &cntChildren,
              const NodeChildExpander<Node> &childAt,
              unsigned int maxDepth) {
    callback(node);
    if (node->depth_ >= maxDepth) return cntChildren(node) == 0;

    bool entireTreeWalked = true;
    for (EdgeId i = 0; i < cntChildren(node); ++i) {
        auto nextNode = childAt(node, i);
        entireTreeWalked = treeWalk(nextNode, callback, cntChildren, childAt, maxDepth)
            && entireTreeWalked;
    }
    return entireTreeWalked;
}

/**
 * Call supplied function at each node of the tree, including leaves,
 * with no restriction to depth of the tree walk.
 */
template<class Node>
void treeWalk(const shared_ptr <Node> &node,
              const NodeCallback<Node> &callback,
              const NodeChildCnt<Node> &cntChildren,
              const NodeChildExpander<Node> &childAt) {
    callback(node);
    for (EdgeId i = 0; i < cntChildren(node); ++i) {
        auto nextNode = childAt(node, i);
        treeWalk(nextNode, callback, cntChildren, childAt);
    }
}

//treeFold
//treeMap
//treeWalkDFS
//treeWalkBFS

template<class Node>
using Tree = unordered_map <shared_ptr<Node>, vector<shared_ptr < Node>>>;

template<class Node>
void copyTree(const Tree<Node> &origTree,
              const shared_ptr <Node> &origParent,
              Tree<Node> &newTree,
              const shared_ptr <Node> &newParent) {

    auto it = origTree.find(origParent);
    if (it == origTree.end()) return; // nothing to copy

    vector<shared_ptr<Node>> origChildren = it->second;
    vector<shared_ptr<Node>> newChildren = vector<shared_ptr<Node>>(origChildren.size(), nullptr);

    for (int i = 0; i < origChildren.size(); ++i) {
        const auto &origChild = origChildren.at(i);
        if (origChild == nullptr) continue;

        newChildren.at(i) = origChild->clone(newParent);
        copyTree(origTree, origChild, newTree, newChildren.at(i));
    }
}

}

#endif //GTLIB2_TREE_H
